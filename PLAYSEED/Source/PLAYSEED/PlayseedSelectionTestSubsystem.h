#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayseedSelectionTestSubsystem.generated.h"

class APlayseedGameModeBase;

// ステージ往復の自動検証だけを担当します。ゲーム中の進行・保存とは別の状態です。
// Subsystem なので OpenLevel で World が入れ替わっても検証の手順番号が残ります。
UCLASS()
class PLAYSEED_API UPlayseedSelectionTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginStage();
    void Update(float DeltaSeconds, APlayseedGameModeBase* CurrentMode);
    bool HasPassed() const { return Step == 6; }
private:
    void Fail(const FString& Message);
    int32 Step = 0;
    float Elapsed = 0;
    FVector InputStart = FVector::ZeroVector;
    bool bSecondMissionEntry = false;
};
