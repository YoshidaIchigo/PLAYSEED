#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderLifeseed.generated.h"
class USphereComponent;
class UStaticMeshComponent;
class AWindseedCharacter;
UCLASS()
class PLAYSEED_API AThunderLifeseed : public AActor
{
    GENERATED_BODY()
public:
    AThunderLifeseed();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    bool IsCollected() const { return bCollected; }
    int32 GetSuccessfulCollections() const { return Collections; }
private:
    void TryCollect(AWindseedCharacter* Player);
    UFUNCTION() void HandleOverlap(UPrimitiveComponent*,AActor*,UPrimitiveComponent*,int32,bool,const FHitResult&);
    UPROPERTY() TObjectPtr<USphereComponent> Trigger;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Seed;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Leaves;
    bool bCollected = false;
    int32 Collections = 0;
    float Age = 0;
};
