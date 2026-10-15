#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderShortcutGate.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

// 一つの Actor が床スイッチ、柵の見た目、通行を止める衝突を所有します。
// 押した状態は入場中だけ保持し、永続のミッション報酬とは分けます。
UCLASS()
class PLAYSEED_API AThunderShortcutGate : public AActor
{
    GENERATED_BODY()
public:
    AThunderShortcutGate();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    void ResetGate();
    bool IsOpen() const { return bOpen; }
    int32 GetActivationCount() const { return ActivationCount; }
    FVector GetSwitchTop() const { return GetActorLocation() + FVector(-450, -300, 20); }
private:
    void OpenGate();
    // UPROPERTY で UE に参照を伝え、Actor と同じ寿命で構成部品を保持します。
    UPROPERTY() TObjectPtr<UBoxComponent> Barrier;
    UPROPERTY() TObjectPtr<USceneComponent> GateVisual;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> PressurePad;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Bars;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> PadMaterial;
    bool bOpen = false;
    int32 ActivationCount = 0;
};
