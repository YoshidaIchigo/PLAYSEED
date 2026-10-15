#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayseedStrikeTarget.h"
#include "PlayseedEnemyBase.generated.h"

class AWindseedCharacter;
class UStaticMesh;
class UStaticMeshComponent;

// 敵の共通の接触判定です。上からの足元の通過と、横からの身体の接触を分けます。
// この段階は静止した練習用の敵で、追跡などのAIは別の作業単位で追加します。
UCLASS()
class PLAYSEED_API APlayseedEnemyBase : public AActor, public IPlayseedStrikeTarget
{
    GENERATED_BODY()
public:
    APlayseedEnemyBase();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void ResetEnemy();
    bool IsDefeated() const { return bDefeated; }
    int32 GetSuccessfulStomps() const { return SuccessfulStomps; }
    virtual bool ReceiveStrike(AWindseedCharacter* Player, int32 Damage) override;
    void ConfigureMeleeDurability(int32 HitPoints);
    int32 GetMeleeHealth() const { return bDefeated ? 0 : MeleeHealth; }
    int32 GetReceivedStrikes() const { return ReceivedStrikes; }
protected:
    virtual void OnDefeated(AWindseedCharacter* Player) {}
private:
    void AddPart(FName Name, UStaticMesh* Mesh, FVector Position, FVector Scale, FLinearColor Color);
    void DefeatFromStomp(AWindseedCharacter* Player);
    void DefeatEnemy(AWindseedCharacter* Player, bool bBounce);
    UPROPERTY() TObjectPtr<USceneComponent> Visual;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Parts;
    TArray<FLinearColor> Colors;
    FTransform SpawnTransform;
    FVector PreviousPlayerPosition = FVector::ZeroVector;
    float PreviousFeetZ = 0;
    float DefeatAge = 0;
    int32 SuccessfulStomps = 0;
    int32 MaximumMeleeHealth = 1;
    int32 MeleeHealth = 1;
    int32 ReceivedStrikes = 0;
    float HurtDisplayRemaining = 0;
    bool bHasPreviousPosition = false;
    bool bDefeated = false;
};
