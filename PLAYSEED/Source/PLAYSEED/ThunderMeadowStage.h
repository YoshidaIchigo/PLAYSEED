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
class APlayseedEnemyBase;
class APlayseedCoin;
class APlayseedBreakableBox;
class AThunderRollingBoulder;
class AThunderWaterBomb;
class APlayseedChasingEnemy;
class APlayseedBurstEnemy;
class APlayseedGaleleafEnemy;
class AThunderGuide;
class AThunderLauncher;
class AThunderFlightBloom;
class AThunderFlightSwitch;
class AThunderStormkeeper;
class AThunderCloudflagRace;
class AThunderSkyCache;
class AThunderScarletTrail;
class AThunderSkyChimes;
class AThunderBoundSentinel;
class AThunderBindingPost;
class AThunderPrisonGate;
class AThunderHundredLantern;
class AThunderMountainRest;
class AThunderLifeseed;

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
    APlayseedEnemyBase* GetPracticeEnemy() const { return PracticeEnemy; }
    const TArray<TObjectPtr<APlayseedCoin>>& GetTrainingCoins() const { return TrainingCoins; }
    APlayseedBreakableBox* GetPracticeBox() const { return PracticeBox; }
    const TArray<TObjectPtr<APlayseedCoin>>& GetTrailCoins() const { return TrailCoins; }
    const TArray<FVector>& GetMeadowCoinFootPath() const { return MeadowCoinFootPath; }
    AThunderRollingBoulder* GetRollingBoulder() const { return RollingBoulder; }
    AThunderWaterBomb* GetWaterBomb() const { return WaterBomb; }
    APlayseedChasingEnemy* GetChasingEnemy() const { return ChasingEnemy; }
    APlayseedBurstEnemy* GetBurstEnemy() const { return BurstEnemy; }
    APlayseedGaleleafEnemy* GetGaleleafEnemy() const { return GaleleafEnemy; }
    AThunderGuide* GetGuide() const { return Guide; }
    AThunderLauncher* GetLauncher() const { return Launcher; }
    AThunderFlightBloom* GetFlightBloom() const { return FlightBloom; }
    AThunderFlightSwitch* GetFlightSwitch() const { return FlightSwitch; }
    AThunderStormkeeper* GetStormkeeper() const { return Stormkeeper; }
    AThunderCloudflagRace* GetCloudflagRace() const { return CloudflagRace; }
    AThunderSkyCache* GetSkyCache() const { return SkyCache; }
    AThunderScarletTrail* GetScarletTrail() const { return ScarletTrail; }
    AThunderSkyChimes* GetSkyChimes() const { return SkyChimes; }
    const TArray<FVector>& GetWingTowerFootPath() const { return WingTowerFootPath; }
    AThunderBoundSentinel* GetBoundSentinel() const { return BoundSentinel; }
    AThunderBindingPost* GetBindingPost() const { return BindingPost; }
    AThunderPrisonGate* GetPrisonGate() const { return PrisonGate; }
    AThunderHundredLantern* GetHundredLantern() const { return HundredLantern; }
    AThunderMountainRest* GetMountainRest() const { return MountainRest; }
    AThunderLifeseed* GetLifeseed() const { return Lifeseed; }
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
    void BuildPracticeEnemy();
    void BuildTrainingCoins();
    void BuildPracticeBox();
    void BuildCoinTrail();
    void BuildRollingBoulder();
    void BuildWaterBomb();
    void BuildChasingEnemy();
    void BuildBurstEnemy();
    void BuildGaleleafEnemy();
    void BuildGuideAndLauncher();
    void BuildFlightBloom();
    void BuildWingTower();
    void BuildStormkeeper();
    void BuildCloudflagRace();
    void BuildSkyCache();
    void BuildScarletTrail();
    void BuildSkyChimes();
    void BuildBoundSentinel();
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
    UPROPERTY() TObjectPtr<APlayseedEnemyBase> PracticeEnemy;
    UPROPERTY() TArray<TObjectPtr<APlayseedCoin>> TrainingCoins;
    UPROPERTY() TObjectPtr<APlayseedBreakableBox> PracticeBox;
    UPROPERTY() TArray<TObjectPtr<APlayseedCoin>> TrailCoins;
    UPROPERTY() TObjectPtr<AThunderRollingBoulder> RollingBoulder;
    UPROPERTY() TObjectPtr<AThunderWaterBomb> WaterBomb;
    UPROPERTY() TObjectPtr<APlayseedChasingEnemy> ChasingEnemy;
    UPROPERTY() TObjectPtr<APlayseedBurstEnemy> BurstEnemy;
    UPROPERTY() TObjectPtr<APlayseedGaleleafEnemy> GaleleafEnemy;
    UPROPERTY() TObjectPtr<AThunderGuide> Guide;
    UPROPERTY() TObjectPtr<AThunderLauncher> Launcher;
    UPROPERTY() TObjectPtr<AThunderFlightBloom> FlightBloom;
    UPROPERTY() TObjectPtr<AThunderFlightSwitch> FlightSwitch;
    UPROPERTY() TObjectPtr<AThunderStormkeeper> Stormkeeper;
    UPROPERTY() TObjectPtr<AThunderCloudflagRace> CloudflagRace;
    UPROPERTY() TObjectPtr<AThunderSkyCache> SkyCache;
    UPROPERTY() TObjectPtr<AThunderScarletTrail> ScarletTrail;
    UPROPERTY() TObjectPtr<AThunderSkyChimes> SkyChimes;
    UPROPERTY() TObjectPtr<AThunderBoundSentinel> BoundSentinel;
    UPROPERTY() TObjectPtr<AThunderBindingPost> BindingPost;
    UPROPERTY() TObjectPtr<AThunderPrisonGate> PrisonGate;
    UPROPERTY() TObjectPtr<AThunderHundredLantern> HundredLantern;
    UPROPERTY() TObjectPtr<AThunderMountainRest> MountainRest;
    UPROPERTY() TObjectPtr<AThunderLifeseed> Lifeseed;
    TArray<FVector> WingTowerFootPath;
    TArray<FVector> MeadowCoinFootPath;
    TArray<FVector> MountainFootPath;
    TArray<FVector> BridgeFootPath;
    // cm単位の上面中央。発射台や着地判定でも同じ目標を参照できます。
    FVector SkyIslandTop = FVector(-200, 2200, 1400);
};
