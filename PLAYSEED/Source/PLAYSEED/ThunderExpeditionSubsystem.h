#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderExpeditionSubsystem.generated.h"

// 選択結果をレベル移動の間だけ保持します。地形や敵の Actor はここで所有しません。
// 獲得済み報酬のディスク保存は次の変更で追加し、選択と達成を混同しないようにします。
UCLASS()
class PLAYSEED_API UThunderExpeditionSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    static constexpr int32 MainMissionCount = 6;
    bool SelectMission(int32 MissionNumber);
    int32 GetSelectedMissionNumber() const { return SelectedMissionNumber; }
    static const TCHAR* GetMissionTitle(int32 MissionNumber);
    static const TCHAR* GetMissionGoal(int32 MissionNumber);
private:
    // 画面の番号と同じ 1 始まりです。UPROPERTY により UE がこの値を認識します。
    UPROPERTY() int32 SelectedMissionNumber = 1;
};
