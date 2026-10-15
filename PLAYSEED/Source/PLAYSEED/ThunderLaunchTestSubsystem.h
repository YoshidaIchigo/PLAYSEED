#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderLaunchTestSubsystem.generated.h"

class AWindseedCharacter;
class AThunderMeadowStage;

// 地上の会話から実際の射出・浮島着地までをつなぎます。飛行中の座標は変更しません。
UCLASS()
class PLAYSEED_API UThunderLaunchTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    void SetPhase(int32 Number);
    bool WalkToPad(AWindseedCharacter* Player, float DeltaSeconds);
    bool CameraRestored(AWindseedCharacter* Player) const;
    TWeakObjectPtr<AThunderMeadowStage> Stage;
    FString Error;
    FVector Seat = FVector::ZeroVector;
    FVector Entry = FVector::ZeroVector;
    FVector LastFlightPosition = FVector::ZeroVector;
    FVector NormalSocket = FVector::ZeroVector;
    float NormalArm = 0;
    float NormalPitchMin = 0;
    float NormalPitchMax = 0;
    float BeforeYaw = 0;
    float PhaseTime = 0;
    float FlightDistance = 0;
    float HighestFeet = 0;
    int32 Phase = 0;
    int32 Waypoint = 0;
    int32 LaunchBefore = 0;
    int32 RecoveryBefore = 0;
    bool bFlightScreenshot = false;
};
