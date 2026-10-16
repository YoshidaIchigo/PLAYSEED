#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderChimeTestSubsystem.generated.h"
class AWindseedCharacter;
class AThunderMeadowStage;

// 実際の翼・発射・マウス軸で飛び、輪の端と中央コインの違いを確認します。
UCLASS()
class PLAYSEED_API UThunderChimeTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage, bool bReadProcess);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    void SetPhase(int32 Number);
    void SetRoute(TArray<FVector> Points, int32 NextPhase);
    bool WalkRoute(AWindseedCharacter* Player, float DeltaSeconds);
    bool FlyRoute(AWindseedCharacter* Player);
    void SteerTo(AWindseedCharacter* Player, FVector Goal);
    void ClearAxes(AWindseedCharacter* Player);
    TWeakObjectPtr<AThunderMeadowStage> Stage;
    TArray<FVector> Route;
    FString Error;
    FVector EdgePrevious = FVector::ZeroVector;
    int32 Phase = 0;
    int32 Waypoint = 0;
    int32 Centre = 0;
    int32 RecoveryBefore = 0;
    int32 GrantsBefore = 0;
    int32 LaunchBefore = 0;
    float PhaseTime = 0;
    bool bEdgeCrossed = false;
};
