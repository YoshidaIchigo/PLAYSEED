#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderMountainRest.generated.h"
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

// Worldが所有する回復の花です。体力の変更はCharacter側の窓口へ依頼します。
UCLASS()
class PLAYSEED_API AThunderMountainRest : public AActor
{
    GENERATED_BODY()
public:
    AThunderMountainRest();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    void ResetRest();
    bool IsReady() const { return Cooldown <= 0 && !bNeedsLeave; }
    float GetCooldown() const { return Cooldown; }
    int32 GetSuccessfulHeals() const { return SuccessfulHeals; }
private:
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Petals;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Bud;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> Tint;
    float Cooldown = 0;
    bool bNeedsLeave = false;
    int32 SuccessfulHeals = 0;
};
