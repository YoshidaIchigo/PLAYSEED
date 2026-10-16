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
// 左右で候補を変え、決定した時だけSubsystemへ番号を渡します。保存のIDとは分離します。
UCLASS()
class PLAYSEED_API AThunderMissionSelectionGameMode : public APlayseedGameModeBase
{
    GENERATED_BODY()
public:
    AThunderMissionSelectionGameMode();
    virtual void StartPlay() override;
    int32 GetHighlightedMission() const { return HighlightedMission; }
    int32 GetVisibleMissionCount() const;
    bool IsMissionSelectable(int32 Number) const;
private:
    int32 GetFirstUnfinishedMission() const;
    void EnterMission(int32 Number);
    void Confirm() { EnterMission(HighlightedMission); }
    void Left() { Navigate(-1); }
    void Right() { Navigate(1); }
    void Navigate(int32 Direction);
    void Stick(float Value);
    void ChooseOne() { EnterMission(1); }
    void ChooseTwo() { EnterMission(2); }
    void ChooseThree() { EnterMission(3); }
    void ChooseFour() { EnterMission(4); }
    void ChooseFive() { EnterMission(5); }
    void ChooseSix() { EnterMission(6); }
    int32 HighlightedMission=1;
    float NextStickTime=0;
    bool bTravelRequested=false;
};
