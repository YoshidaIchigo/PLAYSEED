#pragma once
#include "CoreMinimal.h"
#include "PlayseedCarryable.h"
#include "PlayseedStrikeTarget.h"
#include "ReferenceBomb.generated.h"
class APlayseedCoin;
class USceneComponent;
class UStaticMeshComponent;

// 小型爆弾は既存のE入力で持つ・投げる物です。主人公を直接動かしません。
UCLASS()
class PLAYSEED_API AReferenceBomb : public APlayseedCarryable, public IPlayseedStrikeTarget
{
    GENERATED_BODY()
public:
    AReferenceBomb();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual bool CanBePickedUp(AWindseedCharacter* Player) const override;
    virtual bool TryPickup(AWindseedCharacter* Player) override;
    virtual void ResetToSpawn() override;
    virtual bool ReceiveStrike(AWindseedCharacter* Player,int32 Damage) override;
    void ConfigureStationary(bool Value) { bStationary=Value; }
    bool IsStationary() const { return bStationary; }
    bool IsFuseLit() const { return bFuseLit; }
    bool HasExploded() const { return State>=3; }
    float GetFuseAge() const { return FuseAge; }
    FVector GetHomeLocation() const { return Home; }
    int32 GetChaseStarts() const { return ChaseStarts; }
    int32 GetExplosionCount() const { return ExplosionCount; }
    int32 GetExplosionDamageCount() const { return ExplosionDamageCount; }
    int32 GetDropGenerationCount() const { return DropGenerationCount; }
    APlayseedCoin* GetDropCoin() const { return DropCoin.Get(); }
protected:
    virtual void OnThrownLanding(const FHitResult& Hit) override;
    virtual FVector GetThrowVelocity(const AWindseedCharacter* Player) const override;
private:
    void MoveGround(FVector Target,float Speed,float DeltaSeconds);
    void BeginGrow();
    void Burst();
    void Restore(bool bNewAttempt);
    UFUNCTION() void HandleKickedHit(UPrimitiveComponent*,AActor*,UPrimitiveComponent*,FVector,const FHitResult& Hit);
    UPROPERTY() TObjectPtr<USceneComponent> Visual;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Parts;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Feet;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Spark;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Flash;
    TArray<FLinearColor> Colors;
    TWeakObjectPtr<APlayseedCoin> DropCoin;
    FVector Home=FVector::ZeroVector;
    FRotator HomeRotation=FRotator::ZeroRotator;
    // State: 0巡回、1追跡、2膨張、3爆発、4遠方での再出現待ち。
    int32 State=0,Revision=0,ChaseStarts=0,ExplosionCount=0,ExplosionDamageCount=0,DropGenerationCount=0;
    float FuseAge=0,StateAge=0,WalkAge=0,PatrolAge=0;
    bool bStationary=false,bFuseLit=false,bCoinIssued=false,bExplosionHit=false,bKicked=false;
};
