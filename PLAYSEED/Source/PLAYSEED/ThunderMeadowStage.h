#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderMeadowStage.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;
class UTextRenderComponent;
class AThunderMovingPlatform;
class AThunderTiltingBridge;
class AThunderShortcutGate;
class AThunderWarpPair;
class AThunderDamageThistle;
class APlayseedCarryable;
class APlayseedClimbTree;

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
    AThunderTiltingBridge* GetTiltingBridge() const { return TiltingBridge; }
    AThunderShortcutGate* GetShortcutGate() const { return ShortcutGate; }
    AThunderWarpPair* GetWarpPair() const { return WarpPair; }
    AThunderDamageThistle* GetDamageThistle() const { return DamageThistle; }
    APlayseedCarryable* GetCarryProp() const { return CarryProp; }
    APlayseedClimbTree* GetClimbTree() const { return ClimbTree; }
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
    void BuildTiltingBridge();
    void BuildShortcutGate();
    void BuildWarpCaves();
    void BuildDamageThistle();
    void BuildCarryProp();
    void BuildClimbTree();
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
    UPROPERTY() TObjectPtr<AThunderTiltingBridge> TiltingBridge;
    UPROPERTY() TObjectPtr<AThunderShortcutGate> ShortcutGate;
    UPROPERTY() TObjectPtr<AThunderWarpPair> WarpPair;
    UPROPERTY() TObjectPtr<AThunderDamageThistle> DamageThistle;
    UPROPERTY() TObjectPtr<APlayseedCarryable> CarryProp;
    UPROPERTY() TObjectPtr<APlayseedClimbTree> ClimbTree;
    TArray<FVector> MountainFootPath;
    TArray<FVector> BridgeFootPath;
    // cm単位の上面中央。発射台や着地判定でも同じ目標を参照できます。
    FVector SkyIslandTop = FVector(-200, 2200, 1400);
};
