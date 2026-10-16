#include "ThunderGuideTestSubsystem.h"
#include "ThunderCampaignValidation.h"
#include "ThunderGuide.h"
#include "ThunderLauncher.h"
#include "ThunderMeadowStage.h"
#include "ThunderExpeditionSubsystem.h"
#include "PlayseedInteractionComponent.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Engine/GameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

namespace
{
void GuideTap(APlayerController* PC, FKey Key)
{
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Pressed, 1));
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Released, 0));
}
}
void UThunderGuideTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage, bool bRestoredProcess)
{
    Stage = InStage; bReadProcess = bRestoredProcess;
    Phase = Waypoint = 0; PhaseTime = 0; Error.Reset(); RecoveryBefore = Player->RecoveryCount;
    auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    ExpectedRewards = ThunderCampaignEnabled() ? Progress->GetEarnedRewardCount() : 0;
    if (!Progress->GetSaveSlotName().StartsWith(TEXT("Validation_")) || !Progress->GetProgressProblem().IsEmpty() || Progress->GetEarnedRewardCount() != ExpectedRewards || Progress->IsLauncherUnlocked() != bReadProcess || (!ThunderCampaignEnabled() && UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(), 0) != bReadProcess))
    { Error = TEXT("Guide test requires a healthy isolated fresh/read save with unchanged quest rewards"); return; }
    GuideTap(Cast<APlayerController>(Player->GetController()), EKeys::G);
}
bool UThunderGuideTestSubsystem::Walk(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds)
{
    const FVector Offset = Goal - Player->GetActorLocation();
    if (Offset.Size2D() < 20 && Player->GetCharacterMovement()->IsMovingOnGround()) { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
    Player->AddMovementInput(Offset.GetSafeNormal2D());
    auto* PC = Cast<APlayerController>(Player->GetController());
    PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, Offset.Rotation().Yaw, 0), DeltaSeconds, 3));
    return false;
}
bool UThunderGuideTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* PC = Cast<APlayerController>(Player->GetController());
    auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if (!Stage.IsValid() || !PC) { Error = TEXT("Guide stage/controller missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 50) { Error = FString::Printf(TEXT("Guide phase %d waypoint %d timed out at %s"), Phase, Waypoint, *Player->GetActorLocation().ToString()); return false; }
    if (Player->GetVitality()->GetHealth() != 8 || Progress->GetEarnedRewardCount() != ExpectedRewards || !Progress->GetProgressProblem().IsEmpty()) { Error = TEXT("Guide route changed health/rewards or could not save"); return false; }
    if (Phase == 0 && PhaseTime > .4f)
    {
        if (!Player->GetInteraction()->GetMessage().IsEmpty() || Progress->IsLauncherUnlocked() != bReadProcess) { Error = TEXT("G interacted with a distant actor"); return false; }
        SetPhase(1);
    }
    else if (Phase == 1)
    {
        const FVector GuideApproach[] = {FVector(-2800, -600, 82), FVector(-3000, 1500, 82), Stage->GetGuide()->GetActorLocation() + FVector(-160, 0, 82)};
        if (Waypoint < 3 && Walk(Player, GuideApproach[Waypoint], DeltaSeconds)) ++Waypoint;
        if (Waypoint >= 3) { GuideTap(PC, EKeys::G); SetPhase(2); }
    }
    else if (Phase == 2 && PhaseTime > .4f)
    {
        if (!Player->GetInteraction()->GetMessage().Contains(TEXT("AERI"))) { Error = TEXT("Actual nearby G did not start Aeri dialogue"); return false; }
        if (bReadProcess)
        {
            if (!Progress->IsLauncherUnlocked() || Stage->GetGuide()->IsAwaitingConfirmation() || !Stage->GetLauncher()->IsOpen()) { Error = TEXT("Restarted process did not restore an already open launcher"); return false; }
            SetPhase(5);
        }
        else
        {
            if (Progress->IsLauncherUnlocked() || !Stage->GetGuide()->IsAwaitingConfirmation() || Stage->GetLauncher()->IsOpen()) { Error = TEXT("First dialogue did not await confirmation while locked"); return false; }
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderGuideDialogue.png"), false, false);
            SetPhase(3);
        }
    }
    else if (Phase == 3 && PhaseTime > .7f) { GuideTap(PC, EKeys::Gamepad_DPad_Down); SetPhase(4); }
    else if (Phase == 4 && PhaseTime > .4f)
    {
        if (!Progress->IsLauncherUnlocked() || !Stage->GetLauncher()->IsOpen() || Stage->GetGuide()->IsAwaitingConfirmation() || !UGameplayStatics::DoesSaveGameExist(Progress->GetSaveSlotName(), 0)) { Error = TEXT("Confirmation did not save and visibly open the launcher"); return false; }
        GuideTap(PC, EKeys::G); SetPhase(5);
    }
    else if (Phase == 5 && PhaseTime > .5f)
    {
        if (!Progress->IsLauncherUnlocked() || Stage->GetGuide()->IsAwaitingConfirmation() || !Player->GetInteraction()->GetMessage().Contains(TEXT("open"))) { Error = TEXT("Repeated dialogue did not remain idempotently unlocked"); return false; }
        PC->SetControlRotation(FRotator(-15, 0, 0));
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderLauncherOpen.png"), false, false);
        SetPhase(6);
    }
    else if (Phase == 6 && PhaseTime > .5f) { GuideTap(PC, EKeys::R); SetPhase(7); }
    else if (Phase == 7 && PhaseTime > .4f)
    {
        if (!Progress->IsLauncherUnlocked() || !Stage->GetLauncher()->IsOpen() || Stage->GetGuide()->IsAwaitingConfirmation() || !Player->GetInteraction()->GetMessage().IsEmpty() || Player->RecoveryCount != RecoveryBefore + 1) { Error = TEXT("R did not retain saved unlock and reset temporary dialogue"); return false; }
        UE_LOG(LogTemp, Display, TEXT("GUIDE TEST PASS: %s isolated slot %s; normal walking, distant interaction rejection, actual G/D-pad dialogue, idempotence, launcher visual and R persistence with unchanged quest rewards passed."), bReadProcess ? TEXT("loaded after process restart") : TEXT("unlocked through dialogue into a fresh"), *Progress->GetSaveSlotName());
        return true;
    }
    return false;
}
