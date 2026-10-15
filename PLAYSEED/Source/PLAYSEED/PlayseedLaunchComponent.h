#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayseedLaunchComponent.generated.h"

class AThunderLauncher;
class UInputComponent;

// Character所有の照準・発射担当です。発射後の飛行は通常のCharacterMovementへ戻します。
UCLASS()
class PLAYSEED_API UPlayseedLaunchComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPlayseedLaunchComponent();
    void SetupInput(UInputComponent* Input);
    bool TryBeginAim(AThunderLauncher* Launcher);
    void CancelAim(bool bReturnToEntry = true);
    void AdjustAim(float YawDelta, float PitchDelta);
    void Fire();
    bool IsAiming() const { return bAiming; }
    AThunderLauncher* GetLauncher() const { return Target.Get(); }
    float GetAimYaw() const { return AimYaw; }
    float GetAimPitch() const { return AimPitch; }
    int32 GetLaunchCount() const { return LaunchCount; }
    FVector GetLastLaunchOrigin() const { return LastLaunchOrigin; }
    FVector GetLastLaunchVelocity() const { return LastLaunchVelocity; }
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
    // 台はWorld所有の弱参照です。照準の解除は、参照が失われた場合にも実行します。
    TWeakObjectPtr<AThunderLauncher> Target;
    bool bAiming = false;
    bool bPreviousCameraLag = false;
    float AimYaw = 0;
    float AimPitch = 70;
    float PreviousArmLength = 560;
    float PreviousPitchMin = -65;
    float PreviousPitchMax = 30;
    FVector PreviousSocket = FVector::ZeroVector;
    FVector EntryLocation = FVector::ZeroVector;
    FRotator EntryView = FRotator::ZeroRotator;
    int32 LaunchCount = 0;
    FVector LastLaunchOrigin = FVector::ZeroVector;
    FVector LastLaunchVelocity = FVector::ZeroVector;
};
