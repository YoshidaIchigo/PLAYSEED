#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReferenceBoulderField.generated.h"
class AThunderRollingBoulder;

// Worldが鉄球を所有し、このActorが入場内の配置とRでの再開をまとめます。
UCLASS()
class PLAYSEED_API AReferenceBoulderField : public AActor
{
    GENERATED_BODY()
public:
    AReferenceBoulderField();
    virtual void BeginPlay() override;
    void ResetBoulders();
    const TArray<TObjectPtr<AThunderRollingBoulder>>& GetBalls() const { return Balls; }
    int32 GetFloorFailures() const { return FloorFailures; }
private:
    UPROPERTY() TArray<TObjectPtr<AThunderRollingBoulder>> Balls;
    int32 FloorFailures=0;
};
