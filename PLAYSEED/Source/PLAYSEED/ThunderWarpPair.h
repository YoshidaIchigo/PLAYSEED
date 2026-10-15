#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderWarpPair.generated.h"

class ACharacter;

// 二つの洞窟を一組として管理する Actor です。見た目の地形は Stage が所有します。
// 到着後はいったん両方の入口から離れるまで再作動させず、往復の暴発を防ぎます。
UCLASS()
class PLAYSEED_API AThunderWarpPair : public AActor
{
    GENERATED_BODY()
public:
    AThunderWarpPair();
    void Configure(FVector LowerFloor, FVector UpperFloor, float LowerExitYaw, float UpperExitYaw);
    virtual void Tick(float DeltaSeconds) override;
    void ResetWarps();
    FVector GetCaveFloor(int32 Index) const { return Floors[FMath::Clamp(Index, 0, 1)]; }
    int32 GetWarpCount() const { return WarpCount; }
    bool HasFailedWarp() const { return bFailedWarp; }
private:
    void WarpPlayer(ACharacter* Player, int32 DestinationIndex);
    FVector Floors[2] = {FVector::ZeroVector, FVector::ZeroVector};
    float ExitYaws[2] = {0, 0};
    float Cooldown = 0;
    int32 WarpCount = 0;
    bool bConfigured = false;
    bool bNeedsExit = false;
    bool bFailedWarp = false;
};
