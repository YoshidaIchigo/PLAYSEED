#include "ThunderSkyCache.h"
#include "PlayseedLantern.h"
#include "Engine/World.h"

void AThunderSkyCache::SpawnContents(AWindseedCharacter*)
{
    // 基底側で破壊済みを先に立てているため、重複パンチでも一度だけ呼ばれます。
    // コイン用のSuperは呼ばず、World所有のランタン3を一個だけ出します。
    Reward = GetWorld()->SpawnActor<APlayseedLantern>(GetActorLocation() + FVector(0, 0, 100), FRotator::ZeroRotator);
    if (Reward.IsValid()) { Reward->ConfigureReward(3); ++RewardGenerationCount; }
}
void AThunderSkyCache::ResetBox()
{
    if (Reward.IsValid()) Reward->Destroy(); Reward.Reset(); Super::ResetBox();
}
