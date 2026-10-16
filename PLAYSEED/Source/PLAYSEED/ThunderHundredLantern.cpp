#include "ThunderHundredLantern.h"
#include "ThunderMeadowGameMode.h"
#include "ThunderCoinLedger.h"
#include "PlayseedCoin.h"
#include "PlayseedLantern.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
AThunderHundredLantern::AThunderHundredLantern() { PrimaryActorTick.bCanEverTick = true; RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HundredLanternRoot")); }
void AThunderHundredLantern::BindLedger()
{
    if (!Ledger.IsValid()) if (auto* Mode = Cast<AThunderMeadowGameMode>(GetWorld()->GetAuthGameMode())) { Ledger = Mode->GetCoinLedger(); Ledger->OnCoinCollected.AddUObject(this, &AThunderHundredLantern::HandleCoin); }
}
void AThunderHundredLantern::BeginPlay() { Super::BeginPlay(); BindLedger(); }
void AThunderHundredLantern::Tick(float DeltaSeconds) { Super::Tick(DeltaSeconds); BindLedger(); }
void AThunderHundredLantern::EndPlay(const EEndPlayReason::Type Reason) { if (Ledger.IsValid()) Ledger->OnCoinCollected.RemoveAll(this); Super::EndPlay(Reason); }
FVector AThunderHundredLantern::FindRewardFloor(FVector CoinLocation) const
{
    FCollisionQueryParams Params(SCENE_QUERY_STAT(HundredLanternFloor), false); Params.AddIgnoredActor(this);
    if (auto* Player = UGameplayStatics::GetPlayerPawn(this, 0)) Params.AddIgnoredActor(Player);
    auto FloorAt = [&](FVector At, FHitResult& Hit) { return GetWorld()->LineTraceSingleByChannel(Hit, At + FVector(0, 0, 400), At - FVector(0, 0, 10000), ECC_Visibility, Params) && Hit.ImpactNormal.Z > .65f; };
    FHitResult Base;
    if (!FloorAt(CoinLocation, Base)) return CoinLocation + FVector(0, 0, 240);
    // 最後のコインの近くの実床へ置きます。200cm離し、出現と実取得を区別できるようにします。
    // 山道では同じ高さに近い床を選び、細い道の外へ置くことを避けます。
    for (const FVector& Offset : {FVector(0, 200, 0), FVector(0, -200, 0), FVector(200, 0, 0), FVector(-200, 0, 0)})
    {
        FHitResult Candidate;
        if (FloorAt(FVector(CoinLocation.X, CoinLocation.Y, Base.ImpactPoint.Z) + Offset, Candidate) && FMath::Abs(Candidate.ImpactPoint.Z - Base.ImpactPoint.Z) <= 100)
            return Candidate.ImpactPoint + FVector(0, 0, 100);
    }
    return Base.ImpactPoint + FVector(0, 0, 240); // 狭い足場なら通常ジャンプで触れられる上方へ置きます。
}
void AThunderHundredLantern::HandleCoin(FName CoinId, EPlayseedCoinKind Kind)
{
    if (!Ledger.IsValid() || Ledger->GetTotal() < 100 || bReachedHundred) return;
    FVector LastCoin = FVector::ZeroVector; bool bFound = false;
    for (TActorIterator<APlayseedCoin> It(GetWorld()); It; ++It) if (It->GetCoinId() == CoinId && It->IsCollected()) { LastCoin = It->GetActorLocation(); bFound = true; break; }
    if (!bFound) return;
    RewardLocation = FindRewardFloor(LastCoin);
    Reward = GetWorld()->SpawnActor<APlayseedLantern>(RewardLocation, FRotator::ZeroRotator);
    if (!Reward.IsValid()) return;
    bReachedHundred = true; TriggerCoinId = CoinId; TriggerTotal = Ledger->GetTotal(); ++GenerationCount; Reward->ConfigureReward(7);
    UE_LOG(LogTemp, Display, TEXT("HUNDRED LANTERN: actual coin %s reached total%d, one reward7 at%s."), *CoinId.ToString(), TriggerTotal, *RewardLocation.ToString());
}
void AThunderHundredLantern::ResetBonus() { if (Reward.IsValid()) Reward->Destroy(); Reward.Reset(); bReachedHundred = false; TriggerCoinId = NAME_None; TriggerTotal = 0; }
