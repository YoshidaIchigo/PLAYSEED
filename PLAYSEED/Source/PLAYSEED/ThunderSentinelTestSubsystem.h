#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderSentinelTestSubsystem.generated.h"
class AWindseedCharacter;
class AThunderBoundSentinel;
UCLASS()
class PLAYSEED_API UThunderSentinelTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderBoundSentinel* InSentinel);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    bool Walk(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds);
    bool Approach(AWindseedCharacter* Player, float DeltaSeconds);
    TWeakObjectPtr<AThunderBoundSentinel> Sentinel;
    FString Error;
    int32 Phase = 0, Waypoint = 0, LungeBaseline = 0, HitBaseline = 0, RecoveryBaseline = 0;
    float PhaseTime = 0;
};
