#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderGuideTestSubsystem.generated.h"

class AWindseedCharacter;
class AThunderMeadowStage;

// 書込み側は通常の会話で解放し、読込み側は別プロセスで同じ専用セーブを確認します。
UCLASS()
class PLAYSEED_API UThunderGuideTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage, bool bRestoredProcess);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    bool Walk(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds);
    void SetPhase(int32 Number) { Phase = Number; PhaseTime = 0; }
    TWeakObjectPtr<AThunderMeadowStage> Stage;
    FString Error;
    int32 ExpectedRewards = 0;
    int32 Phase = 0;
    int32 Waypoint = 0;
    int32 RecoveryBefore = 0;
    float PhaseTime = 0;
    bool bReadProcess = false;
};
