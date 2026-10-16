#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "ReferencePaintingTestSubsystem.generated.h"
class APlayerController;
class AWindseedCharacter;
class APlayseedLantern;
UCLASS()
class PLAYSEED_API UReferencePaintingTestSubsystem : public UGameInstanceSubsystem,public FTickableGameObject
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Tick(float DeltaSeconds) override;
    virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UReferencePaintingTestSubsystem,STATGROUP_Tickables); }
    virtual bool IsTickable() const override { return bEnabled&&!bFinished; }
    virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }
private:
    void Tap(APlayerController* PC,FKey Key);
    void Finish(bool bPassed,const FString& Message);
    void UpdateReferenceSky(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateReferenceRed(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateReferenceCannon(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateReferenceRace(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateReferenceBoss(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateChain(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateStarCapture(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void ObserveFade(APlayerController* PC);
    bool PrepareSaveFailure();
    void UpdateSaveRetry(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    bool bReferenceSkyTest=false,bWingHubInitiallyUnlocked=false,bWingHubChecked=false,bWingHubJumped=false,bWingPoundSent=false;
    bool bWingPowerObserved=false,bWingFlightObserved=false,bWingCameraHidden=false,bWingAimSent=false,bWingAimShot=false,bWingSlow=false,bWingFlightKeysSet=false;
    int32 WingWaypoint=0,WingCentre=0;
    float WingLegElapsed=0,WingAimStable=0,WingGoalYaw=0,WingGoalPitch=0;
    bool bReferenceRedTest=false,bRedRouteJump=false,bRedPlatformRidden=false,bRedTreeClimbed=false,bRedBridgeRidden=false,bRedBoardAttempt=false,bRedPlatformJump=false,bRedClimbSent=false;
    int32 RedWaypoint=0,RedDescentPoint=0;
    float RedLegElapsed=0;
    bool bReferenceBossTest=false,bBossRouteJump=false,bBossCoaxed=false,bBossThrowObserved=false;
    bool bReferenceCannonTest=false,bGuideSent=false,bGuideConfirmed=false,bAimSent=false,bCannonAimShot=false,bCannonSawFlight=false,bCannonAimHidden=false,bCannonRouteJump=false;
    int32 CannonWaypoint=0;
    float CannonLegElapsed=0,CannonAimStable=0,CannonGoalYaw=0,CannonGoalPitch=0;
    bool bReferenceRaceTest=false,bRaceStartSent=false,bRaceClaimSent=false,bRaceRouteJump=false,bRaceSprintSeen=false;
    int32 BeforeRewardCount=0,AfterRewardCount=1,RaceWaypoint=0;
    float RaceLegElapsed=0;
    int32 ExpectedReward=6,BossWaypoint=0,BossThrows=0;
    float BossLegElapsed=0;
    FVector BossCarryStart=FVector::ZeroVector;
    float Elapsed=0,JumpHold=0;
    int32 Phase=0;
    bool bEnabled=false,bFinished=false,bJumped=false,bCapturedDive=false;
    bool bDropTest=false,bSawDrop=false,bCapturedLanding=false,bSawJump=false;
    bool bChainTest=false,bChainPoundCycle=false,bChainPoundTriggered=false,bChainGateVerified=false;
    int32 ChainWaypoint=0,ChainPounds=0;
    float ChainLegElapsed=0,ChainPoundElapsed=0;
    bool bStarTest=false,bReadMode=false,bMReentryChecked=false,bPoseObserved=false,bPoseShot=false;
    int32 VerifiedDropVisits=0,PoseHealth=0;
    FVector PoseLocation=FVector::ZeroVector;
    TWeakObjectPtr<APlayseedLantern> ObservedStar;
    bool bFadeTest=false;
    TSet<int32> ObservedFadeOut,ObservedFadeCovered,ObservedFadeIn,FadeShots;
    TMap<int32,int32> FadeMenuSelection;
    bool bSaveFailureTest=false,bSaveObstacleCreated=false,bSaveFailureObserved=false,bSaveObstacleRemoved=false;
    FString SaveObstaclePath;
    float RetryOutsideElapsed=0;
    bool bCameraTest=false,bSawCameraHidden=false,bSawCameraRestored=false;
    bool bSelectionTest=false,bSelectionInitialVerified=false,bSelectionEarnedVerified=false,bSelectionInitialShot=false;
};
