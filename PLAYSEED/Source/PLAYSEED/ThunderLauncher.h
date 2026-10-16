#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayseedInteractionTarget.h"
#include "ThunderLauncher.generated.h"

class UStaticMeshComponent;

// 解放された見た目を保存済みの進行から作ります。Actor自身は保存データを所有しません。
UCLASS()
class PLAYSEED_API AThunderLauncher : public AActor, public IPlayseedInteractionTarget
{
    GENERATED_BODY()
public:
    AThunderLauncher();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void Interact(AWindseedCharacter* Player) override;
    bool IsOpen() const { return bOpen; }
    FVector GetSeatLocation() const { return GetActorLocation() + FVector(0, 0, 140); }
    float GetDefaultAimYaw() const { return DefaultAimYaw; }
    void ConfigureAimYaw(float Yaw);
    void SetAimPose(float Pitch, float WorldYaw);
private:
    void RefreshOpenState();
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Barrel;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Parts;
    TArray<FLinearColor> Colors;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> ClosedLid;
    float DefaultAimYaw=10.304846f;
    bool bReferenceLauncher=false;
    bool bOpen = false;
};
