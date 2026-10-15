#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayseedVitalityComponent.generated.h"

// ActorComponent は位置や見た目を持たない機能の部品です。Character が体力の部品を所有します。
// 敵や危険物はこの窓口へ被弾を依頼し、各自で体力や無敵時間を書き換えません。
UCLASS(ClassGroup=(Playseed))
class PLAYSEED_API UPlayseedVitalityComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPlayseedVitalityComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    bool TryTakeImpact(int32 Points, FVector AwayDirection);
    void Defeat();
    void Restore();
    int32 GetHealth() const { return Health; }
    int32 GetMaximumHealth() const { return 8; }
    bool IsInvulnerable() const { return InvulnerabilityRemaining > 0; }
    int32 GetSuccessfulImpacts() const { return SuccessfulImpacts; }
    int32 GetBlockedImpacts() const { return BlockedImpacts; }
    int32 GetDefeatCount() const { return DefeatCount; }
    FVector GetLastImpulse() const { return LastImpulse; }
private:
    // VisibleInstanceOnly は実行中の状態をEditorで観察するためです。SaveGameにはしません。
    UPROPERTY(VisibleInstanceOnly, Category="Vitality") int32 Health = 8;
    float InvulnerabilityRemaining = 0;
    FVector LastImpulse = FVector::ZeroVector;
    // これらはデバッグ用の累計です。Restoreで消さず、二重被弾や死亡の回数を確認できます。
    int32 SuccessfulImpacts = 0;
    int32 BlockedImpacts = 0;
    int32 DefeatCount = 0;
    bool bDefeating = false;
};
