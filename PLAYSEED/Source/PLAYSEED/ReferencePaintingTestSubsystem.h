#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "ReferencePaintingTestSubsystem.generated.h"
class APlayerController;
class AWindseedCharacter;
class APlayseedLantern;
class AReferenceWalker;
class AReferenceBomb;
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
    void UpdateReferenceBombs(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateReferenceWalkers(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateReferenceWorld(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateReferenceWaterCannons(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateReferenceRain(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateReferenceBoulders(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateReferenceHundred(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateHundredFlight(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateReferenceDeath(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateReferenceGround(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateReferenceSky(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateReferenceRed(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateReferenceCannon(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateReferenceRace(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateReferenceBoss(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    bool UpdateReferenceRoadAvoidance(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    bool UpdateReferenceRouteBombs(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    bool UpdateReferenceRouteCombat(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateChain(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateStarCapture(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void ObserveFade(APlayerController* PC);
    bool PrepareSaveFailure();
    void UpdateSaveRetry(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    bool bGroundOwnedReplay=false,bHundredOwnedReplay=false,bChainOwnedReplay=false,bDeathTest=false,bDeathVerified=false,bDeathStarted=false;
    int32 HundredGroundTotal=0,HundredNeededSky=0,DeathWaypoint=0,DeathImpactBaseline=0,DeathDefeatBaseline=0,DeathHealthBaseline=0,DeathNeededHits=0;
    float DeathLegElapsed=0;
    bool bRedOwnedReplay=false;
    bool bBossOwnedReplay=false,bBossHeartUsed=false,bBossAvoidJumped=false;
    int32 BossHeartWaypoint=0;
    float BossAvoidRemaining=0;
    FVector BossAvoidTarget=FVector::ZeroVector;
    TWeakObjectPtr<AReferenceWalker> RouteCombatTarget;
    TWeakObjectPtr<AReferenceBomb> RouteBombTarget;
    int32 RouteBombExpectedDrops=0,RouteCombatExpectedDrops=0,RouteBombExplosionBaseline=0;
    float RouteBombElapsed=0,RouteBombAttackClock=0,RouteBombAfterBurst=0;
    bool bRouteBombKicked=false,bRouteBombJumped=false;
    float RouteCombatElapsed=0,RouteAttackClock=0;
    bool bSurfaceTest=false,bWingApproachReady=true;
    FVector WingApproachPoint=FVector::ZeroVector;
    bool bBombTest=false,bBombChaseChecked=false,bBombThrowChecked=false,bBombFuseChecked=false,bBombResetChecked=false,bBombHeldPhoto=false;
    int32 BombWaypoint=0;
    float BombLegElapsed=0,BombInputClock=0;
    bool bWalkerTest=false,bWalkerContactChecked=false,bWalkerMeleeChecked=false,bWalkerStompChecked=false,bWalkerResetChecked=false,bWalkerStompJump=false;
    int32 WalkerWaypoint=0,WalkerScoreBaseline=0;
    float WalkerLegElapsed=0,WalkerAttackClock=0;
    bool bWorldTest=false,bWorldGateChecked=false,bWorldGateResetChecked=false,bWorldWestWarpsChecked=false,bWorldMountainWarpsChecked=false,bWorldHeartChecked=false,bWorldRouteJump=false,bWorldFirstVisitDone=false;
    int32 WorldWaypoint=0,WorldWarpPair=1,WorldWarpExpected=0,WorldHealthBaseline=0;
    float WorldLegElapsed=0;
    FVector WorldExitTarget=FVector::ZeroVector;
    bool bWaterCannonTest=false,bFieldWaterCannonChecked=false,bWaterCannonShotsChecked=false,bWaterCannonResetChecked=false,bWaterCannonRouteJump=false,bWaterCannonShotPhoto=false;
    int32 WaterCannonWaypoint=0,WaterCannonShotBaseline=0,WaterCannonHealthBaseline=0;
    float WaterCannonLegElapsed=0;
    bool bRainTest=false,bRainContactVerified=false,bRainDodgeVerified=false,bRainResetVerified=false,bRainBounceShot=false;
    int32 RainWaypoint=0,RainSpawnBaseline=0,RainBurstBaseline=0,RainChosenIndex=1;
    int32 RainApproachHealth=0,RainContactHealth=0,RainAvoidHealth=0;
    float RainLegElapsed=0;
    FVector RainDodgeTarget=FVector::ZeroVector;
    bool bBoulderTest=false,bBoulderPitVerified=false,bBoulderRoadVerified=false,bBoulderResetVerified=false;
    int32 BoulderWaypoint=0,BoulderRoadHitIndex=-1;
    float BoulderLegElapsed=0;
    bool bWingOwnedReplay=false;
    bool bHundredTest=false,bSequentialChainTest=false,bAllStarReadTest=false,bMountainGoldTest=false;
    bool bHundredMountainGold=false,bHundredHighGold=false,bHundredSkyComplete=false;
    int32 HundredWaypoint=0,HundredFlightPoint=0;
    float HundredLegElapsed=0,RedPlatformRideSeconds=0;
    bool bGroundCoinTest=false,bGroundCoinsVerified=false,bGroundResetVerified=false;
    int32 GroundWaypoint=0,GroundPost=-1,GroundDrop=0,GroundCircleAttempts=0;
    float GroundLegElapsed=0;
    FVector GroundRetryTarget=FVector::ZeroVector;
    bool bDefaultStartTest=false;
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
