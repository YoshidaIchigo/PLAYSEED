#pragma once

#include "CoreMinimal.h"
#include "PlayseedGameModeBase.h"
#include "GameFramework/HUD.h"
#include "ThunderMeadowGameMode.generated.h"

UCLASS()
class PLAYSEED_API AThunderMeadowHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};

// 雷鳴の丘のルール。入口確認用の床だけを作り、地形やミッションは次の小さな変更で追加します。
UCLASS()
class PLAYSEED_API AThunderMeadowGameMode : public APlayseedGameModeBase
{
    GENERATED_BODY()
public:
    AThunderMeadowGameMode();
    virtual void StartPlay() override;
private:
    void CreateEntryFloor();
};
