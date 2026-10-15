#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderMeadowStage.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;

// Actor は World 内の一つの存在です。この Actor が地形・装飾の Component を所有します。
// ミッションのルールやセーブは GameMode / Subsystem に任せ、形状の生成だけを担当します。
UCLASS()
class PLAYSEED_API AThunderMeadowStage : public AActor
{
    GENERATED_BODY()
public:
    AThunderMeadowStage();
    virtual void BeginPlay() override;
private:
    UStaticMeshComponent* AddShape(UStaticMesh* Mesh, FVector Position, FVector Dimensions, FLinearColor Color, bool bSolid = false);
    void AddTerrace(FVector TopCenter, FVector2D Size, FLinearColor GrassColor);
    void AddTree(FVector Position);
    void AddSign(FVector Position, const FString& Text);
    void BuildMeadow();
    void BuildLighting();

    // UPROPERTY で素材・メッシュへの参照を UE に伝え、使用中の資産をGCから守ります。
    UPROPERTY() TObjectPtr<UStaticMesh> Cube;
    UPROPERTY() TObjectPtr<UStaticMesh> Sphere;
    UPROPERTY() TObjectPtr<UStaticMesh> Cylinder;
    UPROPERTY() TObjectPtr<UMaterialInterface> Palette;
    UPROPERTY() TObjectPtr<UMaterialInterface> SkyMaterial;
};
