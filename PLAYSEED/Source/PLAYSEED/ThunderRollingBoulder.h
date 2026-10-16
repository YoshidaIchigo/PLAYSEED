#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderRollingBoulder.generated.h"

class UStaticMeshComponent;
class AWindseedCharacter;

// World所有の岩球です。決めた道を下り、接触だけを体力Componentへ依頼します。
UCLASS()
class PLAYSEED_API AThunderRollingBoulder : public AActor
{
    GENERATED_BODY()
public:
    AThunderRollingBoulder();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    void ConfigurePath(const TArray<FVector>& FootPath);
    void ConfigureReferencePath(const TArray<FVector>& FootPath,bool bContinuousLoop=false);
    void ResetMotion();
    bool IsWaiting() const { return bWaiting; }
    float GetWaitRemaining() const { return WaitRemaining; }
    float GetTravelledDistance() const { return TravelledDistance; }
    int32 GetCompletedCycles() const { return CompletedCycles; }
    int32 GetRespawnCount() const { return RespawnCount; }
    int32 GetConfirmedHits() const { return ConfirmedHits; }
    FVector GetSpawnCenter() const { return Path.IsEmpty() ? GetActorLocation() : Path[0]; }
private:
    bool IsSpawnClear(AWindseedCharacter* Player) const;
    void CheckContact(FVector Start, FVector End, AWindseedCharacter* Player);
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Stone;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Vein;
    TArray<FVector> Path;
    int32 NextPoint = 1;
    int32 CompletedCycles = 0;
    int32 RespawnCount = 0;
    int32 ConfirmedHits = 0;
    int32 MotionRevision = 0;
    float TravelledDistance = 0;
    float WaitRemaining = 0;
    bool bWaiting = false;
    // 半径と速さは仕掛け側の設定です。検証側は位置・速度を変更しません。
    float Radius = 70;
    float Speed = 450;
    bool bLoop = false;
    bool bReference = false;
};
