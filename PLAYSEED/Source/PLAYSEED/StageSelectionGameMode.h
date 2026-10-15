#pragma once

#include "CoreMinimal.h"
#include "PlayseedGameModeBase.h"
#include "GameFramework/HUD.h"
#include "StageSelectionGameMode.generated.h"

// HUD は画面表示、GameMode は入場ルールを担当します。選択画面に操作キャラクターは不要です。
UCLASS()
class PLAYSEED_API AStageSelectionHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};

UCLASS()
class PLAYSEED_API AStageSelectionGameMode : public APlayseedGameModeBase
{
    GENERATED_BODY()
public:
    AStageSelectionGameMode();
    virtual void StartPlay() override;
private:
    void EnterWindseedGarden();
    void EnterThunderMeadow();
    bool bTravelRequested = false;
};
