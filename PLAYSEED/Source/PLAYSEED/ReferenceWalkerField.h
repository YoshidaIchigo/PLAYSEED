#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReferenceWalkerField.generated.h"
class AReferenceWalker;

// 単独2体と半径500cmの3体組を3組配置し、Rで同じ実敵を再開します。
UCLASS()
class PLAYSEED_API AReferenceWalkerField : public AActor
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
    void ResetWalkers();
    const TArray<TObjectPtr<AReferenceWalker>>& GetWalkers() const { return Walkers; }
    int32 GetFloorFailures() const { return FloorFailures; }
private:
    UPROPERTY() TArray<TObjectPtr<AReferenceWalker>> Walkers;
    int32 FloorFailures=0;
};
