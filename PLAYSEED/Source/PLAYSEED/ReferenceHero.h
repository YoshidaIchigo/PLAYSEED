#pragma once

#include "CoreMinimal.h"
#include "WindseedCharacter.h"
#include "ReferenceHero.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class USceneComponent;
class UReferenceCameraVisibilityComponent;

// 入力・衝突・体力は既存Characterを再利用し、参照に合わせた人型の表示だけを担当します。
UCLASS()
class PLAYSEED_API AReferenceHero : public AWindseedCharacter
{
    GENERATED_BODY()
public:
    AReferenceHero();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    // 通常のスター取得フローから使う見た目のポーズです。進行値は変更しません。
    void SetStarPose(bool bEnabled) { bStarPose=bEnabled; }
    bool IsShowingStarPose() const { return bStarPose; }
    void SetLandingPose(bool bEnabled) { bLandingPose=bEnabled; }
    void UpdateCameraVisibility();
    bool IsBodyHiddenForCamera() const { return bBodyHiddenForCamera; }
    int32 GetCameraHideCount() const { return CameraHideCount; }
    float GetActualCameraDistance() const { return ActualCameraDistance; }
private:
    UStaticMeshComponent* Part(USceneComponent* Parent,UStaticMesh* ShapeAsset,FVector Position,FVector Dimensions,FLinearColor Color);
    UPROPERTY() TObjectPtr<USceneComponent> Visual;
    UPROPERTY() TObjectPtr<USceneComponent> Head;
    UPROPERTY() TObjectPtr<USceneComponent> LeftArm;
    UPROPERTY() TObjectPtr<USceneComponent> RightArm;
    UPROPERTY() TObjectPtr<USceneComponent> LeftLeg;
    UPROPERTY() TObjectPtr<USceneComponent> RightLeg;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Shadow;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Parts;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> CapFeathers;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> VictoryFingers;
    TArray<FLinearColor> Colors;
    bool bStarPose=false;
    bool bLandingPose=false;
    UPROPERTY() TObjectPtr<UReferenceCameraVisibilityComponent> CameraVisibility;
    bool bBodyHiddenForCamera=false;
    int32 CameraHideCount=0;
    float ActualCameraDistance=900;
};
