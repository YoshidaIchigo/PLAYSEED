#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderTiltingBridge.generated.h"

class UStaticMeshComponent;

// 橋を一つの Actor として管理し、床になる Mesh Component ごと回転させます。
// CharacterMovement が回転する床に追従するため、プレイヤーを Attach する必要はありません。
UCLASS()
class PLAYSEED_API AThunderTiltingBridge : public AActor
{
    GENERATED_BODY()
public:
    AThunderTiltingBridge();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    void ResetTilt();
    UStaticMeshComponent* GetSurface() const { return Surface; }
    FVector GetDeckPoint(float LongitudinalOffset) const;
private:
    bool bReferenceBridge=false;
    int32 ReferencePitch=0;
    float ReferenceVelocity=0,BaseYaw=0;
    double ReferenceAccumulator=0;
    // UPROPERTY で UE に Component の所有関係を伝え、Actor と同じ寿命で保持します。
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Surface;
};
