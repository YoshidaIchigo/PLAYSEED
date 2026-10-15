#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderChaseTestSubsystem.generated.h"

class AWindseedCharacter;
class APlayseedChasingEnemy;

// 近づく・逃げる・戻る・倒す・拾う、という実際の入力からAIを観察します。
UCLASS()
class PLAYSEED_API UThunderChaseTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, APlayseedChasingEnemy* Enemy);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    bool WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds);
    bool Approach(AWindseedCharacter* Player, float DeltaSeconds);
    TWeakObjectPtr<APlayseedChasingEnemy> Enemy;
    FString Error;
    int32 Phase = 0;
    int32 Waypoint = 0;
    int32 Loop = 0;
    int32 ChaseBaseline = 0;
    int32 ReturnBaseline = 0;
    int32 DropBaseline = 0;
    int32 RecoveryBaseline = 0;
    float PhaseTime = 0;
    bool bDropSaved = false;
};
