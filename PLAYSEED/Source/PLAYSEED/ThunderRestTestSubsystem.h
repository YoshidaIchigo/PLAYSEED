#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderRestTestSubsystem.generated.h"
class AWindseedCharacter;
class AThunderMeadowStage;
UCLASS()
class PLAYSEED_API UThunderRestTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    TWeakObjectPtr<AThunderMeadowStage> Stage;
    TArray<FVector> Route;
    FString Error;
    int32 Phase = 0, Waypoint = 0, HitsBefore = 0, RecoveryBefore = 0;
    float PhaseTime = 0;
};
