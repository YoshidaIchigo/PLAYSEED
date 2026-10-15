#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderMovingPlatform.generated.h"

class UStaticMeshComponent;

// Actor が足場全体を動かし、Root の Mesh Component が衝突と Character の立つ床になります。
// プレイヤーを Attach して運ばず、CharacterMovement の移動する床の処理を使います。
UCLASS()
class PLAYSEED_API AThunderMovingPlatform : public AActor
{
    GENERATED_BODY()
public:
    AThunderMovingPlatform();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    void ResetMotion();
    UStaticMeshComponent* GetSurface() const { return Surface; }
    FVector GetSurfaceTop() const { return GetActorLocation() + FVector(0, 0, 30); }
private:
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Surface;
    FVector Origin = FVector::ZeroVector;
    float MotionTime = 0;
};
