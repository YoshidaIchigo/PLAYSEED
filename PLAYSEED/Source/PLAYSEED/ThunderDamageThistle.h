#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderDamageThistle.generated.h"

class UStaticMeshComponent;

// 近づいて接触すると被弾する、オリジナルの光るイバラです。体力の管理はCharacter側に任せます。
UCLASS()
class PLAYSEED_API AThunderDamageThistle : public AActor
{
    GENERATED_BODY()
public:
    AThunderDamageThistle();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
private:
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Thorn;
};
