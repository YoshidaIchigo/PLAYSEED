#pragma once

#include "CoreMinimal.h"
#include "PlayseedCarryable.h"
#include "ThunderStormkeeper.generated.h"

class APlayseedLantern;
class UStaticMesh;

// 山頂の番人。持つ・投げる物理は既存の Carryable に任せ、背後判定と勝敗だけを追加します。
UCLASS()
class PLAYSEED_API AThunderStormkeeper : public APlayseedCarryable
{
    GENERATED_BODY()
public:
    AThunderStormkeeper();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual bool CanBePickedUp(AWindseedCharacter* Player) const override;
    virtual bool TryPickup(AWindseedCharacter* Player) override;
    virtual void ResetToSpawn() override;
    int32 GetValidThrows() const { return ValidThrows; }
    int32 GetFrontCounters() const { return FrontCounters; }
    int32 GetOffArenaReturns() const { return OffArenaReturns; }
    bool IsDefeated() const { return bDefeated; }
    bool IsRecovering() const { return bRecovering; }
    APlayseedLantern* GetReward() const { return Reward.Get(); }
    FVector GetArenaFloor() const { return ArenaFloor; }
protected:
    virtual void OnThrownLanding(const FHitResult& Hit) override;
    virtual FVector GetCarryOffset() const override;
    virtual FVector GetThrowVelocity(const AWindseedCharacter* Player) const override;
private:
    bool CanStandAt(FVector Position) const;
    void ReturnToArena();
    void PaintBody();
    void BeginRecovery(bool bOffArena);
    UStaticMeshComponent* AddPart(UStaticMesh* Mesh, FVector Position, FVector Scale, FLinearColor Color);
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Parts;
    TArray<FLinearColor> PartColors;
    TWeakObjectPtr<APlayseedLantern> Reward;
    FVector ArenaFloor = FVector(2200, 2100, 2000);
    int32 ValidThrows = 0;
    int32 FrontCounters = 0;
    int32 OffArenaReturns = 0;
    float RecoveryTime = 0;
    bool bReferenceBoss = false;
    bool bRecoverInPlace = false;
    bool bRecovering = false;
    bool bDefeated = false;
};
