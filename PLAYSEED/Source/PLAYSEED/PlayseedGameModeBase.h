#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PlayseedGameModeBase.generated.h"

class AWindseedCharacter;

// 各ステージに共通する最小限のルールです。地形や収集物の種類はここへ持ち込みません。
// Character は「やり直したい」と通知するだけで、何を初期化するかは現在の GameMode が決めます。
UCLASS()
class PLAYSEED_API APlayseedGameModeBase : public AGameModeBase
{
    GENERATED_BODY()
public:
    // virtual により、子クラスは自分のステージを初期化してから共通の復帰処理を呼べます。
    // 引数は操作中の Character です。所有権は World にあり、この関数で生成・破棄しません。
    virtual void RestartStage(AWindseedCharacter* Player);
};
