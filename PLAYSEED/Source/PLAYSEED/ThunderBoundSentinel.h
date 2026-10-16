#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderBoundSentinel.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class AWindseedCharacter;

// 茂み色の四足の番獣。予告時の位置へ突進し、結び目から300cmの範囲を守ります。
UCLASS()
class PLAYSEED_API AThunderBoundSentinel : public AActor
{
    GENERATED_BODY()
public:
    AThunderBoundSentinel();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    void Configure(FVector GroundAnchor);
    void ResetSentinel();
    FVector GetAnchor() const { return Anchor; }
    FVector GetHomeLocation() const { return Anchor + FVector(150, 0, 100); }
    bool IsWindingUp() const { return State == EState::Windup; }
    bool IsLunging() const { return State == EState::Lunge; }
    bool IsPatrolling() const { return State == EState::Patrol; }
    int32 GetLungeCount() const { return LungeCount; }
    int32 GetConfirmedHits() const { return ConfirmedHits; }
    float GetMaxObservedReach() const { return MaxObservedReach; }
    FVector GetLockedTarget() const { return LockedTarget; }
private:
    enum class EState : uint8 { Patrol, Windup, Lunge, Recover, Return };
    void EnterState(EState Next);
    void MoveBounded(FVector Desired, AWindseedCharacter* Player);
    void UpdateTether();
    void CheckContact(FVector Start, FVector End, AWindseedCharacter* Player);
    UPROPERTY() TObjectPtr<USphereComponent> Body;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Parts;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Links;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Warning;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> AnchorPlate;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> BodyTint;
    FVector Anchor = FVector::ZeroVector;
    FVector LockedTarget = FVector::ZeroVector;
    FVector LungeDirection = FVector::ForwardVector;
    EState State = EState::Patrol;
    float StateTime = 0;
    float PatrolTime = 0;
    float MaxObservedReach = 0;
    int32 LungeCount = 0;
    int32 ConfirmedHits = 0;
    int32 MotionRevision = 0;
};
