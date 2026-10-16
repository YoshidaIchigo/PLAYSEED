#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderHundredTestSubsystem.generated.h"
class AWindseedCharacter;
class AThunderMeadowStage;
UCLASS()
class PLAYSEED_API UThunderHundredTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage, bool bReadProcess);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    TWeakObjectPtr<AThunderMeadowStage> Stage;
    FString Error;
    int32 Phase = 0, RecoveryBefore = 0;
    float PhaseTime = 0;
    bool bSawNinetyNine = false;
};
