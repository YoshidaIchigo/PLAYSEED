#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderCampaignValidation.h"
#include "ThunderRaceTestSubsystem.generated.h"
class AWindseedCharacter;
class AThunderMeadowStage;

UCLASS()
class PLAYSEED_API UThunderRaceTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage, bool bReadProcess);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    FThunderCampaignBaseline RewardBaseline;
    void SetPhase(int32 Number);
    bool Walk(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds);
    void BeginWarpRoute();
    bool WalkRoute(AWindseedCharacter* Player, float DeltaSeconds);
    TWeakObjectPtr<AThunderMeadowStage> Stage;
    TArray<FVector> Route;
    FString Error;
    int32 Phase = 0;
    int32 Waypoint = 0;
    int32 RecoveryBefore = 0;
    int32 LaunchBefore = 0;
    float PhaseTime = 0;
    bool bWinningAttempt = false;
};
