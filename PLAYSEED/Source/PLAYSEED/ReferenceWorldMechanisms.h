#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReferenceWorldMechanisms.generated.h"
class UProceduralMeshComponent;
class UStaticMeshComponent;
class USceneComponent;
class UMaterialInstanceDynamic;
class AThunderWarpPair;

// このActorが柵・スイッチ・ハート・2組のワープを入場中だけ所有します。
UCLASS()
class PLAYSEED_API AReferenceWorldMechanisms : public AActor
{
    GENERATED_BODY()
public:
    AReferenceWorldMechanisms();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    void ResetMechanisms();
    bool IsGateOpen() const { return bGateOpen; }
    float GetGateOpening() const { return Opening; }
    int32 GetGateActivations() const { return GateActivations; }
    FVector GetSwitchTop() const;
    FVector GetHeartLocation() const;
    int32 GetHeartHeals() const { return HeartHeals; }
    int32 GetFloorFailures() const { return FloorFailures; }
    const TArray<TObjectPtr<AThunderWarpPair>>& GetWarpPairs() const { return WarpPairs; }
private:
    UPROPERTY() TArray<TObjectPtr<USceneComponent>> Hinges;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Bars;
    UPROPERTY() TObjectPtr<UProceduralMeshComponent> PressurePad;
    UPROPERTY() TObjectPtr<UProceduralMeshComponent> Heart;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> PadTint;
    UPROPERTY() TArray<TObjectPtr<AThunderWarpPair>> WarpPairs;
    float PressedSeconds=0,Opening=0,HeartAngle=0,HeartSpinUnits=400,HeartTotalSpin=0;
    bool bGateOpen=false;
    int32 GateActivations=0,HeartHeals=0,FloorFailures=0;
};
