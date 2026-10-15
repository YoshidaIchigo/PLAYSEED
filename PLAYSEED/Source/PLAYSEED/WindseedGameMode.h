#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "WindseedGameMode.generated.h"

class AWindseedStage;
class FAVIWriter;

UCLASS()
class PLAYSEED_API AWindseedHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};

UCLASS()
class PLAYSEED_API AWindseedGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    AWindseedGameMode();
    virtual void StartPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UPROPERTY() TObjectPtr<AWindseedStage> Stage;
    bool bSmokeTest=false;
    bool bCaptureRequested=false;
    int32 TestPhase=0;
    int32 Waypoint=0;
    int32 JumpCount=0;
    float PhaseTime=0;
    float LegTime=0;
    float CaptureTime=0;
    float NextCapture=0;
    TArray<FVector> Route;
    FAVIWriter* CaptureWriter=nullptr;
    void CaptureFrame(float DeltaSeconds);
    void FinishTest(bool bPassed, const FString& Message);
};
