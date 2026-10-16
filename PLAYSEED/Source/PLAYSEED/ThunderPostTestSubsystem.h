#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderPostTestSubsystem.generated.h"
class AWindseedCharacter;
class AThunderMeadowStage;
UCLASS()
class PLAYSEED_API UThunderPostTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage, bool bReadProcess);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    void Next(int32 Number);
    bool Walk(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds);
    bool Approach(AWindseedCharacter* Player, float DeltaSeconds);
    bool JumpToPost(AWindseedCharacter* Player, float DeltaSeconds, bool bPound);
    TWeakObjectPtr<AThunderMeadowStage> Stage;
    FString Error;
    int32 Phase = 0, Waypoint = 0, Round = 0, RecoveryBaseline = 0, BeforePounds = 0, BreakBaseline = 0;
    float PhaseTime = 0, JumpTime = 0;
    bool bJumpStarted = false, bPoundPressed = false;
};
