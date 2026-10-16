#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "ReferenceTravelSubsystem.generated.h"
class AWindseedCharacter;
class ACameraActor;
class APlayseedLantern;
class APlayerController;

UENUM()
enum class EReferenceTravelState : uint8 { InCourse, AtPainting, EnteringPainting, SelectingStar, Dropping, Celebrating, Returning };

enum class EReferenceFadePhase : uint8 { None, Out, Covered, In };

// GameInstanceに残る状態です。マップのActorは保存せず、遷移の二重実行だけを防ぎます。
UCLASS()
class PLAYSEED_API UReferenceTravelSubsystem : public UGameInstanceSubsystem,public FTickableGameObject
{
    GENERATED_BODY()
public:
    void ArriveAtPainting();
    bool BeginPainting();
    bool FinishPainting();
    bool StartPaintingFade();
    bool BeginCourse();
    bool FinishDrop();
    bool BeginReturn();
    bool IsTransitionLocked() const;
    bool Celebrate(AWindseedCharacter* Hero,int32 RewardNumber);
    virtual void Tick(float DeltaSeconds) override;
    virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UReferenceTravelSubsystem,STATGROUP_Tickables); }
    virtual bool IsTickable() const override { return IsCelebrating()||IsFading(); }
    virtual bool IsTickableWhenPaused() const override { return true; }
    virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }
    bool IsCelebrating() const { return State==EReferenceTravelState::Celebrating; }
    bool HasStartedStarPose() const { return bPoseStarted; }
    int32 GetLastRewardNumber() const { return LastRewardNumber; }
    float GetCelebrationFade() const;
    float GetTransitionOpacity() const { return FadeOpacity; }
    bool IsFading() const { return FadePhase!=EReferenceFadePhase::None; }
    EReferenceFadePhase GetFadePhase() const { return FadePhase; }
    int32 GetFadeSequence() const { return FadeSequence; }
    int32 GetCompletedFades() const { return CompletedFades; }
    void HoldIncomingInput();
    bool UsesEntrance() const { return bEntranceFlow; }
    EReferenceTravelState GetState() const { return State; }
private:
    EReferenceTravelState State=EReferenceTravelState::InCourse;
    bool bEntranceFlow=false;
    void StartStarPose();
    TWeakObjectPtr<AWindseedCharacter> CapturedHero;
    TWeakObjectPtr<ACameraActor> ShotCamera;
    TWeakObjectPtr<APlayseedLantern> DisplayStar;
    int32 LastRewardNumber=0;
    float CelebrationTime=0,PoseTime=0;
    bool bPoseStarted=false;
    bool BeginFadeTo(FName Destination);
    void UpdateFade(float DeltaSeconds);
    EReferenceFadePhase FadePhase=EReferenceFadePhase::None;
    FName FadeDestination;
    TWeakObjectPtr<UWorld> FadeSourceWorld;
    TWeakObjectPtr<APlayerController> FadeLockedController;
    float FadeClock=0,FadeOpacity=0;
    int32 FadeSequence=0,CompletedFades=0;
};
