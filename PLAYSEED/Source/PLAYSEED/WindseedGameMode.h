#pragma once

#include "CoreMinimal.h"
#include "PlayseedGameModeBase.h"
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
class PLAYSEED_API AWindseedGameMode : public APlayseedGameModeBase
{
    GENERATED_BODY()
public:
    AWindseedGameMode();
    virtual void StartPlay() override;
    virtual void RestartStage(class AWindseedCharacter* Player) override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UPROPERTY() TObjectPtr<AWindseedStage> Stage;
    bool bSmokeTest=false;
    bool bCaptureRequested=false;
    bool bCaptureFailed=false;
    FDelegateHandle CaptureDrawHandle;
    int32 TestPhase=0;
    int32 Waypoint=0;
    int32 JumpCount=0;
    int32 RouteRecoveryBaseline=0;
    FVector InputTestStart=FVector::ZeroVector;
    float PhaseTime=0;
    float LegTime=0;
    float CaptureTime=0;
    float NextCapture=0;
    TArray<FVector> Route;
    FAVIWriter* CaptureWriter=nullptr;
    void CaptureFrame(float DeltaSeconds);
    void CaptureViewport();
    void FinishTest(bool bPassed, const FString& Message);
};
