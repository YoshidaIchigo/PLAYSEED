#pragma once

#include "CoreMinimal.h"
#include "PlayseedEnemyBase.h"
#include "PlayseedGaleleafEnemy.generated.h"

class APlayseedGlideBoard;
class APlayseedCoin;
class UStaticMeshComponent;

// 初回は葉を手放して生き残り、次の攻撃で倒れる二段階の敵です。
UCLASS()
class PLAYSEED_API APlayseedGaleleafEnemy : public APlayseedEnemyBase
{
    GENERATED_BODY()
public:
    APlayseedGaleleafEnemy();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual bool ReceiveStrike(AWindseedCharacter* Player, int32 Damage) override;
    virtual void ResetEnemy() override;
    bool HasGaleleaf() const { return bHasGaleleaf; }
    APlayseedGlideBoard* GetDroppedGaleleaf() const { return DroppedGaleleaf.Get(); }
    APlayseedCoin* GetAzureCoin() const { return AzureCoin.Get(); }
    int32 GetLeafDrops() const { return LeafDrops; }
    int32 GetAzureDrops() const { return AzureDrops; }
protected:
    virtual void HandleStomp(AWindseedCharacter* Player) override;
    virtual void OnDefeated(AWindseedCharacter* Player) override;
    virtual FLinearColor GetBudTint() const override { return bHasGaleleaf ? FLinearColor(.12f, .65f, .25f) : FLinearColor(.66f, .44f, .12f); }
private:
    void ReleaseGaleleaf(AWindseedCharacter* Player);
    void FleeOnGround(AWindseedCharacter* Player, float DeltaSeconds);
    UPROPERTY() TObjectPtr<UStaticMeshComponent> CarriedLeaf;
    TWeakObjectPtr<APlayseedGlideBoard> DroppedGaleleaf;
    TWeakObjectPtr<APlayseedCoin> AzureCoin;
    FVector HomeLocation = FVector::ZeroVector;
    int32 LeafDrops = 0;
    int32 AzureDrops = 0;
    bool bHasGaleleaf = true;
    bool bAzureIssued = false;
};
