#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderBindingPost.generated.h"
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UPlayseedActionComponent;
class AThunderBoundSentinel;
class AThunderPrisonGate;
UCLASS()
class PLAYSEED_API AThunderBindingPost : public AActor
{
    GENERATED_BODY()
public:
    AThunderBindingPost();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    void Configure(AThunderBoundSentinel* InSentinel, AThunderPrisonGate* InGate);
    void ResetPost();
    int32 GetPoundCount() const { return PoundCount; }
    float GetTopHeight() const { return FMath::Max(0, 120 - PoundCount * 40); }
private:
    void HandlePound(FVector ImpactPoint);
    void RefreshShape();
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Post;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> Tint;
    TWeakObjectPtr<UPlayseedActionComponent> Actions;
    TWeakObjectPtr<AThunderBoundSentinel> Sentinel;
    TWeakObjectPtr<AThunderPrisonGate> Gate;
    int32 PoundCount = 0;
};
