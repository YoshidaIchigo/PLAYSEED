#include "StageSelectionGameMode.h"
#include "Kismet/GameplayStatics.h"
AStageSelectionGameMode::AStageSelectionGameMode() { DefaultPawnClass=nullptr; HUDClass=nullptr; }
void AStageSelectionGameMode::StartPlay()
{
    Super::StartPlay();
    UGameplayStatics::OpenLevel(this,FName(TEXT("/Game/ThunderMeadow/MissionSelect")));
}
