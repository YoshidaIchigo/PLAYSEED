#include "ThunderLivesComponent.h"
#include "Kismet/GameplayStatics.h"
UThunderLivesComponent::UThunderLivesComponent() { PrimaryComponentTick.bCanEverTick=true; }
bool UThunderLivesComponent::TryCollectLife()
{
    if (IsGameOver() || Remaining>=9) return false;
    ++Remaining; UE_LOG(LogTemp,Display,TEXT("LIFESEED: actual pickup, lives%d."),Remaining); return true;
}
bool UThunderLivesComponent::ConsumeLife()
{
    if (IsGameOver()) return false;
    --Remaining; ++ConsumedLives;
    UE_LOG(LogTemp,Display,TEXT("THUNDER LIVES: actual defeat consumed one, lives%d, deaths%d."),Remaining,ConsumedLives);
    if (IsGameOver()) GameOverRemaining=3;
    return !IsGameOver();
}
void UThunderLivesComponent::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime,TickType,ThisTickFunction);
    if (!IsGameOver() || bReturningToMenu) return;
    GameOverRemaining=FMath::Max(0.f,GameOverRemaining-DeltaTime);
    if (GameOverRemaining<=0)
    {
        bReturningToMenu=true;
        // OpenLevelで古いWorldと入場中の残機を終了します。永続の報酬は変更しません。
        UGameplayStatics::OpenLevel(GetOwner(),FName(TEXT("/Game/ThunderMeadow/MissionSelect")));
    }
}
