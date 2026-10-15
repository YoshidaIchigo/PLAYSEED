#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderScarletTestSubsystem.generated.h"
class AWindseedCharacter;
class AThunderMeadowStage;

// 8枚を同じ入場で集める検証です。通常入力・衝突・物理飛行を使います。
UCLASS()
class PLAYSEED_API UThunderScarletTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage, bool bReadProcess);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    void SetPhase(int32 Number);
    void SetRoute(TArray<FVector> Points, int32 NextPhase);
    bool Walk(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds, bool bJump = true);
    bool WalkRoute(AWindseedCharacter* Player, float DeltaSeconds);
    bool CheckCount(int32 Expected);
    TWeakObjectPtr<AThunderMeadowStage> Stage;
    TArray<FVector> Route;
    FString Error;
    FVector LaunchOrigin = FVector::ZeroVector;
    int32 Phase = 0;
    int32 Waypoint = 0;
    int32 RecoveryBefore = 0;
    int32 LaunchBefore = 0;
    float PhaseTime = 0;
    float HighestFeet = 0;
};
