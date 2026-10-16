#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderFlightSwitch.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UPlayseedActionComponent;

// World所有のスイッチです。実際の急降下着地を購読し、永続の解放だけをSubsystemへ依頼します。
UCLASS()
class PLAYSEED_API AThunderFlightSwitch : public AActor
{
    GENERATED_BODY()
public:
    AThunderFlightSwitch();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    void RefreshFromProgress();
    bool IsActivated() const { return bActivated; }
    int32 GetActivationCount() const { return ActivationCount; }
    float GetTopZ() const { return GetActorLocation().Z + (bActivated ? 20 : 40); }
private:
    void HandlePound(FVector ImpactPoint);
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Button;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> ButtonTint;
    TWeakObjectPtr<UPlayseedActionComponent> BoundActions;
    bool bReferenceSwitch=false;
    bool bActivated = false;
    int32 ActivationCount = 0;
};
