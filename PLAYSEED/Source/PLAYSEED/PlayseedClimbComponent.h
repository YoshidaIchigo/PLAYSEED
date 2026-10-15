#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayseedClimbComponent.generated.h"

class APlayseedClimbTree;
class UInputComponent;

// 木の周囲に取り付き、CharacterMovementの速度で上下する部品です。プレイヤーをAttachしません。
UCLASS(ClassGroup=(Playseed))
class PLAYSEED_API UPlayseedClimbComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPlayseedClimbComponent();
    void SetupInput(UInputComponent* Input);
    void ToggleClimb();
    void SetClimbInput(float Value) { ClimbInput = FMath::Clamp(Value, -1.f, 1.f); }
    void CancelClimb();
    bool JumpAway();
    bool IsClimbing() const { return bClimbing; }
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
    TWeakObjectPtr<APlayseedClimbTree> Tree;
    FVector Outward = FVector::ForwardVector;
    float ClimbInput = 0;
    bool bClimbing = false;
};
