#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderLivesTestSubsystem.generated.h"
class AWindseedCharacter;
class AThunderMeadowStage;
UCLASS()
class PLAYSEED_API UThunderLivesTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player,AThunderMeadowStage* InStage);
    void UpdateTest(AWindseedCharacter* Player,float DeltaSeconds);
    const FString& GetError() const { return Error; }
    bool HasObservedGameOver() const { return bObservedGameOver; }
private:
    TWeakObjectPtr<AThunderMeadowStage> Stage;
    TArray<FVector> Route;
    FString Error;
    int32 Phase=0, Waypoint=0, HitsBefore=0, DefeatsBefore=0, RecoveryBefore=0, FallIndex=0;
    float PhaseTime=0;
    bool bObservedGameOver=false;
};
