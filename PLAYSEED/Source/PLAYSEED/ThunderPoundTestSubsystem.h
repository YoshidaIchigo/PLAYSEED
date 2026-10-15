#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderPoundTestSubsystem.generated.h"

class AWindseedCharacter;

UCLASS()
class PLAYSEED_API UThunderPoundTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    void RecordImpact(FVector Position);
    FString Error;
    int32 Phase = 0;
    int32 PoundBaseline = 0;
    int32 RecoveryBaseline = 0;
    int32 HitBaseline = 0;
    int32 ObservedImpacts = 0;
    float PhaseTime = 0;
    float NormalGravity = 0;
    float NormalAirControl = 0;
    bool bSnapshotSaved = false;
};
