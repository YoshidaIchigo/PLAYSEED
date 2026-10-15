#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderRideTestSubsystem.generated.h"

class AWindseedCharacter;
class AThunderMeadowStage;
class APlayseedGlideBoard;

// 生成・乗降・坂道・被弾・各アクション中断を、実際の移動と入力でつなぐ検証です。
UCLASS()
class PLAYSEED_API UThunderRideTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    bool Walk(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds);
    bool FollowRoute(AWindseedCharacter* Player, float DeltaSeconds);
    bool AcquireAndMount(AWindseedCharacter* Player, float DeltaSeconds);
    bool NormalMovementRestored(AWindseedCharacter* Player) const;
    void SetPhase(int32 Number);
    TWeakObjectPtr<AThunderMeadowStage> Stage;
    TWeakObjectPtr<APlayseedGlideBoard> Board;
    TArray<FVector> Route;
    FString Error;
    FVector CoastStart = FVector::ZeroVector;
    int32 Phase = 0;
    int32 Waypoint = 0;
    int32 RecoveryBeforeReset = 0;
    int32 RecoveryBeforeFall = 0;
    int32 DefeatBeforeFall = 0;
    int32 ImpactBefore = 0;
    int32 PoundBefore = 0;
    float PhaseTime = 0;
    float JumpHold = 0;
    float HighestFeet = 0;
    float NormalFriction = 0;
    float NormalBraking = 0;
    float NormalAcceleration = 0;
    float NormalAirControl = 0;
    bool bAcquireStrikeSent = false;
    bool bJumpHeld = false;
    bool bSlopeScreenshot = false;
};
