#pragma once
#include "CoreMinimal.h"
#include "PlayseedGameModeBase.h"
#include "GameFramework/HUD.h"
#include "ReferenceEntranceGameMode.generated.h"
class AReferenceEntranceRoom;
class AThunderFlightSwitch;
class UStaticMesh;
class USceneComponent;

UCLASS()
class PLAYSEED_API AReferenceEntranceHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};

UCLASS()
class PLAYSEED_API AReferenceEntranceGameMode : public APlayseedGameModeBase
{
    GENERATED_BODY()
public:
    AReferenceEntranceGameMode();
    virtual void StartPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void RestartStage(AWindseedCharacter* Player) override;
    virtual void HandlePlayerDefeat(AWindseedCharacter* Player) override;
    virtual bool CanLeaveStage() const override { return false; }
    float GetEntryFade() const;
    bool IsEntering() const { return bEntering; }
private:
    UPROPERTY() TObjectPtr<AReferenceEntranceRoom> Room;
    FVector DiveStart=FVector::ZeroVector;
    float EntryTime=0;
    bool bEntering=false;
};

// 部屋は小さな入口だけです。絵と額は自作の単純図形で構成し、画像の転載を使いません。
UCLASS()
class PLAYSEED_API AReferenceEntranceRoom : public AActor
{
    GENERATED_BODY()
public:
    AReferenceEntranceRoom();
    virtual void BeginPlay() override;
    void SetRipple(float Time);
    AThunderFlightSwitch* GetWingSwitch() const { return WingSwitch; }
private:
    void Shape(UStaticMesh* Asset,FVector Position,FVector Size,FLinearColor Color,bool bSolid=false,USceneComponent* Parent=nullptr);
    UPROPERTY() TObjectPtr<USceneComponent> Painting;
    UPROPERTY() TObjectPtr<AThunderFlightSwitch> WingSwitch;
};
