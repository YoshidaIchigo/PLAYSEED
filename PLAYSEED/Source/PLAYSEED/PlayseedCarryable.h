#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayseedCarryable.generated.h"

class AWindseedCharacter;
class UStaticMeshComponent;
class UPrimitiveComponent;

// 持てる物そのものを表す Actor です。入力は持ち主のCarry Componentが担当します。
// 投げた後はMeshの物理移動と衝突を使い、着地を一度だけ観察します。
UCLASS()
class PLAYSEED_API APlayseedCarryable : public AActor
{
    GENERATED_BODY()
public:
    APlayseedCarryable();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual bool CanBePickedUp(AWindseedCharacter* Player) const;
    bool TryPickup(AWindseedCharacter* Player);
    void Throw();
    void Drop();
    void ResetToSpawn();
    bool IsHeldBy(const AWindseedCharacter* Player) const { return bHeld && Holder.Get() == Player; }
    bool IsHeld() const { return bHeld; }
    bool IsInThrownFlight() const { return bThrown; }
    UStaticMeshComponent* GetBody() const { return Body; }
    int32 GetPickupCount() const { return PickupCount; }
    int32 GetThrowCount() const { return ThrowCount; }
    int32 GetThrownLandingCount() const { return ThrownLandingCount; }
protected:
    virtual void OnThrownLanding(const FHitResult& Hit) {}
private:
    void Release(FVector Velocity, bool bWasThrown);
    UFUNCTION() void HandleBodyHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Body;
    // 持ち主を所有せず、破棄された場合に自動で無効になる弱参照にします。
    TWeakObjectPtr<AWindseedCharacter> Holder;
    FTransform SpawnTransform;
    float PawnIgnoreTime = 0;
    bool bHeld = false;
    bool bThrown = false;
    int32 PickupCount = 0;
    int32 ThrowCount = 0;
    int32 ThrownLandingCount = 0;
};
