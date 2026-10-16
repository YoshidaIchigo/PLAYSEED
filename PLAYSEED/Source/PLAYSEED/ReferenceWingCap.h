#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReferenceWingCap.generated.h"
class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;
class AWindseedCharacter;

// 箱から出た帽子です。実際のカプセル接触で一度だけ羽根の力を渡します。
UCLASS()
class PLAYSEED_API AReferenceWingCap : public AActor
{
    GENERATED_BODY()
public:
    AReferenceWingCap();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    bool IsCollected() const { return bCollected; }
    int32 GetCollections() const { return Collections; }
private:
    void TryCollect(AWindseedCharacter* Hero);
    UFUNCTION() void HandleOverlap(UPrimitiveComponent* Component,AActor* OtherActor,UPrimitiveComponent* OtherComponent,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& Sweep);
    UPROPERTY() TObjectPtr<USphereComponent> Trigger;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Parts;
    TArray<FLinearColor> Colors;
    float Age=0;
    bool bCollected=false;
    int32 Collections=0;
};
