#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderBoxTestSubsystem.generated.h"

class AWindseedCharacter;
class APlayseedBreakableBox;

// 通常の歩行・パンチ・ジャンプ・急降下で箱と内容物を確認する検証です。
UCLASS()
class PLAYSEED_API UThunderBoxTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, APlayseedBreakableBox* Box);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    bool WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds);
    TWeakObjectPtr<APlayseedBreakableBox> Box;
    FString Error;
    int32 Phase = 0;
    int32 DropIndex = 0;
    int32 BreakBaseline = 0;
    int32 BatchBaseline = 0;
    int32 RecoveryBaseline = 0;
    int32 PoundBaseline = 0;
    float PhaseTime = 0;
    bool bCacheSaved = false;
    bool bDropsSaved = false;
};
