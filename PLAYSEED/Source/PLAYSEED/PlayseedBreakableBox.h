#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayseedStrikeTarget.h"
#include "PlayseedBreakableBox.generated.h"

class AWindseedCharacter;
class UPlayseedActionComponent;
class UStaticMeshComponent;
class APlayseedCoin;

// World所有の箱です。攻撃の受け手になり、壊れた後の内容物も自分で追跡します。
UCLASS()
class PLAYSEED_API APlayseedBreakableBox : public AActor, public IPlayseedStrikeTarget
{
    GENERATED_BODY()
public:
    APlayseedBreakableBox();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual bool ReceiveStrike(AWindseedCharacter* Player, int32 Damage) override;
    virtual void ResetBox();
    void ConfigureBox(FName Id) { BoxId = Id; }
    bool IsBroken() const { return bBroken; }
    int32 GetBreakCount() const { return BreakCount; }
    int32 GetDropGenerationCount() const { return DropGenerationCount; }
    const TArray<TWeakObjectPtr<APlayseedCoin>>& GetDropCoins() const { return DropCoins; }
protected:
    // 後続の報酬箱はここを上書きでき、破壊・重複防止の処理を再利用できます。
    virtual void SpawnContents(AWindseedCharacter* Player);
    UStaticMeshComponent* GetBoxBody() const { return Body; }
    const TArray<TObjectPtr<UStaticMeshComponent>>& GetBoxBands() const { return Bands; }
private:
    void HandlePound(FVector ImpactPoint);
    void BreakBox(AWindseedCharacter* Player);
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Body;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Bands;
    // AddUObjectの購読は弱参照です。こちらも部品の寿命を延ばさず追跡します。
    TWeakObjectPtr<UPlayseedActionComponent> BoundActions;
    TArray<TWeakObjectPtr<APlayseedCoin>> DropCoins;
    FName BoxId;
    int32 BreakCount = 0;
    int32 DropGenerationCount = 0;
    bool bBroken = false;
};
