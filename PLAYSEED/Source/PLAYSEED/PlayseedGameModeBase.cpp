#include "PlayseedGameModeBase.h"
#include "WindseedCharacter.h"
#include "PlayseedSelectionTestSubsystem.h"
#include "ReferenceTravelSubsystem.h"
#include "Components/InputComponent.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

APlayseedGameModeBase::APlayseedGameModeBase()
{
    PrimaryActorTick.bCanEverTick = true;
}

void APlayseedGameModeBase::StartPlay()
{
    Super::StartPlay();
    // GameMode は通常は入力を受けません。ローカル1人用の共通メニューキーだけを登録します。
    if (auto* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        EnableInput(PC);
        if (InputComponent)
            InputComponent->BindKey(EKeys::M, IE_Pressed, this, &APlayseedGameModeBase::ReturnToStageSelection);
    }
#if !UE_BUILD_SHIPPING
    bSelectionTest = FParse::Param(FCommandLine::Get(), TEXT("StageSelectionSmokeTest"));
    if (bSelectionTest) GetGameInstance()->GetSubsystem<UPlayseedSelectionTestSubsystem>()->BeginStage();
#endif
}

void APlayseedGameModeBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
#if !UE_BUILD_SHIPPING
    if (bSelectionTest) GetGameInstance()->GetSubsystem<UPlayseedSelectionTestSubsystem>()->Update(DeltaSeconds, this);
#endif
}

bool APlayseedGameModeBase::HasPassedSelectionTest() const
{
    return !bSelectionTest || GetGameInstance()->GetSubsystem<UPlayseedSelectionTestSubsystem>()->HasPassed();
}

void APlayseedGameModeBase::ReturnToStageSelection()
{
    if(!CanLeaveStage()||UGameplayStatics::GetCurrentLevelName(this,true)==TEXT("MissionSelect"))return;
    auto* Flow=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>();
    // 絵から入ったプレイは入口へ戻し、次回も同じ入場手順を使える状態にします。
    if(Flow->UsesEntrance())
    {if(Flow->BeginReturn())UGameplayStatics::OpenLevel(this,FName(TEXT("/Game/Battlefield/Entrance")));}
    else UGameplayStatics::OpenLevel(this,FName(TEXT("/Game/ThunderMeadow/MissionSelect")));
}

void APlayseedGameModeBase::RestartStage(AWindseedCharacter* Player)
{
    // やり直し時の速度停止と開始位置への移動は Character の一か所に集約します。
    if (Player) Player->ReturnToTrail();
}

void APlayseedGameModeBase::HandlePlayerDefeat(AWindseedCharacter* Player)
{
    RestartStage(Player);
}

void APlayseedGameModeBase::HandlePlayerFall(AWindseedCharacter* Player)
{
    // 既存Gardenの落下は収集物をリセットせず復帰する従来の動作を保ちます。
    if (Player) Player->ReturnToTrail();
}
