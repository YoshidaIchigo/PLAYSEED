#pragma once

#include "CoreMinimal.h"
#include "PlayseedGameModeBase.h"
#include "GameFramework/HUD.h"
#include "ThunderMissionSelectionGameMode.generated.h"

UCLASS()
class PLAYSEED_API AThunderMissionSelectionHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};

// 画面の入力を受け取り、Subsystem に選択番号を渡してからプレイ用マップを開きます。
UCLASS()
class PLAYSEED_API AThunderMissionSelectionGameMode : public APlayseedGameModeBase
{
    GENERATED_BODY()
public:
    AThunderMissionSelectionGameMode();
    virtual void StartPlay() override;
private:
    void EnterMission(int32 Number);
    void ChooseOne() { EnterMission(1); }
    void ChooseTwo() { EnterMission(2); }
    void ChooseThree() { EnterMission(3); }
    void ChooseFour() { EnterMission(4); }
    void ChooseFive() { EnterMission(5); }
    void ChooseSix() { EnterMission(6); }
    bool bTravelRequested = false;
};
