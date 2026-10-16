#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReferenceBattlefieldTerrain.generated.h"

class UProceduralMeshComponent;

// 描画と当たり判定は別のComponentです。装飾の隙間が歩行面に影響しません。
UCLASS()
class PLAYSEED_API AReferenceBattlefieldTerrain : public AActor
{
    GENERATED_BODY()
public:
    AReferenceBattlefieldTerrain();
    virtual void BeginPlay() override;
    static bool IsReferenceCourse();
    int32 GetCollisionTriangleCount() const { return 1060; }
    bool FindFloor(FVector Near, FVector& Floor) const;
private:
    UPROPERTY() TObjectPtr<UProceduralMeshComponent> Surface;
    UPROPERTY() TObjectPtr<UProceduralMeshComponent> Collision;
};
