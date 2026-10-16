#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayseedStrikeTarget.h"
#include "ReferenceWingBox.generated.h"
class UStaticMeshComponent;
class UPrimitiveComponent;
class UPlayseedActionComponent;
class AReferenceWingCap;
class AWindseedCharacter;

// 解放前は中を通れる枠、解放後は実際に壊せる赤箱です。約10秒で箱を戻します。
UCLASS()
class PLAYSEED_API AReferenceWingBox : public AActor,public IPlayseedStrikeTarget
{
    GENERATED_BODY()
public:
    AReferenceWingBox();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual bool ReceiveStrike(AWindseedCharacter* Hero,int32 Damage) override;
    void ResetBox();
    bool IsBroken() const { return bBroken; }
    bool IsUnlocked() const { return bUnlocked; }
    AReferenceWingCap* GetCap() const { return Cap.Get(); }
    int32 GetSpawnCount() const { return SpawnCount; }
    int32 GetHeadBumpCount() const { return HeadBumps; }
private:
    void Refresh();
    void HandlePound(FVector Impact);
    UFUNCTION() void HandleHeadBump(UPrimitiveComponent* Component,AActor* OtherActor,UPrimitiveComponent* OtherComponent,FVector Impulse,const FHitResult& Hit);
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Body;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Markers;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Outline;
    TWeakObjectPtr<UPlayseedActionComponent> Actions;
    TWeakObjectPtr<AReferenceWingCap> Cap;
    bool bUnlocked=false,bBroken=false;
    float RespawnRemaining=0;
    int32 SpawnCount=0,HeadBumps=0;
};
