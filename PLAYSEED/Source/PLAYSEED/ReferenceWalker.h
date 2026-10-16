#pragma once
#include "CoreMinimal.h"
#include "PlayseedChasingEnemy.h"
#include "ReferenceWalker.generated.h"
class UStaticMeshComponent;

// 追跡・踏みつけ・通常攻撃・実コインは共通処理を使い、姿と接触値だけを変えます。
UCLASS()
class PLAYSEED_API AReferenceWalker : public APlayseedChasingEnemy
{
    GENERATED_BODY()
public:
    AReferenceWalker();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void ResetEnemy() override;
protected:
    // 通常サイズ1.5倍の接触半径72×1.5cm、頭の高さ50×1.5cmです。
    virtual int32 GetContactDamage() const override { return 1; }
    virtual float GetContactRadius() const override { return 108; }
    virtual float GetStompHeight() const override { return 75; }
    virtual float GetDetectionRadius() const override { return 500; }
    // 現行の主人公の歩行470cm/sに合わせ、走れば距離を離せる追跡速度にします。
    virtual float GetChasingSpeed() const override { return 440; }
    virtual float GetGroundProbeHeight() const override { return 120; }
private:
    void HideOriginalParts();
    UPROPERTY() TObjectPtr<USceneComponent> WalkerVisual;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> WalkerParts;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Feet;
    TArray<FLinearColor> WalkerColors;
    float WalkClock=0;
};
