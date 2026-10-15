#pragma once
#include "CoreMinimal.h"
#include "PlayseedBreakableBox.h"
#include "ThunderSkyCache.generated.h"
class APlayseedLantern;

// 箱の壊れ方は共通基底、浮島固有の内容物だけをここで担当します。
UCLASS()
class PLAYSEED_API AThunderSkyCache : public APlayseedBreakableBox
{
    GENERATED_BODY()
public:
    virtual void ResetBox() override;
    APlayseedLantern* GetReward() const { return Reward.Get(); }
    int32 GetRewardGenerationCount() const { return RewardGenerationCount; }
protected:
    virtual void SpawnContents(AWindseedCharacter* Player) override;
private:
    TWeakObjectPtr<APlayseedLantern> Reward;
    int32 RewardGenerationCount = 0;
};
