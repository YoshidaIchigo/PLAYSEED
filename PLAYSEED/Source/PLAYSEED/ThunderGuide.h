#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayseedInteractionTarget.h"
#include "ThunderGuide.generated.h"

class UStaticMeshComponent;

// World所有のオリジナル案内役「エアリ」。会話状態は一時的、解放結果はSaveGameです。
UCLASS()
class PLAYSEED_API AThunderGuide : public AActor, public IPlayseedInteractionTarget
{
    GENERATED_BODY()
public:
    AThunderGuide();
    virtual void BeginPlay() override;
    virtual void Interact(AWindseedCharacter* Player) override;
    void ConfigureOpeningGuide(bool bOpens) { bCanOpenCannon=bOpens; }
    void ResetDialogue() { bAwaitingConfirmation = false; }
    bool IsAwaitingConfirmation() const { return bAwaitingConfirmation; }
private:
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Parts;
    TArray<FLinearColor> Colors;
    bool bReferenceGuide=false,bCanOpenCannon=true;
    bool bAwaitingConfirmation = false;
};
