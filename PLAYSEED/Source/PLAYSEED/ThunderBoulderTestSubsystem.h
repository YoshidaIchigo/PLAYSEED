#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderBoulderTestSubsystem.generated.h"

class AWindseedCharacter;
class AThunderRollingBoulder;

// 低い道で実接触し、端を徒歩で登って安全な再出現も観察する検証です。
UCLASS()
class PLAYSEED_API UThunderBoulderTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderRollingBoulder* Boulder, const TArray<FVector>& FootPath, FVector ThistlePosition);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    bool WalkRoute(AWindseedCharacter* Player, float DeltaSeconds);
    TWeakObjectPtr<AThunderRollingBoulder> Boulder;
    TArray<FVector> Route;
    TArray<FVector> MountainPath;
    FVector Thistle;
    FString Error;
    int32 Phase = 0;
    int32 Waypoint = 0;
    int32 DamageBaseline = 0;
    int32 BoulderHitBaseline = 0;
    int32 RecoveryBaseline = 0;
    int32 RespawnBaseline = 0;
    int32 DefeatBaseline = 0;
    float PhaseTime = 0;
    bool bImpactSaved = false;
    bool bReturnSaved = false;
};
