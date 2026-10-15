#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderWarpTestSubsystem.generated.h"

class AWindseedCharacter;
class AThunderWarpPair;

// 通常歩行で入口へ入り、仕掛け自体によるワープを観察する検証です。
UCLASS()
class PLAYSEED_API UThunderWarpTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderWarpPair* Pair, const TArray<FVector>& FootPath);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    bool WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds);
    void SetLowerRoute();
    void SetUpperRoute();
    // World は Pair を所有します。検証は弱参照だけを保持し、レベル退出を妨げません。
    TWeakObjectPtr<AThunderWarpPair> Pair;
    TArray<FVector> Trail;
    TArray<FVector> Route;
    int32 Phase = 0;
    int32 Waypoint = 0;
    int32 RecoveryBaseline = 0;
    float PhaseTime = 0;
    FString Error;
    bool bRestartVerified = false;
    bool bSummitReached = false;
    bool bSnapshotSaved = false;
};
