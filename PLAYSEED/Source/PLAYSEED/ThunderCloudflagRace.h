#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayseedInteractionTarget.h"
#include "ThunderCloudflagRace.generated.h"
class UStaticMeshComponent;
class APlayseedLantern;

UENUM()
enum class ECloudflagRaceState : uint8 { Idle, Running, AwaitingRival, Won, Lost, Disqualified };

// NPCの徒歩経路と競走ルールを所有します。プレイヤーの移動は一切置き換えません。
UCLASS()
class PLAYSEED_API AThunderCloudflagRace : public AActor, public IPlayseedInteractionTarget
{
    GENERATED_BODY()
public:
    AThunderCloudflagRace();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void Interact(AWindseedCharacter* Player) override;
    void Configure(const TArray<FVector>& MountainPath);
    void ResetRace();
    ECloudflagRaceState GetRaceState() const { return State; }
    bool HasRivalFinished() const { return bRivalFinished; }
    bool HasPlayerReachedFlag() const { return PlayerFinishTime >= 0; }
    bool UsedWarp() const { return bUsedWarp; }
    float GetElapsed() const { return Elapsed; }
    float GetPlayerTime() const { return PlayerFinishTime; }
    float GetRivalTime() const { return RivalFinishTime; }
    FVector GetFlagFloor() const { return FlagFloor; }
    FVector GetStartFloor() const { return StartFloor; }
    APlayseedLantern* GetReward() const { return Reward.Get(); }
    const TCHAR* GetStateLabel() const;
private:
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Parts;
    TArray<FLinearColor> Colors;
    TArray<FVector> Route;
    TWeakObjectPtr<APlayseedLantern> Reward;
    FVector StartFloor = FVector::ZeroVector;
    FVector FlagFloor = FVector(2200, 2100, 2000);
    ECloudflagRaceState State = ECloudflagRaceState::Idle;
    int32 Waypoint = 0;
    int32 LaunchBaseline = 0;
    int32 WarpBaseline = 0;
    float Elapsed = 0;
    float PlayerFinishTime = -1;
    float RivalFinishTime = -1;
    bool bRivalFinished = false;
    bool bUsedWarp = false;
};
