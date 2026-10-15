#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderBurstTestSubsystem.generated.h"

class AWindseedCharacter;
class APlayseedBurstEnemy;

// 近づいて待つ場合と退避する場合、攻撃とRによる導火線中断を実入力で比べます。
UCLASS()
class PLAYSEED_API UThunderBurstTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, APlayseedBurstEnemy* Enemy);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    bool WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds);
    bool Approach(AWindseedCharacter* Player, float DeltaSeconds);
    TWeakObjectPtr<APlayseedBurstEnemy> Enemy;
    FString Error;
    int32 Phase = 0;
    int32 Cycle = 0;
    int32 Waypoint = 0;
    int32 ExplosionBaseline = 0;
    int32 HitsBaseline = 0;
    int32 RecoveryBaseline = 0;
    float PhaseTime = 0;
    bool bRecoverySaved = false;
};
