#include "PlayseedGameModeBase.h"
#include "WindseedCharacter.h"

void APlayseedGameModeBase::RestartStage(AWindseedCharacter* Player)
{
    // やり直し時の速度停止と開始位置への移動は Character の一か所に集約します。
    if (Player) Player->ReturnToTrail();
}
