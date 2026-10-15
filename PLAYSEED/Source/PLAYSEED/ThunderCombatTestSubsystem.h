#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderCombatTestSubsystem.generated.h"

class AWindseedCharacter;
class APlayseedEnemyBase;

// 通常の接近と攻撃入力を使い、範囲・連続技・重複命中・解除を観察する検証です。
UCLASS()
class PLAYSEED_API UThunderCombatTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, APlayseedEnemyBase* Enemy);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    bool WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds);
    bool WalkFacingRoute(AWindseedCharacter* Player, float DeltaSeconds, bool bStopBeforeLast = false);
    TWeakObjectPtr<APlayseedEnemyBase> Enemy;
    FString Error;
    int32 Phase = 0;
    int32 RouteIndex = 0;
    int32 RecoveryBaseline = 0;
    int32 ReceivedBaseline = 0;
    int32 SuccessfulBaseline = 0;
    int32 SerialBaseline = 0;
    int32 ComboSerial = 0;
    int32 DamageBaseline = 0;
    float PhaseTime = 0;
    bool bSpamSent = false;
    bool bKickSaved = false;
    bool bRecoverySaved = false;
};
