#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReferenceWaterCannon.generated.h"
class UStaticMeshComponent;

// 乗り込む大砲とは別の仕掛けです。最初の目的の個体は発射せず、山側だけが水玉を出します。
UCLASS()
class PLAYSEED_API AReferenceWaterCannon : public AActor
{
    GENERATED_BODY()
public:
    AReferenceWaterCannon();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    void ConfigureCanFire(bool bAllowed) { bCanFire=bAllowed; }
    void ResetCannon();
    bool CanFireWater() const { return bCanFire; }
    bool IsBaseVisible() const { return bVisible; }
    bool IsShotActive() const { return bShotActive; }
    int32 GetShotCount() const { return ShotCount; }
    FVector GetShotPosition() const { return ShotPosition; }
private:
    void UpdateBarrel();
    void Fire();
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Body;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Rim;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Barrel;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Muzzle;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Shot;
    float AimYaw=0,AimPitch=39.375f,TargetYaw=0,TargetPitch=39.375f,Clock=0,ShotAge=0;
    bool bCanFire=true,bVisible=false,bShotActive=false;
    int32 ShotCount=0;
    FVector ShotPosition=FVector::ZeroVector,ShotVelocity=FVector::ZeroVector;
};
