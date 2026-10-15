#include "PlayseedGaleleafEnemy.h"
#include "PlayseedGlideBoard.h"
#include "PlayseedCoin.h"
#include "WindseedCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

APlayseedGaleleafEnemy::APlayseedGaleleafEnemy()
{
    CarriedLeaf = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarriedGaleleaf"));
    CarriedLeaf->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> LeafMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    CarriedLeaf->SetStaticMesh(LeafMesh.Object);
    CarriedLeaf->SetRelativeLocation(FVector(0, 0, 6));
    CarriedLeaf->SetRelativeScale3D(FVector(1.5f, .9f, .18f));
    CarriedLeaf->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APlayseedGaleleafEnemy::BeginPlay()
{
    Super::BeginPlay();
    HomeLocation = GetActorLocation();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        auto* Tint = UMaterialInstanceDynamic::Create(Palette, this);
        Tint->SetVectorParameterValue(TEXT("Tint"), FLinearColor(.04f, .5f, .43f));
        CarriedLeaf->SetMaterial(0, Tint);
    }
}

void APlayseedGaleleafEnemy::ReleaseGaleleaf(AWindseedCharacter* Player)
{
    if (!bHasGaleleaf) return;
    bHasGaleleaf = false;
    ++LeafDrops;
    CarriedLeaf->SetVisibility(false);
    RefreshBudTint();
    const FVector Side = Player ? Player->GetActorRightVector() : FVector::RightVector;
    DroppedGaleleaf = GetWorld()->SpawnActor<APlayseedGlideBoard>(GetActorLocation() + Side * 180, FRotator::ZeroRotator);
}

bool APlayseedGaleleafEnemy::ReceiveStrike(AWindseedCharacter* Player, int32 Damage)
{
    if (IsDefeated() || Damage <= 0 || !Player) return false;
    if (bHasGaleleaf) { ReleaseGaleleaf(Player); return true; }
    return Super::ReceiveStrike(Player, Damage);
}

void APlayseedGaleleafEnemy::HandleStomp(AWindseedCharacter* Player)
{
    if (IsDefeated()) return;
    if (bHasGaleleaf) { ReleaseGaleleaf(Player); BounceFromStomp(Player); }
    else Super::HandleStomp(Player);
}

void APlayseedGaleleafEnemy::OnDefeated(AWindseedCharacter* Player)
{
    // 初回の葉とは別に、倒した後だけ青5点を出します。通知の重複にも備えます。
    if (bHasGaleleaf) ReleaseGaleleaf(Player);
    if (bAzureIssued) return;
    bAzureIssued = true;
    ++AzureDrops;
    auto* Coin = GetWorld()->SpawnActor<APlayseedCoin>(GetActorLocation() + FVector(0, 0, 62), FRotator::ZeroRotator);
    if (!Coin) return;
    Coin->ConfigureCoin(*FString::Printf(TEXT("%s_AzureDrop"), *GetFName().ToString()), EPlayseedCoinKind::Azure);
    AzureCoin = Coin;
}

void APlayseedGaleleafEnemy::FleeOnGround(AWindseedCharacter* Player, float DeltaSeconds)
{
    const FVector Start = GetActorLocation();
    FVector Direction = (Start - Player->GetActorLocation()).GetSafeNormal2D();
    if (Direction.IsNearlyZero()) Direction = GetActorForwardVector();
    FVector Candidate = Start + Direction * (180 * DeltaSeconds);
    const FVector FromHome = Candidate - HomeLocation;
    if (FromHome.Size2D() > 700) Candidate = HomeLocation + FromHome.GetSafeNormal2D() * 700;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(Player);
    FHitResult Wall, Floor;
    if (GetWorld()->SweepSingleByChannel(Wall, Start + FVector(0, 0, 80), Candidate + FVector(0, 0, 80), FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(70), Params)) return;
    if (!GetWorld()->LineTraceSingleByChannel(Floor, Candidate + FVector(0, 0, 150), Candidate - FVector(0, 0, 150), ECC_Visibility, Params) || Floor.ImpactNormal.Z < .7f || FMath::Abs(Floor.ImpactPoint.Z - Start.Z) > 35) return;
    Candidate.Z = Floor.ImpactPoint.Z;
    SetActorLocation(Candidate);
    SetActorRotation(FRotator(0, Direction.Rotation().Yaw, 0));
}

void APlayseedGaleleafEnemy::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (IsDefeated() || bHasGaleleaf) return;
    if (auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
        if (FVector::Dist2D(Player->GetActorLocation(), GetActorLocation()) < 800) FleeOnGround(Player, DeltaSeconds);
}

void APlayseedGaleleafEnemy::ResetEnemy()
{
    // World所有の生成物を、この敵の弱参照だけから片付けます。
    if (DroppedGaleleaf.IsValid()) DroppedGaleleaf->Destroy();
    if (AzureCoin.IsValid()) AzureCoin->Destroy();
    DroppedGaleleaf.Reset();
    AzureCoin.Reset();
    bHasGaleleaf = true;
    bAzureIssued = false;
    CarriedLeaf->SetVisibility(true);
    Super::ResetEnemy();
    RefreshBudTint();
}
