#include "PlayseedBreakableBox.h"
#include "PlayseedActionComponent.h"
#include "PlayseedCoin.h"
#include "WindseedCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

APlayseedBreakableBox::APlayseedBreakableBox()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("CacheRoot"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CacheMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CopperCache"));
    Body->SetupAttachment(RootComponent);
    Body->SetStaticMesh(CacheMesh.Object);
    Body->SetRelativeLocation(FVector(0, 0, 50));
    Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Body->SetCollisionResponseToAllChannels(ECR_Block);
    const FVector BandLocations[] = {FVector(0, -51, 50), FVector(0, 0, 102)};
    const FVector BandScales[] = {FVector(1.08f, .06f, .2f), FVector(.2f, 1.08f, .06f)};
    for (int32 Index = 0; Index < 2; ++Index)
    {
        auto* Band = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("TealBand%d"), Index));
        Band->SetupAttachment(RootComponent);
        Band->SetStaticMesh(CacheMesh.Object);
        Band->SetRelativeLocation(BandLocations[Index]);
        Band->SetRelativeScale3D(BandScales[Index]);
        Band->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Bands.Add(Band);
    }
}

void APlayseedBreakableBox::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        auto Paint = [this, Palette](UStaticMeshComponent* Part, FLinearColor Color)
        {
            auto* Material = UMaterialInstanceDynamic::Create(Palette, this);
            Material->SetVectorParameterValue(TEXT("Tint"), Color);
            Part->SetMaterial(0, Material);
        };
        Paint(Body, FLinearColor(.65f, .29f, .12f));
        for (UStaticMeshComponent* Band : Bands) Paint(Band, FLinearColor(.04f, .4f, .42f));
    }
}

void APlayseedBreakableBox::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    // ActorのBeginPlay順に依存せず、操作Pawnが用意されてから着地通知を購読します。
    if (!BoundActions.IsValid())
    {
        if (auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
        {
            BoundActions = Player->GetActions();
            BoundActions->OnPoundImpact.AddUObject(this, &APlayseedBreakableBox::HandlePound);
        }
    }
}

void APlayseedBreakableBox::EndPlay(const EEndPlayReason::Type Reason)
{
    if (BoundActions.IsValid()) BoundActions->OnPoundImpact.RemoveAll(this);
    Super::EndPlay(Reason);
}

bool APlayseedBreakableBox::ReceiveStrike(AWindseedCharacter* Player, int32 Damage)
{
    if (bBroken || Damage <= 0 || !Player) return false;
    BreakBox(Player);
    return true;
}

void APlayseedBreakableBox::HandlePound(FVector ImpactPoint)
{
    const FVector Local = GetActorTransform().InverseTransformPosition(ImpactPoint);
    if (bBroken || FMath::Abs(Local.X) > 65 || FMath::Abs(Local.Y) > 65 || FMath::Abs(Local.Z - 100) > 15) return;
    // 地面の近くで急降下しても壊さず、箱の上面への着地だけを受け付けます。
    BreakBox(Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)));
}

void APlayseedBreakableBox::BreakBox(AWindseedCharacter* Player)
{
    if (bBroken) return;
    bBroken = true;
    ++BreakCount;
    Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetActorHiddenInGame(true);
    SpawnContents(Player);
}

void APlayseedBreakableBox::SpawnContents(AWindseedCharacter*)
{
    ++DropGenerationCount;
    const FVector DropOffsets[] = {FVector(-95, 0, 62), FVector(0, 100, 62), FVector(95, 0, 62)};
    for (int32 Index = 0; Index < 3; ++Index)
    {
        auto* Coin = GetWorld()->SpawnActor<APlayseedCoin>(GetActorLocation() + DropOffsets[Index], FRotator::ZeroRotator);
        if (!Coin) continue;
        const FString Base = BoxId.IsNone() ? GetFName().ToString() : BoxId.ToString();
        Coin->ConfigureCoin(*FString::Printf(TEXT("%s_Drop_%d"), *Base, Index), EPlayseedCoinKind::Gold);
        DropCoins.Add(Coin);
    }
}

void APlayseedBreakableBox::ResetBox()
{
    // この箱が作ったActorだけを除きます。ステージ上のほかのコインには触れません。
    for (const auto& Coin : DropCoins) if (Coin.IsValid()) Coin->Destroy();
    DropCoins.Reset();
    bBroken = false;
    Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SetActorHiddenInGame(false);
}
