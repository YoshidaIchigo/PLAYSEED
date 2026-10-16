#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderCampaignValidation.h"
#include "ThunderSkyCacheTestSubsystem.generated.h"
class AWindseedCharacter;
class AThunderMeadowStage;

UCLASS()
class PLAYSEED_API UThunderSkyCacheTestSubsystem : public UGameInstanceSubsystem
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
    TWeakObjectPtr<AThunderMeadowStage> Stage;
    TArray<FVector> Route;
    FString Error;
    FVector LaunchOrigin = FVector::ZeroVector;
    int32 Phase = 0;
    int32 Waypoint = 0;
    int32 RecoveryBefore = 0;
    int32 LaunchBefore = 0;
    float HighestFeet = 0;
    float PhaseTime = 0;
};
