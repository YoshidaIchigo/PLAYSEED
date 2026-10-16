#include "ThunderExpeditionSubsystem.h"
#include "ThunderProgressSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
    // 参照コースの6つの目的です。保存の番号は変更せず、日本語の表示名だけを置き換えます。
    const TCHAR* Titles[] = {
        TEXT("やまのうえの ボムキング"), TEXT("はくねつ ノコノコレース"),
        TEXT("そらのしままで ぶっとべ"), TEXT("8まいの あかコイン"),
        TEXT("そらにはばたけ はねマリオ"), TEXT("ワンワンの いぬごやで")
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

void UThunderExpeditionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    if (ChooseSaveSlot()) LoadProgress();
}

bool UThunderExpeditionSubsystem::ChooseSaveSlot()
{
#if !UE_BUILD_SHIPPING
    const bool bSaveTest = FParse::Param(FCommandLine::Get(), TEXT("ProgressWriteTest")) ||
        FParse::Param(FCommandLine::Get(), TEXT("ProgressReadTest")) || FParse::Param(FCommandLine::Get(), TEXT("ThunderValidation"));
    if (bSaveTest)
    {
        FString Candidate;
        bool bValid = FParse::Value(FCommandLine::Get(), TEXT("PlayseedValidationSlot="), Candidate) &&
            Candidate.StartsWith(TEXT("Validation_")) && Candidate.Len() <= 80;
        for (TCHAR Character : Candidate)
            if (!FChar::IsAlnum(Character) && Character != TEXT('_')) bValid = false;
        // 検証で通常の進行を汚さないため、専用スロット名がなければ保存も読込もしません。
        if (!bValid)
        {
            bCanWriteProgress = false;
            ProgressProblem = TEXT("Validation needs a separate Validation_ save slot.");
            return false;
        }
        SaveSlotName = Candidate;
    }
#endif
    return true;
}

bool UThunderExpeditionSubsystem::IsValidProgress(const UThunderProgressSaveGame* Data) const
{
    if (!Data || Data->FormatVersion != 1) return false;
    TSet<int32> UniqueNumbers;
    for (int32 Number : Data->EarnedRewardNumbers)
    {
        if (Number < 1 || Number > TotalRewardCount || UniqueNumbers.Contains(Number)) return false;
        UniqueNumbers.Add(Number);
    }
    return true;
}

void UThunderExpeditionSubsystem::LoadProgress()
{
    if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
    {
        Progress = Cast<UThunderProgressSaveGame>(UGameplayStatics::CreateSaveGameObject(UThunderProgressSaveGame::StaticClass()));
        return;
    }
    auto* Loaded = Cast<UThunderProgressSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
    if (!IsValidProgress(Loaded))
    {
        // 読めない既存データを空の進行で上書きしません。原因をHUDへ渡して保存を止めます。
        bCanWriteProgress = false;
        ProgressProblem = TEXT("Progress could not be loaded. Existing save preserved.");
        UE_LOG(LogTemp, Warning, TEXT("THUNDER PROGRESS: existing save was not loaded and will not be overwritten."));
        return;
    }
    Progress = Loaded;
}

bool UThunderExpeditionSubsystem::HasEarnedReward(int32 RewardNumber) const
{
    return Progress && Progress->EarnedRewardNumbers.Contains(RewardNumber);
}

int32 UThunderExpeditionSubsystem::GetEarnedRewardCount() const
{
    return Progress ? Progress->EarnedRewardNumbers.Num() : 0;
}

bool UThunderExpeditionSubsystem::TryAwardReward(int32 RewardNumber)
{
    if (!bCanWriteProgress || !Progress || RewardNumber < 1 || RewardNumber > TotalRewardCount || HasEarnedReward(RewardNumber)) return false;
    Progress->EarnedRewardNumbers.Add(RewardNumber);
    if (!UGameplayStatics::SaveGameToSlot(Progress, SaveSlotName, 0))
    {
        // 保存が失敗したらメモリーの追加も戻し、次の取得で再試行できるようにします。
        Progress->EarnedRewardNumbers.Remove(RewardNumber);
        ProgressProblem = TEXT("Progress could not be saved. Please try the reward again.");
        return false;
    }
    ProgressProblem.Reset();
    return true;
}

bool UThunderExpeditionSubsystem::IsLauncherUnlocked() const { return Progress && Progress->bLauncherUnlocked; }

bool UThunderExpeditionSubsystem::TryUnlockLauncher()
{
    if (!bCanWriteProgress || !Progress || Progress->bLauncherUnlocked) return false;
    Progress->bLauncherUnlocked = true;
    if (!UGameplayStatics::SaveGameToSlot(Progress, SaveSlotName, 0))
    {
        // ディスクへ残らなければメモリーも戻し、会話からもう一度試せるようにします。
        Progress->bLauncherUnlocked = false;
        ProgressProblem = TEXT("Launcher progress could not be saved. Please try again.");
        return false;
    }
    ProgressProblem.Reset();
    return true;
}

bool UThunderExpeditionSubsystem::IsFlightUnlocked() const { return Progress && Progress->bFlightUnlocked; }
bool UThunderExpeditionSubsystem::TryUnlockFlight()
{
    if (!bCanWriteProgress || !Progress || Progress->bFlightUnlocked) return false;
    Progress->bFlightUnlocked = true;
    if (!UGameplayStatics::SaveGameToSlot(Progress, SaveSlotName, 0))
    {
        Progress->bFlightUnlocked = false;
        ProgressProblem = TEXT("Leafwing progress could not be saved. Please try the switch again.");
        return false;
    }
    ProgressProblem.Reset();
    return true;
}
