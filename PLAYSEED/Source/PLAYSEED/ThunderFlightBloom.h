#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderFlightBloom.generated.h"

class UStaticMeshComponent;

// World所有の再取得できる羽の花です。能力の残り時間はCharacter側が所有します。
UCLASS()
class PLAYSEED_API AThunderFlightBloom : public AActor
{
    GENERATED_BODY()
public:
    AThunderFlightBloom();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    void ResetBloom();
    bool IsAvailable() const { return bUnlocked && !bCollected; }
    int32 GetCollections() const { return Collections; }
private:
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Bud;
    bool bCollected = false;
    bool bUnlocked = false;
    float RespawnRemaining = 0;
    int32 Collections = 0;
};
