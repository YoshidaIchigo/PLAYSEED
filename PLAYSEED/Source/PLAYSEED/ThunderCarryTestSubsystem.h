#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderCarryTestSubsystem.generated.h"

class AWindseedCharacter;
class APlayseedCarryable;

UCLASS()
class PLAYSEED_API UThunderCarryTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, APlayseedCarryable* Prop);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    bool WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds);
    TWeakObjectPtr<APlayseedCarryable> Prop;
    FVector ThrowStart;
    int32 Phase = 0;
    int32 RecoveryBaseline = 0;
    int32 PickupBaseline = 0;
    int32 ThrowBaseline = 0;
    int32 LandingBaseline = 0;
    int32 HitBaseline = 0;
    float PhaseTime = 0;
    FString Error;
    bool bSnapshotSaved = false;
};
