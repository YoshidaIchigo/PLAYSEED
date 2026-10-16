#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ThunderLivesComponent.generated.h"

// 雷鳴の丘のGameModeが所有する入場中の残機です。報酬のSaveGameとは分けます。
UCLASS(ClassGroup=(Playseed))
class PLAYSEED_API UThunderLivesComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UThunderLivesComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    bool TryCollectLife();
    bool ConsumeLife();
    int32 GetRemaining() const { return Remaining; }
    int32 GetConsumedLives() const { return ConsumedLives; }
    bool IsGameOver() const { return Remaining == 0; }
    float GetGameOverRemaining() const { return GameOverRemaining; }
private:
    UPROPERTY(VisibleInstanceOnly,Category="Lives") int32 Remaining = 3;
    int32 ConsumedLives = 0;
    float GameOverRemaining = 0;
    bool bReturningToMenu = false;
};
