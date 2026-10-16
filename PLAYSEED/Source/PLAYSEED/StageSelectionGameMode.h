#pragma once
#include "CoreMinimal.h"
#include "PlayseedGameModeBase.h"
#include "StageSelectionGameMode.generated.h"
// 古い起動リンクを壊さない入口です。ステージ選択UIは持たず、コースの目的選択へ進みます。
UCLASS()
class PLAYSEED_API AStageSelectionGameMode : public APlayseedGameModeBase
{
    GENERATED_BODY()
public:
    AStageSelectionGameMode();
    virtual void StartPlay() override;
};
