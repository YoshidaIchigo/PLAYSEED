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
    void UpdateChain(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void UpdateStarCapture(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC);
    void ObserveFade(APlayerController* PC);
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
};
