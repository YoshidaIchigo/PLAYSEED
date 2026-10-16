#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayseedBreakableBox.h"
#include "ReferenceCoinField.generated.h"

class APlayseedCoin;
class UPlayseedActionComponent;
class UStaticMeshComponent;

// 木の周囲を実際に3周すると5枚を出します。角度はプレイヤーの実位置から読みます。
UCLASS()
class PLAYSEED_API AReferenceCoinPost : public AActor
{
    GENERATED_BODY()
public:
    AReferenceCoinPost();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    void ConfigurePost(FName Id) { PostId=Id; }
    void ResetPost();
    int32 GetGenerationCount() const { return GenerationCount; }
    int32 GetPoundCount() const { return Pounds; }
    float GetAccumulatedDegrees() const { return TotalAngle; }
    float GetCircleSeconds() const { return CircleSeconds; }
    const TArray<TWeakObjectPtr<APlayseedCoin>>& GetDropCoins() const { return Drops; }
private:
    void HandlePound(FVector ImpactPoint);
    void RefreshShape();
    void SpawnCoins();
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Body;
    TWeakObjectPtr<UPlayseedActionComponent> Actions;
    TArray<TWeakObjectPtr<APlayseedCoin>> Drops;
    FName PostId;
    float PreviousAngle=0,TotalAngle=0,CircleSeconds=0;
    int32 Pounds=0,GenerationCount=0;
    bool bHaveAngle=false;
};

// 既存の3コイン箱の破壊と重複防止を使い、見た目だけ木箱へ合わせます。
UCLASS()
class PLAYSEED_API AReferenceCoinBox : public APlayseedBreakableBox
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
};

UCLASS()
class PLAYSEED_API AReferenceCoinField : public AActor
{
    GENERATED_BODY()
public:
    AReferenceCoinField();
    virtual void BeginPlay() override;
    void ResetCoins();
    const TArray<TObjectPtr<APlayseedCoin>>& GetCoins() const { return Coins; }
    const TArray<TObjectPtr<AReferenceCoinPost>>& GetPosts() const { return Posts; }
    AReferenceCoinBox* GetCoinBox() const { return CoinBox; }
private:
    void AddGroundCoin(FName Id,FVector Near);
    UPROPERTY() TArray<TObjectPtr<APlayseedCoin>> Coins;
    UPROPERTY() TArray<TObjectPtr<AReferenceCoinPost>> Posts;
    UPROPERTY() TObjectPtr<AReferenceCoinBox> CoinBox;
};
