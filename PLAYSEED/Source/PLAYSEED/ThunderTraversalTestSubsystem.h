#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderTraversalTestSubsystem.generated.h"

class AWindseedCharacter;

// 検証専用の道順です。実ワールドの衝突・通常移動・Jump を使い、足場間をテレポートしません。
UCLASS()
class PLAYSEED_API UThunderTraversalTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTerraceRoute(AWindseedCharacter* Player);
    bool UpdateTerraceRoute(AWindseedCharacter* Player, float DeltaSeconds);
    bool HasFailed() const { return !Error.IsEmpty(); }
    const FString& GetError() const { return Error; }
private:
    TArray<FVector> Route;
    int32 Waypoint = 0;
    int32 JumpCount = 0;
    int32 RecoveryBaseline = 0;
    float LegTime = 0;
    float FinishTime = 0;
    FString Error;
    bool bScreenshotRequested = false;
};
