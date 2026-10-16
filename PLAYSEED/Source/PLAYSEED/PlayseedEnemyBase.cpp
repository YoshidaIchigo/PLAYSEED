#include "PlayseedEnemyBase.h"
#include "PlayseedActionComponent.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

APlayseedEnemyBase::APlayseedEnemyBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostPhysics; // Characterの移動後の足元を観察します。
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("EnemyRoot"));
    Visual = CreateDefaultSubobject<USceneComponent>(TEXT("TuftlingVisual"));
    Visual->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
    AddPart(TEXT("Bud"), Sphere.Object, FVector(0, 0, 40), FVector(1.5f, 1.25f, .8f), FLinearColor(.25f, .35f, .55f));
    AddPart(TEXT("Leaf"), Cone.Object, FVector(0, 0, 95), FVector(.3f, .25f, .4f), FLinearColor(.2f, .65f, .45f));
    for (int32 Side = 0; Side < 2; ++Side)
    {
        const float Y = Side ? 25 : -25;
        AddPart(*FString::Printf(TEXT("Eye%d"), Side), Sphere.Object, FVector(60, Y, 60), FVector(.24f), FLinearColor(.95f, .95f, .8f));
        AddPart(*FString::Printf(TEXT("Pupil%d"), Side), Sphere.Object, FVector(70, Y, 60), FVector(.12f), FLinearColor(.025f, .04f, .07f));
    }
}

void APlayseedEnemyBase::AddPart(FName Name, UStaticMesh* Mesh, FVector Position, FVector Scale, FLinearColor Color)
{
    auto* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
    Part->SetupAttachment(Visual);
    Part->SetStaticMesh(Mesh);
    Part->SetRelativeLocation(Position);
    Part->SetRelativeScale3D(Scale);
    // 接触は下の足元の通過判定で処理します。見た目の小さな凹凸へ引っ掛けません。
    Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Parts.Add(Part);
    Colors.Add(Color);
}

void APlayseedEnemyBase::BeginPlay()
{
    Super::BeginPlay();
    SpawnTransform = GetActorTransform();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        for (int32 Index = 0; Index < Parts.Num(); ++Index)
        {
            auto* Material = UMaterialInstanceDynamic::Create(Palette, this);
            Material->SetVectorParameterValue(TEXT("Tint"), Index == 0 ? GetBudTint() : Colors[Index]);
            Parts[Index]->SetMaterial(0, Material);
        }
    }
}

void APlayseedEnemyBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bDefeated)
    {
        DefeatAge += DeltaSeconds;
        if (DefeatAge > .4f) { SetActorHiddenInGame(true); SetActorTickEnabled(false); }
        return;
    }
    if (HurtDisplayRemaining > 0)
    {
        HurtDisplayRemaining -= DeltaSeconds;
        if (HurtDisplayRemaining <= 0) Visual->SetRelativeScale3D(FVector::OneVector);
    }
    // 生き残る敵では、LaunchCharacterの反動が次の移動Tickへ届くまで側面接触を猶予します。
    // 長い一フレームで残り時間を越えても、そのフレームまでは猶予を維持します。
    const bool bStompGraceActive = StompContactGrace > 0;
    StompContactGrace = FMath::Max(0.f, StompContactGrace - DeltaSeconds);
    auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (!Player) return;
    const FVector Position = Player->GetActorLocation();
    const float HalfHeight = Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    const float Feet = Position.Z - HalfHeight;
    const float Top = GetActorLocation().Z + GetStompHeight();
    const float Radius = GetContactRadius() + Player->GetCapsuleComponent()->GetScaledCapsuleRadius();
    bool bStomp = false;
    if (bHasPreviousPosition && PreviousFeetZ >= Top && Feet <= Top && PreviousFeetZ - Feet > .01f && Player->GetCharacterMovement()->MovementMode != MOVE_Flying)
    {
        // 長いフレームで足元が頭の高さを通り過ぎても、前後位置から通過点を求めます。
        const float Fraction = FMath::Clamp((PreviousFeetZ - Top) / (PreviousFeetZ - Feet), 0.f, 1.f);
        const FVector Crossing = FMath::Lerp(PreviousPlayerPosition, Position, Fraction);
        bStomp = FVector::Dist2D(Crossing, GetActorLocation()) < Radius;
    }
    if (bStomp) HandleStomp(Player);
    else if (!bStompGraceActive && FVector::Dist2D(Position, GetActorLocation()) < Radius && Feet < Top - 15 && Feet + HalfHeight * 2 > GetActorLocation().Z + 10)
        Player->GetVitality()->TryTakeImpact(GetContactDamage(), Position - GetActorLocation());
    PreviousPlayerPosition = Position;
    PreviousFeetZ = Feet;
    bHasPreviousPosition = true;
}

void APlayseedEnemyBase::HandleStomp(AWindseedCharacter* Player)
{
    DefeatEnemy(Player, true);
}

void APlayseedEnemyBase::DefeatEnemy(AWindseedCharacter* Player, bool bBounce)
{
    if (bDefeated) return;
    bDefeated = true;
    DefeatAge = 0;
    Visual->SetRelativeScale3D(FVector(1.2f, 1.2f, .25f));
    if (bBounce) BounceFromStomp(Player);
    OnDefeated(Player);
}

void APlayseedEnemyBase::BounceFromStomp(AWindseedCharacter* Player)
{
    ++SuccessfulStomps;
    StompContactGrace = .3f;
    Player->GetActions()->CancelGroundPound();
    Player->StopJumping();
    // 撃破しない初回の踏みつけでも、同じ通常の反動を使えます。
    const FVector Velocity = Player->GetVelocity();
    Player->LaunchCharacter(FVector(Velocity.X, Velocity.Y, 540), true, true);
}

void APlayseedEnemyBase::RefreshBudTint()
{
    if (!Parts.IsEmpty())
        if (auto* Tint = Cast<UMaterialInstanceDynamic>(Parts[0]->GetMaterial(0))) Tint->SetVectorParameterValue(TEXT("Tint"), GetBudTint());
}

void APlayseedEnemyBase::ResetEnemy()
{
    bDefeated = false;
    bHasPreviousPosition = false;
    DefeatAge = 0;
    MeleeHealth = MaximumMeleeHealth;
    HurtDisplayRemaining = 0;
    StompContactGrace = 0;
    SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
    Visual->SetRelativeScale3D(FVector::OneVector);
    SetActorHiddenInGame(false);
    SetActorTickEnabled(true);
}

void APlayseedEnemyBase::ConfigureMeleeDurability(int32 HitPoints)
{
    MaximumMeleeHealth = FMath::Max(1, HitPoints);
    MeleeHealth = MaximumMeleeHealth;
}

bool APlayseedEnemyBase::ReceiveStrike(AWindseedCharacter* Player, int32 Damage)
{
    if (bDefeated || Damage <= 0) return false;
    ++ReceivedStrikes;
    MeleeHealth = FMath::Max(0, MeleeHealth - Damage);
    if (MeleeHealth == 0) DefeatEnemy(Player, false);
    else
    {
        HurtDisplayRemaining = .12f;
        Visual->SetRelativeScale3D(FVector(.9f, 1.1f, .9f));
    }
    return true;
}
