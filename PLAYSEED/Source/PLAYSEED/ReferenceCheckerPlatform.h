#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReferenceCheckerPlatform.generated.h"
class UStaticMeshComponent;

// 二枚の足場を元資料の30Hzの経路で動かします。乗った人はCharacterMovementが運びます。
UCLASS()
class PLAYSEED_API AReferenceCheckerPlatform : public AActor
{
    GENERATED_BODY()
public:
    AReferenceCheckerPlatform();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    void Configure(int32 Index);
    void ResetMotion();
    UStaticMeshComponent* GetSurface() const { return Surface; }
    bool IsRising() const { return Action==1; }
    FVector GetSurfaceTop() const { return GetActorLocation()+FVector(0,0,39); }
private:
    void StepMotion();
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Surface;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Tiles;
    FVector Origin=FVector::ZeroVector,Offset=FVector::ZeroVector;
    double Accumulator=0;
    int32 PlatformIndex=0,Action=1,Timer=0,MoveAngle=0,FaceAngle=0;
    bool bConfigured=false;
};
