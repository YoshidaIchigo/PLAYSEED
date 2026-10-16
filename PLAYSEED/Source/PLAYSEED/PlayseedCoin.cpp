#include "PlayseedCoin.h"
#include "ReferenceBattlefieldTerrain.h"
#include "ThunderMeadowGameMode.h"
#include "ThunderCoinLedger.h"
#include "WindseedCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

APlayseedCoin::APlayseedCoin()
{
    PrimaryActorTick.bCanEverTick = true;
    Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("CoinTrigger"));
    RootComponent = Trigger;
    Trigger->InitSphereRadius(52);
    Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Trigger->SetCollisionObjectType(ECC_WorldDynamic);
    Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Trigger->SetGenerateOverlapEvents(true);
    Disc = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LightcoinDisc"));
    Disc->SetupAttachment(Trigger);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CoinMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    Disc->SetStaticMesh(CoinMesh.Object);
    Disc->SetRelativeScale3D(FVector(.64f, .64f, .12f));
    Disc->SetRelativeRotation(FRotator(90, 0, 0));
    Disc->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APlayseedCoin::BeginPlay()
{
    Super::BeginPlay();
    Trigger->OnComponentBeginOverlap.AddDynamic(this, &APlayseedCoin::HandleOverlap);
    UpdateTint();
}

void APlayseedCoin::ConfigureCoin(FName Id, EPlayseedCoinKind Kind)
{
    CoinId = Id;
    CoinKind = Kind;
    UpdateTint();
}

int32 APlayseedCoin::ValueOf(EPlayseedCoinKind Kind)
{
    switch (Kind)
    {
        case EPlayseedCoinKind::Gold: return 1;
        case EPlayseedCoinKind::Scarlet: return 2;
        case EPlayseedCoinKind::Azure: return 5;
        default: return 0;
    }
}

void APlayseedCoin::UpdateTint()
{
    const bool bReference=AReferenceBattlefieldTerrain::IsReferenceCourse();
    // 現行コースは自作の色パレットを使い、照明による緑やピンクへの偏りを避けます。
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr,bReference?TEXT("/Game/Battlefield/M_Actor.M_Actor"):TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        auto* Tint = UMaterialInstanceDynamic::Create(Palette, this);
        const FLinearColor Color = bReference?(CoinKind==EPlayseedCoinKind::Scarlet?FLinearColor(.85f,.005f,.005f):CoinKind==EPlayseedCoinKind::Azure?FLinearColor(.015f,.18f,1):FLinearColor(1,.55f,.005f)):CoinKind == EPlayseedCoinKind::Scarlet ? FLinearColor(1, .12f, .2f) :
            CoinKind == EPlayseedCoinKind::Azure ? FLinearColor(.08f, .4f, 1) : FLinearColor(1, .7f, .06f);
        Tint->SetVectorParameterValue(TEXT("Tint"), Color);
        Disc->SetMaterial(0, Tint);
    }
}

void APlayseedCoin::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bCollected) return;
    VisualAge += DeltaSeconds;
    Disc->SetRelativeRotation(FRotator(90, VisualAge * 100, 0));
    Disc->SetRelativeLocation(FVector(0, 0, FMath::Sin(VisualAge * 3) * 6));
    // 生成時から重なっていた場合も拾えます。通常の高速移動はOverlap通知でも受け取ります。
    auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (Player && Trigger->IsOverlappingActor(Player)) TryCollect(Player);
}

void APlayseedCoin::HandleOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    TryCollect(Cast<AWindseedCharacter>(OtherActor));
}

void APlayseedCoin::TryCollect(AWindseedCharacter* Player)
{
    if (bCollected || !Player) return;
    auto* Mode = Cast<AThunderMeadowGameMode>(GetWorld()->GetAuthGameMode());
    if (!Mode) return;
    // 先にガードを立て、集計側から通知が戻っても同じActorを再取得しません。
    bCollected = true;
    if (!Mode->GetCoinLedger()->TryCollect(CoinId, CoinKind))
    {
        bCollected = Mode->GetCoinLedger()->HasCollected(CoinId);
        if (!bCollected) return;
    }
    // 衝突はOverlapのまま残します。再開時の衝突再有効化による、その場での再取得を避けます。
    SetActorHiddenInGame(true);
}

void APlayseedCoin::ResetCoin()
{
    bCollected = false;
    VisualAge = 0;
    SetActorHiddenInGame(false);
}
