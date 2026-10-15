#include "ThunderExpeditionSubsystem.h"

namespace
{
    // オリジナルの名称と目的文です。番号は選択画面、入場後HUD、後続の達成判定で共通にします。
    const TCHAR* Titles[] = {
        TEXT("The Summit Stormkeeper"), TEXT("Race to the Cloudflag"),
        TEXT("The Lantern Above"), TEXT("Eight Scarlet Chimes"),
        TEXT("Five Sky Sparks"), TEXT("Break the Watcher's Bond")
    };
    const TCHAR* Goals[] = {
        TEXT("Reach the summit and throw the stormkeeper three times."),
        TEXT("Challenge the runner and reach the summit flag first."),
        TEXT("Launch to the sky island and open the golden cache."),
        TEXT("Find eight scarlet coins and claim the meadow lantern."),
        TEXT("Fly through all five unique center sparks in the sky."),
        TEXT("Strike the binding post three times and enter the opened gate.")
    };
}

bool UThunderExpeditionSubsystem::SelectMission(int32 MissionNumber)
{
    if (MissionNumber < 1 || MissionNumber > MainMissionCount) return false;
    SelectedMissionNumber = MissionNumber;
    return true;
}

const TCHAR* UThunderExpeditionSubsystem::GetMissionTitle(int32 MissionNumber)
{
    return MissionNumber >= 1 && MissionNumber <= MainMissionCount ? Titles[MissionNumber - 1] : TEXT("Unknown expedition");
}

const TCHAR* UThunderExpeditionSubsystem::GetMissionGoal(int32 MissionNumber)
{
    return MissionNumber >= 1 && MissionNumber <= MainMissionCount ? Goals[MissionNumber - 1] : TEXT("Choose an expedition to begin.");
}
