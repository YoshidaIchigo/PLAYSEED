#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayseedClimbTree.generated.h"

class UStaticMesh;
class UStaticMeshComponent;

// 幹と頂上の枝の足場を持つ、登れる木です。移動入力はCharacter側の部品が担当します。
UCLASS()
class PLAYSEED_API APlayseedClimbTree : public AActor
{
    GENERATED_BODY()
public:
    APlayseedClimbTree();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    float GetTrunkRadius() const { return bReferenceTree?30:55; }
    float GetTopZ() const { return GetActorLocation().Z + 650; }
private:
    bool bReferenceTree=false;
    void AddPart(FName Name, UStaticMesh* Mesh, FVector Position, FVector Scale, FLinearColor Color, bool bSolid);
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Parts;
    TArray<FLinearColor> Colors;
};
