#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WindseedCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class PLAYSEED_API AWindseedCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    AWindseedCharacter();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    void ReturnToTrail();
    void SetSprinting(bool bEnabled);
    USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    int32 RecoveryCount = 0;
private:
    UPROPERTY() TObjectPtr<USpringArmComponent> CameraBoom;
    UPROPERTY() TObjectPtr<UCameraComponent> FollowCamera;
    UPROPERTY() TObjectPtr<USceneComponent> SeedBody;
    void MoveForward(float Value);
    void MoveRight(float Value);
    void LookYaw(float Value);
    void LookPitch(float Value);
    void LookYawRate(float Value);
    void LookPitchRate(float Value);
    void Zoom(float Value);
    void StartSprint();
    void StopSprint();
    void RestartTrail();
    void ReleasePointer();
};
