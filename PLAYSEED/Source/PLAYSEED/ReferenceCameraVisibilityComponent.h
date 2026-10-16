#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReferenceCameraVisibilityComponent.generated.h"

// カメラの壁判定が終わった後に、視点を塞ぐ自分の見た目だけを更新します。
UCLASS()
class PLAYSEED_API UReferenceCameraVisibilityComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UReferenceCameraVisibilityComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) override;
};
