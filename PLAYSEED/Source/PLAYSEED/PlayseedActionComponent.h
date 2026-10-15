#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayseedActionComponent.generated.h"

class UInputComponent;
struct FHitResult;
DECLARE_MULTICAST_DELEGATE_OneParam(FPlayseedPoundImpact, FVector);

// 入力から始まる特殊アクションを担当する、Character所有の部品です。
// 地形やミッションを直接操作せず、着地した場所を通知して仕掛け側が反応できるようにします。
UCLASS(ClassGroup=(Playseed))
class PLAYSEED_API UPlayseedActionComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPlayseedActionComponent();
    void SetupInput(UInputComponent* Input);
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    void BeginGroundPound();
    void CancelGroundPound();
    void HandleLanded(const FHitResult& Hit);
    bool IsGroundPounding() const { return bGroundPounding; }
    int32 GetCompletedPounds() const { return CompletedPounds; }
    // C++用の通知です。後続の杭などはAddUObjectで自身の寿命を延ばさず購読できます。
    FPlayseedPoundImpact OnPoundImpact;
private:
    bool bGroundPounding = false;
    float PreviousGravity = 1.45f;
    float PreviousAirControl = .65f;
    int32 CompletedPounds = 0;
};
