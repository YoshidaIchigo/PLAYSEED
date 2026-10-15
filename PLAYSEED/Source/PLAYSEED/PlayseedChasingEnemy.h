#pragma once

#include "CoreMinimal.h"
#include "PlayseedEnemyBase.h"
#include "PlayseedChasingEnemy.generated.h"

class APlayseedCoin;
enum class EPlayseedChaseState : uint8 { Idle, Chasing, Returning };

// 共通の接触・撃破に、発見・追跡・帰還だけを重ねる敵Actorです。
UCLASS()
class PLAYSEED_API APlayseedChasingEnemy : public APlayseedEnemyBase
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void ResetEnemy() override;
    bool IsChasing() const { return State == EPlayseedChaseState::Chasing; }
    bool IsIdle() const { return State == EPlayseedChaseState::Idle; }
    FVector GetHomeLocation() const { return HomeLocation; }
    int32 GetChaseStarts() const { return ChaseStarts; }
    int32 GetCompletedReturns() const { return CompletedReturns; }
    int32 GetDropGenerationCount() const { return DropGenerationCount; }
    APlayseedCoin* GetDropCoin() const { return DropCoin.Get(); }
protected:
    virtual void OnDefeated(AWindseedCharacter* Player) override;
private:
    void MoveOnGround(FVector Goal, float Speed, float DeltaSeconds);
    EPlayseedChaseState State = EPlayseedChaseState::Idle;
    FVector HomeLocation = FVector::ZeroVector;
    // 生成したコインの寿命はWorldが管理し、Rの片付け用に弱参照で追跡します。
    TWeakObjectPtr<APlayseedCoin> DropCoin;
    int32 ChaseStarts = 0;
    int32 CompletedReturns = 0;
    int32 DropGenerationCount = 0;
    bool bDropIssued = false;
};
