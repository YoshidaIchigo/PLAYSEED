#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderGateTestSubsystem.generated.h"

class AWindseedCharacter;
class AThunderShortcutGate;

UCLASS()
class PLAYSEED_API UThunderGateTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderShortcutGate* Gate);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    bool WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds);
    TWeakObjectPtr<AThunderShortcutGate> Gate;
    int32 Phase = 0;
    int32 RecoveryBaseline = 0;
    float PhaseTime = 0;
    FString Error;
    bool bRestartVerified = false;
    bool bSnapshotSaved = false;
};
