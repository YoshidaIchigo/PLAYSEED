#pragma once

#include "CoreMinimal.h"
#include "PlayseedGameModeBase.h"
#include "GameFramework/HUD.h"
#include "WindseedGameMode.generated.h"

class AWindseedStage;
class UPlayseedRecordingSubsystem;

UCLASS()
class PLAYSEED_API AWindseedHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};

UCLASS()
class PLAYSEED_API AWindseedGameMode : public APlayseedGameModeBase
{
    GENERATED_BODY()
public:
    AWindseedGameMode();
    virtual void StartPlay() override;
    virtual void RestartStage(class AWindseedCharacter* Player) override;
    virtual void Tick(float DeltaSeconds) override;
private:
    UPROPERTY() TObjectPtr<AWindseedStage> Stage;
    bool bSmokeTest=false;
    bool bCaptureRequested=false;
    // Subsystem の所有者は GameInstance。UPROPERTY は UE に参照を伝えます。
    UPROPERTY() TObjectPtr<UPlayseedRecordingSubsystem> Recording;
    bool bTravelTest=false;
    bool bCaptureRetainedAfterTravel=false;
    int32 TestPhase=0;
    int32 Waypoint=0;
    int32 JumpCount=0;
    int32 RouteRecoveryBaseline=0;
    FVector InputTestStart=FVector::ZeroVector;
    float PhaseTime=0;
    float LegTime=0;
    TArray<FVector> Route;
    void FinishTest(bool bPassed, const FString& Message);
};
