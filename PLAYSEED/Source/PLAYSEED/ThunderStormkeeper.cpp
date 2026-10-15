#include "ThunderStormkeeper.h"
#include "PlayseedLantern.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AThunderStormkeeper::AThunderStormkeeper()
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
    GetBody()->SetStaticMesh(Sphere.Object);
    GetBody()->SetRelativeScale3D(FVector(1.8f));
    // 子の形は見た目専用です。物理は親の球一つに集約し、角の引っ掛かりを防ぎます。
    AddPart(Cone.Object, FVector(0, 0, 55), FVector(.65f, .65f, .65f), FLinearColor(.8f, .43f, .12f));
    AddPart(Sphere.Object, FVector(46, -22, 12), FVector(.11f, .11f, .15f), FLinearColor(.04f, .08f, .13f));
    AddPart(Sphere.Object, FVector(46, 22, 12), FVector(.11f, .11f, .15f), FLinearColor(.04f, .08f, .13f));
}
UStaticMeshComponent* AThunderStormkeeper::AddPart(UStaticMesh* Mesh, FVector Position, FVector Scale, FLinearColor Color)
{
    auto* Part = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("KeeperPart%d"), Parts.Num()));
    Part->SetupAttachment(GetBody()); Part->SetStaticMesh(Mesh); Part->SetRelativeLocation(Position); Part->SetRelativeScale3D(Scale);
    Part->SetCollisionEnabled(ECollisionEnabled::NoCollision); Parts.Add(Part); PartColors.Add(Color); return Part;
}
void AThunderStormkeeper::PaintBody()
{
    auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette"));
    if (!Palette) return;
    auto* BodyMaterial = UMaterialInstanceDynamic::Create(Palette, this);
    BodyMaterial->SetVectorParameterValue(TEXT("Tint"), FLinearColor(.42f, .25f, .65f)); GetBody()->SetMaterial(0, BodyMaterial);
    for (int32 Index = 0; Index < Parts.Num(); ++Index)
    {
        auto* Material = UMaterialInstanceDynamic::Create(Palette, this);
        Material->SetVectorParameterValue(TEXT("Tint"), PartColors[Index]); Parts[Index]->SetMaterial(0, Material);
    }
}
void AThunderStormkeeper::BeginPlay()
{
    Super::BeginPlay(); ArenaFloor = GetActorLocation() - FVector(0, 0, 100);
    GetBody()->SetSimulatePhysics(false); GetBody()->SetMassOverrideInKg(NAME_None, 80, true); PaintBody();
}
bool AThunderStormkeeper::CanBePickedUp(AWindseedCharacter* Player) const
{
    // 正面からの E も TryPickup まで届かせるため、方向はここでは除外しません。
    return !bDefeated && !bRecovering && Super::CanBePickedUp(Player);
}
bool AThunderStormkeeper::TryPickup(AWindseedCharacter* Player)
{
    if (!CanBePickedUp(Player)) return false;
    const FVector TowardPlayer = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
    if (FVector::DotProduct(TowardPlayer, GetActorForwardVector()) > -.35f)
    {
        if (Player->GetVitality()->TryTakeImpact(2, Player->GetActorLocation() - GetActorLocation())) ++FrontCounters;
        return false;
    }
    return Super::TryPickup(Player);
}
FVector AThunderStormkeeper::GetThrowVelocity(const AWindseedCharacter* Player) const
{
    return Player->GetActorForwardVector() * 520 + FVector(0, 0, 300);
}
void AThunderStormkeeper::BeginRecovery(bool bOffArena)
{
    if (bRecovering || bDefeated) return;
    if (bOffArena) ++OffArenaReturns;
    bRecovering = true; RecoveryTime = 1;
    // 敵だけを戻す前に Carryable の保持・投擲状態を解除します。プレイヤーは移動しません。
    APlayseedCarryable::ResetToSpawn(); GetBody()->SetSimulatePhysics(false); GetBody()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void AThunderStormkeeper::ReturnToArena()
{
    APlayseedCarryable::ResetToSpawn(); GetBody()->SetSimulatePhysics(false); bRecovering = false; RecoveryTime = 0;
}
void AThunderStormkeeper::OnThrownLanding(const FHitResult& Hit)
{
    if (bDefeated || bRecovering) return;
    const bool bOnSummit = FMath::Abs(Hit.ImpactPoint.Z - ArenaFloor.Z) < 20 && FVector::Dist2D(GetActorLocation(), ArenaFloor) <= 550;
    if (!bOnSummit) { BeginRecovery(true); return; }
    ++ValidThrows;
    UE_LOG(LogTemp, Display, TEXT("STORMKEEPER: physical summit landing %d/3 at %s"), ValidThrows, *Hit.ImpactPoint.ToString());
    if (ValidThrows < 3) { BeginRecovery(false); return; }
    bDefeated = true; GetBody()->SetSimulatePhysics(false); SetActorEnableCollision(false); SetActorHiddenInGame(true);
    // 報酬は World が所有します。ここは弱参照だけを持ち、R で自分の未取得物だけを片付けます。
    Reward = GetWorld()->SpawnActor<APlayseedLantern>(ArenaFloor + FVector(0, 0, 100), FRotator::ZeroRotator);
    if (Reward.IsValid()) Reward->ConfigureReward(1);
}
void AThunderStormkeeper::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bDefeated || IsHeld()) return;
    if (bRecovering) { RecoveryTime -= DeltaSeconds; if (RecoveryTime <= 0) ReturnToArena(); return; }
    if (IsInThrownFlight()) { if (GetActorLocation().Z < ArenaFloor.Z - 500) BeginRecovery(true); return; }
    // E 投擲以外の落下・ダメージによる手放しには得点を与えません。
    if (GetBody()->IsSimulatingPhysics()) { BeginRecovery(false); return; }
    auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)); if (!Player) return;
    const FVector Offset = Player->GetActorLocation() - GetActorLocation();
    const float Feet = Player->GetActorLocation().Z - 82;
    if (FMath::Abs(Feet - ArenaFloor.Z) > 180 || Offset.Size2D() > 900) return;
    FRotator Rotation = GetActorRotation(); Rotation.Yaw = FMath::FixedTurn(Rotation.Yaw, Offset.Rotation().Yaw, 25 * DeltaSeconds); SetActorRotation(Rotation);
    if (Offset.Size2D() > 185)
    {
        FVector Destination = GetActorLocation() + Offset.GetSafeNormal2D() * 90 * DeltaSeconds;
        const FVector Radial = (Destination - ArenaFloor).GetClampedToMaxSize2D(400); Destination.X = ArenaFloor.X + Radial.X; Destination.Y = ArenaFloor.Y + Radial.Y;
        SetActorLocation(Destination, true);
    }
    if (Offset.Size2D() < 140 && Feet < GetActorLocation().Z + 50) Player->GetVitality()->TryTakeImpact(2, Offset);
}
void AThunderStormkeeper::ResetToSpawn()
{
    if (Reward.IsValid()) Reward->Destroy(); Reward.Reset(); ValidThrows = FrontCounters = OffArenaReturns = 0; bDefeated = false;
    SetActorHiddenInGame(false); SetActorEnableCollision(true); ReturnToArena();
}
