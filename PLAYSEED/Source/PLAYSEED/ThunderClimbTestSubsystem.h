#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderClimbTestSubsystem.generated.h"

class AWindseedCharacter;
class APlayseedClimbTree;

UCLASS()
class PLAYSEED_API UThunderClimbTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, APlayseedClimbTree* Tree);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    bool WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds);
    TWeakObjectPtr<APlayseedClimbTree> Tree;
    FString Error;
    int32 Phase = 0;
    int32 RecoveryBaseline = 0;
    float PhaseTime = 0;
    float HeightBeforeDescent = 0;
    float NormalGravity = 0;
    bool bJumpVerified = false;
    bool bJumpAwayVerified = false;
    bool bTopSaved = false;
    bool bRecoverySaved = false;
};
