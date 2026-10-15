#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayseedInteractionComponent.generated.h"

class UInputComponent;

// Character所有の会話入力と短い案内表示です。永続の進行はExpeditionへ任せます。
UCLASS()
class PLAYSEED_API UPlayseedInteractionComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPlayseedInteractionComponent();
    void SetupInput(UInputComponent* Input);
    void InteractNearest();
    void ShowMessage(const FString& Text);
    void ClearMessage();
    const FString& GetMessage() const { return Message; }
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
    FString Message;
    float MessageRemaining = 0;
    float Cooldown = 0;
};
