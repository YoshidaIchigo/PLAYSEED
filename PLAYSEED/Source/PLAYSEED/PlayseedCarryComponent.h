#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayseedCarryComponent.generated.h"

class UInputComponent;
class APlayseedCarryable;

// Characterが所有する入力と保持の管理です。物の物理や描画はCarryable Actorへ任せます。
UCLASS(ClassGroup=(Playseed))
class PLAYSEED_API UPlayseedCarryComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPlayseedCarryComponent();
    void SetupInput(UInputComponent* Input);
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    void Interact();
    void DropHeld();
    APlayseedCarryable* GetHeldObject() const { return Held.Get(); }
private:
    TWeakObjectPtr<APlayseedCarryable> Held;
};
