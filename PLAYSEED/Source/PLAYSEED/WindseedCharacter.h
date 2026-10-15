#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WindseedCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UPlayseedVitalityComponent;

// Character は「入力で操作される Pawn」に、カプセル衝突と移動用 Component を加えた UE のクラスです。
// このクラスは移動や見た目を担当し、ステージ固有の開始位置は GameMode から受け取ります。
// UCLASS と GENERATED_BODY は UE の型登録に必要です。生成されるコードを手作業で編集しません。
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
    // 新しいステージを始めるときに GameMode が設定します。設定後の落下復帰も同じ位置を使います。
    void SetTrailStartLocation(const FVector& Location) { TrailStartLocation = Location; }
    void SetSprinting(bool bEnabled);
    USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    UPlayseedVitalityComponent* GetVitality() const { return Vitality; }
    int32 RecoveryCount = 0;
private:
    // UPROPERTY は UE にこの値を認識させる印です。EditAnywhere により Editor でも位置を調整できます。
    // FVector は座標そのものを保持する値型です。初期値は既存ステージの開始位置を保ちます。
    UPROPERTY(EditAnywhere, Category="Recovery")
    FVector TrailStartLocation = FVector(-450,-250,100);

    UPROPERTY() TObjectPtr<USpringArmComponent> CameraBoom;
    UPROPERTY() TObjectPtr<UCameraComponent> FollowCamera;
    UPROPERTY() TObjectPtr<USceneComponent> SeedBody;
    UPROPERTY() TObjectPtr<UPlayseedVitalityComponent> Vitality;
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
