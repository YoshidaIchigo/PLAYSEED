#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayseedInteractionTarget.generated.h"

class AWindseedCharacter;

// 会話・発射台などの共通窓口です。Actorごとに入力を奪わず、一番近い対象だけを呼びます。
UINTERFACE(MinimalAPI)
class UPlayseedInteractionTarget : public UInterface
{
    GENERATED_BODY()
};

class PLAYSEED_API IPlayseedInteractionTarget
{
    GENERATED_BODY()
public:
    virtual void Interact(AWindseedCharacter* Player) = 0;
};
