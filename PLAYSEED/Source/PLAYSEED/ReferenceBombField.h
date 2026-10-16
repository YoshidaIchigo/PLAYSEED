#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReferenceBombField.generated.h"
class AReferenceBomb;
UCLASS()
class PLAYSEED_API AReferenceBombField : public AActor
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
    void ResetBombs();
    const TArray<TObjectPtr<AReferenceBomb>>& GetBombs() const { return Bombs; }
    int32 GetFloorFailures() const { return FloorFailures; }
private:
    UPROPERTY() TArray<TObjectPtr<AReferenceBomb>> Bombs;
    int32 FloorFailures=0;
};
