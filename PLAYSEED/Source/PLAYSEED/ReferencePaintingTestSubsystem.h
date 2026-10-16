#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "ReferencePaintingTestSubsystem.generated.h"
class APlayerController;
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
    float Elapsed=0,JumpHold=0;
    int32 Phase=0;
    bool bEnabled=false,bFinished=false,bJumped=false,bCapturedDive=false;
};
