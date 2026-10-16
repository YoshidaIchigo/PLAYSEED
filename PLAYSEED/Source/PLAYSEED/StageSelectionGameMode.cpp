#include "StageSelectionGameMode.h"
#include "Components/InputComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void AStageSelectionHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;
    const float S = FMath::Min(Canvas->SizeX / 1280.f, Canvas->SizeY / 720.f);
    const float X = (Canvas->SizeX - 900 * S) * .5f;
    DrawRect(FLinearColor(.025f, .07f, .1f), 0, 0, Canvas->SizeX, Canvas->SizeY);
    auto Text = [&](const FString& Message, float Y, float Scale, FLinearColor Color)
    { DrawText(Message, Color, X, Y * S, GEngine->GetMediumFont(), Scale * S, false); };
    Text(TEXT("PLAYSEED"), 100, 4, FLinearColor(1, .8f, .3f));
    Text(TEXT("Choose a trail"), 185, 1.8f, FLinearColor::White);
    Text(TEXT("1   WINDSEED GARDEN"), 300, 2, FLinearColor(.7f, 1, .82f));
    Text(TEXT("Six seeds across the floating gardens"), 350, 1.3f, FLinearColor::White);
    Text(TEXT("2   THUNDER MEADOW"), 440, 2, FLinearColor(.65f, .83f, 1));
    Text(TEXT("Six expeditions and a hundred-lightcoin bonus"), 490, 1.3f, FLinearColor::White);
    Text(TEXT("Press 1 or 2 to enter. Press M during play to return here."), 620, 1.3f, FLinearColor(.85f, .88f, .9f));
}

AStageSelectionGameMode::AStageSelectionGameMode()
{
    DefaultPawnClass = nullptr;
    HUDClass = AStageSelectionHUD::StaticClass();
}

void AStageSelectionGameMode::StartPlay()
{
    Super::StartPlay();
    // Base が EnableInput で作った InputComponent に、この画面だけのキーを追加します。
    if (InputComponent)
    {
        InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AStageSelectionGameMode::EnterWindseedGarden);
        InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AStageSelectionGameMode::EnterThunderMeadow);
    }
    if (auto* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
    }
}

void AStageSelectionGameMode::EnterWindseedGarden()
{
    if (bTravelRequested) return;
    bTravelRequested = true;
    UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/Windseed/WindseedGarden")));
}

void AStageSelectionGameMode::EnterThunderMeadow()
{
    if (bTravelRequested) return;
    bTravelRequested = true;
    UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/ThunderMeadow/MissionSelect")));
}
