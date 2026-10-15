#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ThunderProgressSaveGame.generated.h"

// USaveGame はディスクへ残すデータ専用です。World 内の Actor や一時的なコイン数は保存しません。
// UPROPERTY(SaveGame) を付けた項目を Unreal の保存APIで扱います。
UCLASS()
class PLAYSEED_API UThunderProgressSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame) int32 FormatVersion = 1;
    // 主ミッションは1〜6、100コインの追加報酬は7。重複しない番号だけを保持します。
    UPROPERTY(SaveGame) TArray<int32> EarnedRewardNumbers;
};
