#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WindseedStage.generated.h"

class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;
class UTextRenderComponent;

UCLASS()
class PLAYSEED_API AWindseedStage : public AActor
{
    GENERATED_BODY()
public:
    AWindseedStage();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    void ResetTrail();
    int32 CollectedCount = 0;
    bool bFinished = false;
    float FinishedAt = 0;
    FString Hint = TEXT("Find all six windseeds. Follow the floating gardens.");
    static constexpr int32 SeedCount = 6;
    FVector GoalPosition = FVector(2250,650,400);
private:
    UPROPERTY() TObjectPtr<UStaticMesh> Cube;
    UPROPERTY() TObjectPtr<UStaticMesh> Sphere;
    UPROPERTY() TObjectPtr<UStaticMesh> Cylinder;
    UPROPERTY() TObjectPtr<UStaticMesh> Cone;
    UPROPERTY() TObjectPtr<UMaterialInterface> Palette;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Seeds;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Beacon;
    TArray<FVector> SeedPositions;
    TArray<bool> Collected;
    UStaticMeshComponent* Shape(UStaticMesh* Mesh, FVector Position, FVector Scale, FLinearColor Color, bool bSolid = false);
    void Island(FVector Center, FVector2D Size);
    void Plant(FVector Position, float Scale = 1);
    void Sign(FVector Position, const FString& Text, float Yaw = 180);
};
