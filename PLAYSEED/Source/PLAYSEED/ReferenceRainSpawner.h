#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReferenceRainSpawner.generated.h"
class UStaticMeshComponent;
class AWindseedCharacter;
enum class EReferenceRainPhase : uint8 { Idle,Falling,Squash,Splash };

// 一つの出現範囲が一つの水玉を管理します。Rでは自分の水玉だけを取り消します。
UCLASS()
class PLAYSEED_API AReferenceRainSpawner : public AActor
{
    GENERATED_BODY()
public:
    AReferenceRainSpawner();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    void ConfigureRange(float InRange) { Range=InRange; }
    void ResetRain();
    float GetRange() const { return Range; }
    int32 GetSpawnCount() const { return SpawnCount; }
    int32 GetBurstCount() const { return BurstCount; }
    int32 GetConfirmedHits() const { return ConfirmedHits; }
    bool IsActive() const { return Phase==EReferenceRainPhase::Falling||Phase==EReferenceRainPhase::Squash; }
    bool IsFalling() const { return Phase==EReferenceRainPhase::Falling; }
    bool IsIdle() const { return Phase==EReferenceRainPhase::Idle; }
    FVector GetRainPosition() const { return Position; }
    FVector GetRainTarget() const { return Target; }
private:
    bool StartRain(AWindseedCharacter* Hero);
    bool CheckHeroContact(FVector Start,FVector End,AWindseedCharacter* Hero);
    void Burst(FVector At);
    void RefreshShadow();
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Drop;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Glint;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Shadow;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Bubbles;
    EReferenceRainPhase Phase=EReferenceRainPhase::Idle;
    FVector Position=FVector::ZeroVector,Velocity=FVector::ZeroVector,Target=FVector::ZeroVector;
    float Range=950,WaitRemaining=.7f,PhaseAge=0;
    int32 SpawnCount=0,BurstCount=0,ConfirmedHits=0,Bounces=0,Revision=0;
};
