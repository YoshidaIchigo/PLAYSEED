#pragma once

#include "CoreMinimal.h"
#include "PlayseedGameModeBase.h"
#include "GameFramework/HUD.h"
#include "ThunderMeadowGameMode.generated.h"

class AThunderMeadowStage;
class UThunderCoinLedger;
class UThunderLivesComponent;

UCLASS()
class PLAYSEED_API AThunderMeadowHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
private:
    int32 LastHealth=8;
    float PowerUntil=0;
};

// 雷鳴の丘のルールを担当します。地形の形状は専用 Actor へ分けます。
UCLASS()
class PLAYSEED_API AThunderMeadowGameMode : public APlayseedGameModeBase
{
    GENERATED_BODY()
public:
    AThunderMeadowGameMode();
    virtual void StartPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual bool CanLeaveStage() const override;
    float GetDropFade() const;
    virtual void RestartStage(AWindseedCharacter* Player) override;
    virtual void HandlePlayerDefeat(AWindseedCharacter* Player) override;
    UThunderLivesComponent* GetLives() const { return Lives; }
    virtual void HandlePlayerFall(AWindseedCharacter* Player) override;
    UThunderCoinLedger* GetCoinLedger() const { return CoinLedger; }
    bool ShouldShowControls() const { return bShowControls; }
private:
    void ToggleControls();
    bool bShowControls=false;
    bool bDropActive=false;
    float DropTime=0,DropLandedTime=-1;
    UPROPERTY() TObjectPtr<UThunderCoinLedger> CoinLedger;
    UPROPERTY() TObjectPtr<UThunderLivesComponent> Lives;
    // World が Stage を所有します。UPROPERTY は参照を UE に伝えます。
    UPROPERTY() TObjectPtr<AThunderMeadowStage> Stage;
};
