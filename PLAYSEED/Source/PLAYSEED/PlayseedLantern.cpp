#include "PlayseedLantern.h"
#include "ThunderExpeditionSubsystem.h"
#include "WindseedCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

APlayseedLantern::APlayseedLantern()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("LanternRoot"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
    auto Part = [this](FName Name, UStaticMesh* Mesh, FVector Position, FVector Scale, FLinearColor Color)
    {
        auto* Shape = CreateDefaultSubobject<UStaticMeshComponent>(Name); Shape->SetupAttachment(RootComponent); Shape->SetStaticMesh(Mesh);
        Shape->SetRelativeLocation(Position); Shape->SetRelativeScale3D(Scale); Shape->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Parts.Add(Shape); Colors.Add(Color);
    };
    Part(TEXT("GoldenSeedLight"), Sphere.Object, FVector::ZeroVector, FVector(.7f, .7f, 1), FLinearColor(1, .75f, .12f));
    Part(TEXT("LanternCap"), Cone.Object, FVector(0, 0, 55), FVector(.9f, .9f, .35f), FLinearColor(.05f, .58f, .5f));
    Part(TEXT("LanternFoot"), Sphere.Object, FVector(0, 0, -48), FVector(.8f, .8f, .2f), FLinearColor(.08f, .28f, .35f));
}
void APlayseedLantern::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette")))
        for (int32 Index = 0; Index < Parts.Num(); ++Index)
        {
            auto* Tint = UMaterialInstanceDynamic::Create(Palette, this); Tint->SetVectorParameterValue(TEXT("Tint"), Colors[Index]); Parts[Index]->SetMaterial(0, Tint);
        }
}
void APlayseedLantern::ConfigureReward(int32 Number)
{
    RestLocation = GetActorLocation();
    if (Number < 1 || Number > UThunderExpeditionSubsystem::TotalRewardCount) { SetActorHiddenInGame(true); return; }
    RewardNumber = Number;
    if (GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->HasEarnedReward(Number)) { bCollected = true; SetActorHiddenInGame(true); }
}
void APlayseedLantern::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bCollected || RewardNumber == 0) return;
    RetryRemaining = FMath::Max(0.f, RetryRemaining - DeltaSeconds);
    SetActorLocation(RestLocation + FVector(0, 0, FMath::Sin(GetWorld()->GetTimeSeconds() * 3) * 8));
    AddActorLocalRotation(FRotator(0, DeltaSeconds * 65, 0));
    auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (!Player || RetryRemaining > 0 || FVector::Dist2D(Player->GetActorLocation(), GetActorLocation()) > 75 || FMath::Abs(Player->GetActorLocation().Z - GetActorLocation().Z) > 100) return;
    auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    // 一度に複数の報酬が出ても各Actorと固有IDで守り、Worldを自動で出ません。
    // これにより100コインと主ミッションの報酬を同じ入場で両方取れます。
    bCollected = true;
    const bool bNewReward = Progress->TryAwardReward(RewardNumber);
    if (bNewReward || Progress->HasEarnedReward(RewardNumber))
    {
        SetActorHiddenInGame(true);
        if (bNewReward) { ++SuccessfulCollections; UE_LOG(LogTemp, Display, TEXT("LANTERN COLLECTED: actual reward %d, saved total %d."), RewardNumber, Progress->GetEarnedRewardCount()); }
    }
    else { bCollected = false; RetryRemaining = 1; } // 保存失敗なら消さずに再試行できます。
}
