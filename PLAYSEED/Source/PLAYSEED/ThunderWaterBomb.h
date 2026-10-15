#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderWaterBomb.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
enum class EThunderWaterPhase : uint8 { Warning, Falling, Recovering };

// World所有の周期的な仕掛けです。予告・落下・一度だけの着弾を状態で区別します。
UCLASS()
class PLAYSEED_API AThunderWaterBomb : public AActor
{
    GENERATED_BODY()
public:
    AThunderWaterBomb();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    void ResetCycle();
    bool IsWarning() const { return Phase == EThunderWaterPhase::Warning; }
    bool IsFalling() const { return Phase == EThunderWaterPhase::Falling; }
    float GetPhaseAge() const { return PhaseAge; }
    float GetBombHeight() const { return BombHeight; }
    int32 GetWarningSerial() const { return WarningSerial; }
    int32 GetImpactCount() const { return ImpactCount; }
    int32 GetConfirmedHits() const { return ConfirmedHits; }
private:
    void BeginWarning();
    void Impact();
    // 部品と動的素材の参照をUEへ伝え、Actorが使用中の資産を保持します。
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Marker;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Drop;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> MarkerTint;
    EThunderWaterPhase Phase = EThunderWaterPhase::Warning;
    float PhaseAge = 0;
    float BombHeight = 700;
    float FallSpeed = 0;
    int32 WarningSerial = 0;
    int32 ImpactCount = 0;
    int32 ConfirmedHits = 0;
};
