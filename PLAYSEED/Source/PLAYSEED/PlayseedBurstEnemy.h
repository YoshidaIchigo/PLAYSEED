#pragma once

#include "CoreMinimal.h"
#include "PlayseedEnemyBase.h"
#include "PlayseedBurstEnemy.generated.h"

class UStaticMeshComponent;

// 近づくと二秒の導火線が始まる、オリジナルの赤い芽の敵です。
UCLASS()
class PLAYSEED_API APlayseedBurstEnemy : public APlayseedEnemyBase
{
    GENERATED_BODY()
public:
    APlayseedBurstEnemy();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void ResetEnemy() override;
    bool IsFuseActive() const { return bFuseActive; }
    bool HasExploded() const { return bExploded; }
    float GetFuseAge() const { return FuseAge; }
    int32 GetExplosionCount() const { return ExplosionCount; }
    int32 GetBlastHits() const { return BlastHits; }
protected:
    virtual FLinearColor GetBudTint() const override { return FLinearColor(.85f, .22f, .08f); }
    virtual void OnDefeated(AWindseedCharacter* Player) override;
private:
    void Explode(AWindseedCharacter* Player);
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Fuse;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Blast;
    float FuseAge = 0;
    float FlashAge = 0;
    int32 ExplosionCount = 0;
    int32 BlastHits = 0;
    bool bFuseActive = false;
    bool bExploded = false;
};
