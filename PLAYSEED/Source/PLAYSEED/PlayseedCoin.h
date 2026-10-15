#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayseedCoin.generated.h"

class AWindseedCharacter;
class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;

// 種類と点数を混同しないための列挙型です。保存ファイルには入場中のコインを記録しません。
UENUM()
enum class EPlayseedCoinKind : uint8 { Gold, Scarlet, Azure };

// Worldが所有する収集Actorです。触れた事実をGameModeの集計部品へ渡します。
UCLASS()
class PLAYSEED_API APlayseedCoin : public AActor
{
    GENERATED_BODY()
public:
    APlayseedCoin();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    void ConfigureCoin(FName Id, EPlayseedCoinKind Kind);
    void ResetCoin();
    bool IsCollected() const { return bCollected; }
    FName GetCoinId() const { return CoinId; }
    EPlayseedCoinKind GetCoinKind() const { return CoinKind; }
    static int32 ValueOf(EPlayseedCoinKind Kind);
private:
    void TryCollect(AWindseedCharacter* Player);
    void UpdateTint();
    UFUNCTION() void HandleOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);
    // UPROPERTYで部品への参照をUEへ伝えます。衝突の球と描画の円盤を分けます。
    UPROPERTY() TObjectPtr<USphereComponent> Trigger;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Disc;
    FName CoinId;
    EPlayseedCoinKind CoinKind = EPlayseedCoinKind::Gold;
    float VisualAge = 0;
    bool bCollected = false;
};
