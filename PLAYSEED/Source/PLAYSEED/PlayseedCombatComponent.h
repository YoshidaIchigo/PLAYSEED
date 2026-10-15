#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayseedCombatComponent.generated.h"

class UInputComponent;

// 入力・三段の連続攻撃・命中済み相手の管理をCharacter所有の部品へまとめます。
UCLASS(ClassGroup=(Playseed))
class PLAYSEED_API UPlayseedCombatComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPlayseedCombatComponent();
    void SetupInput(UInputComponent* Input);
    void BeginMelee();
    void CancelMelee();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    bool IsAttacking() const { return bAttacking; }
    int32 GetComboStage() const { return ComboStage; }
    int32 GetAttackSerial() const { return AttackSerial; }
    int32 GetSuccessfulHits() const { return SuccessfulHits; }
private:
    void CheckTargets();
    // 弱参照なので、攻撃した相手をレベル退出後まで保持しません。
    TSet<TWeakObjectPtr<AActor>> HitTargets;
    float AttackAge = 0;
    float SinceLastAttack = 2;
    float Cooldown = 0;
    int32 ComboStage = 0;
    int32 AttackSerial = 0;
    int32 SuccessfulHits = 0;
    bool bAttacking = false;
};
