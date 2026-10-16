#include "ReferenceCameraVisibilityComponent.h"
#include "ReferenceHero.h"
#include "Engine/World.h"
#include "GameFramework/SpringArmComponent.h"

UReferenceCameraVisibilityComponent::UReferenceCameraVisibilityComponent()
{
    PrimaryComponentTick.bCanEverTick=true;PrimaryComponentTick.TickGroup=TG_PostUpdateWork;
}
void UReferenceCameraVisibilityComponent::BeginPlay()
{
    Super::BeginPlay();
    if(auto* Hero=Cast<AReferenceHero>(GetOwner()))AddTickPrerequisiteComponent(Hero->GetCameraBoom());
}
void UReferenceCameraVisibilityComponent::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime,TickType,ThisTickFunction);
    if(GetWorld()&&GetWorld()->IsGameWorld())if(auto* Hero=Cast<AReferenceHero>(GetOwner()))Hero->UpdateCameraVisibility();
}
