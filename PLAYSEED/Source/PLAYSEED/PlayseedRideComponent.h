#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayseedRideComponent.generated.h"

class APlayseedGlideBoard;
class UInputComponent;

// Characterが所有する乗降担当です。プレイヤーは葉へAttachせず、普段のカプセル衝突で移動します。
UCLASS()
class PLAYSEED_API UPlayseedRideComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPlayseedRideComponent();
    void SetupInput(UInputComponent* Input);
    void ToggleRide();
    void CancelRide();
    bool IsRiding() const { return bRiding; }
    APlayseedGlideBoard* GetBoard() const { return Board.Get(); }
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
    // 葉はWorld所有なので弱参照です。Rで先に破棄されても、bRidingを基準に移動設定を戻します。
    TWeakObjectPtr<APlayseedGlideBoard> Board;
    bool bRiding = false;
    float PreviousSpeed = 470;
    float PreviousFriction = 8;
    float PreviousBraking = 2048;
    float PreviousAcceleration = 2048;
    float PreviousAirControl = .65f;
};
