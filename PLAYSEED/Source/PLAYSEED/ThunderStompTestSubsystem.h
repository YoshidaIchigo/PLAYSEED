#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderStompTestSubsystem.generated.h"

class AWindseedCharacter;
class APlayseedEnemyBase;

// 通常の歩行とジャンプで、側面接触と上からの踏みつけを比較する検証です。
UCLASS()
class PLAYSEED_API UThunderStompTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, APlayseedEnemyBase* Enemy);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    bool WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds);
    TWeakObjectPtr<APlayseedEnemyBase> Enemy;
    FString Error;
    int32 Phase = 0;
    int32 ApproachLeg = 0;
    int32 Loop = 0;
    int32 RecoveryBaseline = 0;
    int32 HitBaseline = 0;
    int32 StompBaseline = 0;
    float PhaseTime = 0;
    bool bLiveSaved = false;
    bool bImpactSaved = false;
    bool bLoggedPass = false;
};
