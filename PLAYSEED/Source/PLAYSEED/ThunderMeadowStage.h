#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderMeadowStage.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;
class UTextRenderComponent;
class AThunderMovingPlatform;

// Actor は World 内の一つの存在です。この Actor が地形・装飾の Component を所有します。
// ミッションのルールやセーブは GameMode / Subsystem に任せ、形状の生成だけを担当します。
UCLASS()
class PLAYSEED_API AThunderMeadowStage : public AActor
{
    GENERATED_BODY()
public:
    AThunderMeadowStage();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    // 山道の足元の順路です。原点に生成する Stage 内で、徒歩の案内・検証に共用します。
    const TArray<FVector>& GetMountainFootPath() const { return MountainFootPath; }
    FVector GetSkyIslandTop() const { return SkyIslandTop; }
    const TArray<FVector>& GetBridgeFootPath() const { return BridgeFootPath; }
    AThunderMovingPlatform* GetMovingPlatform() const { return MovingPlatform; }
    void ResetMechanisms();
private:
    UStaticMeshComponent* AddShape(UStaticMesh* Mesh, FVector Position, FVector Dimensions, FLinearColor Color, bool bSolid = false, FRotator Rotation = FRotator::ZeroRotator);
    void AddTerrace(FVector TopCenter, FVector2D Size, FLinearColor GrassColor);
    void AddTree(FVector Position);
    void AddSign(FVector Position, const FString& Text);
    void BuildMeadow();
    void BuildMountain();
    void BuildSkyIsland();
    void BuildWoodenBridge();
    void BuildMovingPlatform();
    void AddRamp(FVector Start, FVector End, float Width);
    void BuildLighting();

    // UPROPERTY で素材・メッシュへの参照を UE に伝え、使用中の資産をGCから守ります。
    UPROPERTY() TObjectPtr<UStaticMesh> Cube;
    UPROPERTY() TObjectPtr<UStaticMesh> Sphere;
    UPROPERTY() TObjectPtr<UStaticMesh> Cylinder;
    UPROPERTY() TObjectPtr<UStaticMesh> Cone;
    UPROPERTY() TObjectPtr<UMaterialInterface> Palette;
    UPROPERTY() TObjectPtr<UMaterialInterface> SkyMaterial;
    UPROPERTY() TArray<TObjectPtr<UTextRenderComponent>> Signs;
    UPROPERTY() TObjectPtr<AThunderMovingPlatform> MovingPlatform;
    TArray<FVector> MountainFootPath;
    TArray<FVector> BridgeFootPath;
    // cm単位の上面中央。発射台や着地判定でも同じ目標を参照できます。
    FVector SkyIslandTop = FVector(-200, 2200, 1400);
};
