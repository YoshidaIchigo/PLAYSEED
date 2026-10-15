#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderBalanceTestSubsystem.generated.h"

class AWindseedCharacter;
class AThunderTiltingBridge;

// 通常の歩行とジャンプで橋を渡り、左右の傾斜と R を確認する検証用 Subsystem です。
UCLASS()
class PLAYSEED_API UThunderBalanceTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderTiltingBridge* Bridge);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    bool MoveToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds);
    // GameInstance はレベル間で残るため、World の Actor を強参照で保持しません。
    TWeakObjectPtr<AThunderTiltingBridge> Bridge;
    int32 Phase = 0;
    int32 RecoveryBaseline = 0;
    float PhaseTime = 0;
    FString Error;
    bool bRestartVerified = false;
    bool bSnapshotSaved = false;
};
