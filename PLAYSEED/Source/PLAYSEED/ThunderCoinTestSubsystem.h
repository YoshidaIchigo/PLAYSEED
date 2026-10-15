#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderCoinTestSubsystem.generated.h"

class AWindseedCharacter;
class AThunderMeadowStage;

// 集計結果を直接書かず、通常歩行で実際の収集球へ触れる検証です。
UCLASS()
class PLAYSEED_API UThunderCoinTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* Stage);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    bool WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds);
    TWeakObjectPtr<AThunderMeadowStage> Stage;
    FString Error;
    int32 Phase = 0;
    int32 CoinIndex = 0;
    int32 RecoveryBaseline = 0;
    int32 RewardsBaseline = 0;
    float PhaseTime = 0;
    bool bCoinsSaved = false;
    bool bTotalSaved = false;
};
