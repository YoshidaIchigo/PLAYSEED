#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderExpeditionSubsystem.generated.h"

class UThunderProgressSaveGame;

// 選択と獲得済み報酬を管理します。地形や敵の Actor はここで所有しません。
// 選択はメモリーだけ、獲得済み報酬は SaveGame へ残し、入場中の状態と区別します。
UCLASS()
class PLAYSEED_API UThunderExpeditionSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    static constexpr int32 MainMissionCount = 6;
    static constexpr int32 TotalRewardCount = 7;
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    bool SelectMission(int32 MissionNumber);
    int32 GetSelectedMissionNumber() const { return SelectedMissionNumber; }
    static const TCHAR* GetMissionTitle(int32 MissionNumber);
    static const TCHAR* GetMissionGoal(int32 MissionNumber);
    bool TryAwardReward(int32 RewardNumber);
    bool HasEarnedReward(int32 RewardNumber) const;
    int32 GetEarnedRewardCount() const;
    const FString& GetProgressProblem() const { return ProgressProblem; }
    const FString& GetSaveSlotName() const { return SaveSlotName; }
private:
    void LoadProgress();
    bool IsValidProgress(const UThunderProgressSaveGame* Data) const;
    bool ChooseSaveSlot();
    // 画面の番号と同じ 1 始まりです。UPROPERTY により UE がこの値を認識します。
    UPROPERTY() int32 SelectedMissionNumber = 1;
    // Subsystem が参照を持ち、GC が使用中の SaveGame を回収しないようにします。
    UPROPERTY() TObjectPtr<UThunderProgressSaveGame> Progress;
    FString SaveSlotName = TEXT("Playseed_ThunderProgress");
    FString ProgressProblem;
    bool bCanWriteProgress = true;
};
