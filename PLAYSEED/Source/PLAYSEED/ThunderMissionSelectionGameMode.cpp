#include "ThunderMissionSelectionGameMode.h"
#include "ThunderExpeditionSubsystem.h"
#include "Components/InputComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

void AThunderMissionSelectionHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;
    const float S = FMath::Min(Canvas->SizeX / 1280.f, Canvas->SizeY / 720.f);
    const float X = (Canvas->SizeX - 1020 * S) * .5f;
    DrawRect(FLinearColor(.025f, .07f, .1f), 0, 0, Canvas->SizeX, Canvas->SizeY);
    auto Text = [&](const FString& Message, float Y, float Scale, FLinearColor Color)
    { DrawText(Message, Color, X, Y * S, GEngine->GetMediumFont(), Scale * S, false); };
    Text(TEXT("THUNDER MEADOW"), 55, 2.7f, FLinearColor(.7f, .9f, 1));
    Text(TEXT("Choose your expedition"), 125, 1.6f, FLinearColor::White);
    auto* Expedition = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    Text(FString::Printf(TEXT("LANTERNS  %d / 7"), Expedition->GetEarnedRewardCount()), 165, 1.1f, FLinearColor(.7f, 1, .8f));
    for (int32 Number = 1; Number <= UThunderExpeditionSubsystem::MainMissionCount; ++Number)
    {
        const float Y = 205 + (Number - 1) * 58;
        const bool bEarned = Expedition->HasEarnedReward(Number);
        Text(FString::Printf(TEXT("%d   %s%s"), Number, UThunderExpeditionSubsystem::GetMissionTitle(Number), bEarned ? TEXT("  [earned]") : TEXT("")), Y, 1.5f,
            bEarned ? FLinearColor(.7f, 1, .8f) : FLinearColor(1, .86f, .52f));
        Text(UThunderExpeditionSubsystem::GetMissionGoal(Number), Y + 29, .95f, FLinearColor::White);
    }
    Text(Expedition->HasEarnedReward(7) ? TEXT("Bonus lantern: 100 lightcoins  [earned]") : TEXT("Bonus lantern: gather 100 lightcoins during one visit."), 590, 1.2f, FLinearColor(.7f, 1, .8f));
    Text(TEXT("Press 1-6 to enter. M: stage selection. Seven lanterns to discover."), 645, 1.15f, FLinearColor::White);
    if (!Expedition->GetProgressProblem().IsEmpty()) Text(Expedition->GetProgressProblem(), 683, .9f, FLinearColor(1, .5f, .4f));
}

AThunderMissionSelectionGameMode::AThunderMissionSelectionGameMode()
{
    DefaultPawnClass = nullptr;
    HUDClass = AThunderMissionSelectionHUD::StaticClass();
}

void AThunderMissionSelectionGameMode::StartPlay()
{
    Super::StartPlay();
    if (!InputComponent) return;
    InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AThunderMissionSelectionGameMode::ChooseOne);
    InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AThunderMissionSelectionGameMode::ChooseTwo);
    InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AThunderMissionSelectionGameMode::ChooseThree);
    InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &AThunderMissionSelectionGameMode::ChooseFour);
    InputComponent->BindKey(EKeys::Five, IE_Pressed, this, &AThunderMissionSelectionGameMode::ChooseFive);
    InputComponent->BindKey(EKeys::Six, IE_Pressed, this, &AThunderMissionSelectionGameMode::ChooseSix);
}

void AThunderMissionSelectionGameMode::EnterMission(int32 Number)
{
    if (bTravelRequested) return;
    // GameInstance が所有する Subsystem は、OpenLevel 後も同じ選択番号を保持します。
    auto* Expedition = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if (!Expedition->SelectMission(Number)) return;
    bTravelRequested = true;
    UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/ThunderMeadow/ThunderMeadow")));
}
