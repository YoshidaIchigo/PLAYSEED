#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderWaterTestSubsystem.generated.h"

class AWindseedCharacter;
class AThunderWaterBomb;

// 通常歩行で警告円へ入り、被弾・落下中のR・警告からの退避を比較します。
UCLASS()
class PLAYSEED_API UThunderWaterTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderWaterBomb* Bomb);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    bool WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds);
    bool WalkApproach(AWindseedCharacter* Player, float DeltaSeconds);
    TWeakObjectPtr<AThunderWaterBomb> Bomb;
    FString Error;
    int32 Phase = 0;
    int32 Waypoint = 0;
    int32 HitBaseline = 0;
    int32 RecoveryBaseline = 0;
    int32 SeenWarningSerial = 0;
    int32 FirstImpactBaseline = 0;
    int32 CancelImpactCount = 0;
    int32 EscapeImpactBaseline = 0;
    float PhaseTime = 0;
    bool bWarningSaved = false;
    bool bEscapeSaved = false;
};
