#include "PlayseedSelectionTestSubsystem.h"
#include "PlayseedGameModeBase.h"
#include "PlayseedRecordingSubsystem.h"
#include "WindseedCharacter.h"
#include "WindseedStage.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
    void Key(APlayerController* PC, FKey Key, bool bPressed)
    {
        if (PC) PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, bPressed ? IE_Pressed : IE_Released, bPressed ? 1 : 0));
    }
    void Tap(APlayerController* PC, FKey PressedKey)
    {
        Key(PC, PressedKey, true);
        Key(PC, PressedKey, false);
    }
}

void UPlayseedSelectionTestSubsystem::BeginStage()
{
    Elapsed = 0;
    auto* Recording = GetGameInstance()->GetSubsystem<UPlayseedRecordingSubsystem>();
    if (!Recording->StartRecording(TEXT("Validation/WindseedPlaythrough.avi")))
        Fail(TEXT("Stage selection recording could not start"));
}

void UPlayseedSelectionTestSubsystem::Update(float DeltaSeconds, APlayseedGameModeBase* CurrentMode)
{
    if (Step < 0 || HasPassed()) return;
    Elapsed += DeltaSeconds;
    if (Elapsed > 20) { Fail(TEXT("Stage selection timed out")); return; }
    auto* PC = UGameplayStatics::GetPlayerController(CurrentMode, 0);
    if (!PC) return;
    const FString Map = UGameplayStatics::GetCurrentLevelName(CurrentMode, true);
    auto* Player = Cast<AWindseedCharacter>(PC->GetPawn());
    if (Step == 0 && Map == TEXT("StageSelect") && Elapsed > 3)
    {
        Tap(PC, EKeys::Two);
        Step = 1;
        UE_LOG(LogTemp, Display, TEXT("SELECTION TEST: pressed 2 to enter Thunder Meadow."));
    }
    else if (Step == 1 && Map == TEXT("ThunderMeadow") && Player && Elapsed > 2)
    {
        TArray<AActor*> Gardens;
        UGameplayStatics::GetAllActorsOfClass(CurrentMode, AWindseedStage::StaticClass(), Gardens);
        if (Gardens.Num() || FVector::Dist2D(Player->GetActorLocation(), FVector(-600, -600, 100)) > 5)
        { Fail(TEXT("The new stage has mixed actors or a wrong start")); return; }
        InputStart = Player->GetActorLocation();
        PC->SetControlRotation(FRotator(-18, 0, 0));
        Key(PC, EKeys::W, true);
        Key(PC, EKeys::SpaceBar, true);
        Step = 2;
        Elapsed = 0;
    }
    else if (Step == 2 && Elapsed > .4f && Player)
    {
        Key(PC, EKeys::W, false);
        Key(PC, EKeys::SpaceBar, false);
        if (Player->GetActorLocation().X < InputStart.X + 70 || Player->GetActorLocation().Z < InputStart.Z + 40)
        { Fail(TEXT("Movement and jump in the new stage failed")); return; }
        Tap(PC, EKeys::R);
        Step = 3;
        Elapsed = 0;
    }
    else if (Step == 3 && Elapsed > 1 && Player)
    {
        if (FVector::Dist2D(Player->GetActorLocation(), FVector(-600, -600, 100)) > 5 || Player->RecoveryCount != 1)
        { Fail(TEXT("Restart in the new stage failed")); return; }
        Tap(PC, EKeys::M);
        Step = 4;
        UE_LOG(LogTemp, Display, TEXT("SELECTION TEST: Thunder movement, jump and R passed; pressed M."));
    }
    else if (Step == 4 && Map == TEXT("StageSelect") && Elapsed > 3)
    {
        Tap(PC, EKeys::One);
        Step = 5;
    }
    else if (Step == 5 && Map == TEXT("WindseedGarden") && Player && Elapsed > 1)
    {
        Step = 6;
        UE_LOG(LogTemp, Display, TEXT("SELECTION TEST PASS: menu -> Thunder -> menu -> Garden through 2/M/1 input."));
    }
}

void UPlayseedSelectionTestSubsystem::Fail(const FString& Message)
{
    Step = -1;
    auto* Recording = GetGameInstance()->GetSubsystem<UPlayseedRecordingSubsystem>();
    const int32 Frames = Recording->FinishRecording();
    const FString Report = FString::Printf(TEXT("FAIL\n%s\nCaptureFrames: %d\n"), *Message, Frames);
    FFileHelper::SaveStringToFile(Report, *(FPaths::ProjectSavedDir() / TEXT("Validation/Result.txt")));
    UE_LOG(LogTemp, Error, TEXT("SELECTION TEST FAIL: %s"), *Message);
    FPlatformMisc::RequestExitWithStatus(false, 1);
}
