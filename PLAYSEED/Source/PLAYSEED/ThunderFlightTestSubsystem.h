#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderFlightTestSubsystem.generated.h"

class AWindseedCharacter;
class AThunderMeadowStage;

// 花の取得から実時間の失効・再取得・発射台飛行・着地まで、通常の入力で実行します。
UCLASS()
class PLAYSEED_API UThunderFlightTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    void SetPhase(int32 Number);
    void SteerTo(AWindseedCharacter* Player, FVector Goal);
    void ClearAxes(AWindseedCharacter* Player);
    bool WalkRoute(AWindseedCharacter* Player, float DeltaSeconds);
    bool SettingsRestored(AWindseedCharacter* Player) const;
    TWeakObjectPtr<AThunderMeadowStage> Stage;
    TArray<FVector> Route;
    FString Error;
    int32 Phase = 0;
    int32 Waypoint = 0;
    int32 OrbitPoint = 0;
    int32 GrantsBefore = 0;
    int32 StartsBefore = 0;
    int32 ExpirationsBefore = 0;
    int32 LandingsBefore = 0;
    int32 LaunchesBefore = 0;
    int32 RecoveryBefore = 0;
    float PhaseTime = 0;
    float PowerStartTime = 0;
    float NormalFlySpeed = 0;
    float NormalAcceleration = 0;
    float NormalBraking = 0;
    float NormalArm = 0;
    FVector NormalSocket = FVector::ZeroVector;
    bool bSlowVerified = false;
    bool bFlightScreenshot = false;
};
