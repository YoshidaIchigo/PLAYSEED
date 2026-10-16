#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderCampaignValidation.h"
#include "ThunderBossTestSubsystem.generated.h"
class AWindseedCharacter;
class AThunderMeadowStage;

// 検証は入力と通常移動だけを使い、敵の命中数や保存データを直接変更しません。
UCLASS()
class PLAYSEED_API UThunderBossTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage, bool bReadProcess);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    FThunderCampaignBaseline RewardBaseline;
    void SetPhase(int32 Number);
    bool Walk(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds, float Tolerance = 16);
    TWeakObjectPtr<AThunderMeadowStage> Stage;
    FString Error;
    int32 Phase = 0;
    int32 RecoveryBefore = 0;
    int32 ThrowsBefore = 0;
    int32 RouteIndex = 0;
    float PhaseTime = 0;
    bool bInvalidThrowDone = false;
    TArray<FVector> CarryRoute;
};
