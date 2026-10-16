#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "InputCoreTypes.h"
#include "ReferenceCourseTestSubsystem.generated.h"

class AWindseedCharacter;
class APlayerController;

// 通常の移動・ジャンプ・R・Mを通す地形検証。キャラクター座標や進行値は書き換えません。
UCLASS()
class PLAYSEED_API UReferenceCourseTestSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Tick(float DeltaSeconds) override;
    virtual bool IsTickable() const override { return bEnabled && !bFinished && !IsTemplate(); }
    virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UReferenceCourseTestSubsystem,STATGROUP_Tickables); }
    virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }
private:
    void Finish(bool bPassed,const FString& Message);
    void Tap(APlayerController* PC,FKey Key);
    TArray<FVector> Route;
    int32 Phase=0,Waypoint=0,RecoveryBaseline=0;
    float Elapsed=0,LegElapsed=0,JumpTime=0;
    float JumpStartZ=0,JumpPeak=0,JumpObservationTime=0;
    bool bFlatJumpStarted=false,bFlatJumpAirborne=false,bFlatJumpFinished=false;
    bool bEnabled=false,bFinished=false,bRecording=false;
};
