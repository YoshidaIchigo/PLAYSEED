#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderDamageTestSubsystem.generated.h"

class AWindseedCharacter;
class AThunderDamageThistle;

UCLASS()
class PLAYSEED_API UThunderDamageTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderDamageThistle* Thistle);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    void Approach(AWindseedCharacter* Player, float DeltaSeconds);
    TWeakObjectPtr<AThunderDamageThistle> Thistle;
    FString Error;
    int32 Phase = 0;
    int32 RecoveryBaseline = 0;
    int32 HitBaseline = 0;
    int32 BlockBaseline = 0;
    int32 DefeatBaseline = 0;
    int32 RewardBaseline = 0;
    float PhaseTime = 0;
    bool bSnapshotSaved = false;
};
