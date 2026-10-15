#pragma once

#include "CoreMinimal.h"
#include "PlayseedGameModeBase.h"
#include "GameFramework/HUD.h"
#include "ThunderMeadowGameMode.generated.h"

class AThunderMeadowStage;

UCLASS()
class PLAYSEED_API AThunderMeadowHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};

// 雷鳴の丘のルールを担当します。地形の形状は専用 Actor へ分けます。
UCLASS()
class PLAYSEED_API AThunderMeadowGameMode : public APlayseedGameModeBase
{
    GENERATED_BODY()
public:
    AThunderMeadowGameMode();
    virtual void StartPlay() override;
private:
    // World が Stage を所有します。UPROPERTY は参照を UE に伝えます。
    UPROPERTY() TObjectPtr<AThunderMeadowStage> Stage;
};
