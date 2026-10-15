#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayseedFlightComponent.generated.h"

class UStaticMeshComponent;

// Character所有の時間制限つき植物の翼です。座標を置換せず、Flyingモードの衝突移動を使います。
UCLASS()
class PLAYSEED_API UPlayseedFlightComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPlayseedFlightComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    void GrantPower();
    bool TryBeginFlight();
    void RequestFlightAfterLaunch();
    void CancelFlight();
    void ResetPower();
    void AdjustDirection(float YawDelta, float PitchDelta);
    void SetSpeedInput(float Value) { SpeedInput = FMath::Clamp(Value, -1.f, 1.f); }
    void SetTurnInput(float Value) { TurnInput = FMath::Clamp(Value, -1.f, 1.f); }
    bool IsFlying() const { return bFlying; }
    bool HasPower() const { return Remaining > 0; }
    float GetRemaining() const { return Remaining; }
    float GetFlightYaw() const { return FlightYaw; }
    float GetFlightPitch() const { return FlightPitch; }
    int32 GetPowerGrants() const { return PowerGrants; }
    int32 GetFlightStarts() const { return FlightStarts; }
    int32 GetExpirations() const { return Expirations; }
    int32 GetFlightLandings() const { return FlightLandings; }
private:
    bool IsTouchingFloor() const;
    void RefreshWings();
    // NewObjectで作る見た目も、UPROPERTYから参照して使用中のGC回収を防ぎます。
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Wings;
    bool bFlying = false;
    bool bAwaitingLaunch = false;
    bool bPreviousCameraLag = true;
    float Remaining = 0;
    float FlightYaw = 0;
    float FlightPitch = 0;
    float SpeedInput = 0;
    float TurnInput = 0;
    float PreviousFlySpeed = 0;
    float PreviousAcceleration = 0;
    float PreviousBraking = 0;
    float PreviousArm = 0;
    FVector PreviousSocket = FVector::ZeroVector;
    int32 PowerGrants = 0;
    int32 FlightStarts = 0;
    int32 Expirations = 0;
    int32 FlightLandings = 0;
};
