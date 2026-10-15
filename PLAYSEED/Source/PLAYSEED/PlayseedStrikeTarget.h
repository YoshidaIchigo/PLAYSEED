#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayseedStrikeTarget.generated.h"

class AWindseedCharacter;

// 攻撃を受けられるActorの共通窓口です。敵・後続の箱がそれぞれ反応を実装できます。
UINTERFACE(MinimalAPI)
class UPlayseedStrikeTarget : public UInterface
{
    GENERATED_BODY()
};

class PLAYSEED_API IPlayseedStrikeTarget
{
    GENERATED_BODY()
public:
    virtual bool ReceiveStrike(AWindseedCharacter* Player, int32 Damage) = 0;
};
