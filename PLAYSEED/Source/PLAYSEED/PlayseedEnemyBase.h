#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayseedStrikeTarget.h"
#include "PlayseedEnemyBase.generated.h"

class AWindseedCharacter;
class UStaticMesh;
class UStaticMeshComponent;

// 敵の共通の接触判定です。上からの足元の通過と、横からの身体の接触を分けます。
// 基底クラス自身は移動せず、追跡などのAIは派生Actorへ分けます。
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
    virtual int32 GetContactDamage() const { return 2; }
    virtual float GetContactRadius() const { return 70; }
    virtual float GetStompHeight() const { return 80; }
    virtual void OnDefeated(AWindseedCharacter* Player) {}
    // 派生Actorは色と撃破のきっかけだけを変え、共通の描画部品やガードは内部で保ちます。
    virtual FLinearColor GetBudTint() const { return FLinearColor(.25f, .35f, .55f); }
    void DefeatEnemy(AWindseedCharacter* Player, bool bBounce);
    virtual void HandleStomp(AWindseedCharacter* Player);
    void BounceFromStomp(AWindseedCharacter* Player);
    void RefreshBudTint();
private:
    void AddPart(FName Name, UStaticMesh* Mesh, FVector Position, FVector Scale, FLinearColor Color);
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
    float StompContactGrace = 0;
    bool bHasPreviousPosition = false;
    bool bDefeated = false;
};
