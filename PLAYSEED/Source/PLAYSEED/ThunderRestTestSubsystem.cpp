#include "ThunderRestTestSubsystem.h"
#include "ThunderMountainRest.h"
#include "ThunderMeadowStage.h"
#include "ThunderDamageThistle.h"
#include "ThunderRollingBoulder.h"
#include "ThunderExpeditionSubsystem.h"
#include "ThunderCampaignValidation.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Engine/GameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void UThunderRestTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage)
{
    Stage = InStage; Error.Reset(); Phase = Waypoint = 0; PhaseTime = 0; HitsBefore = Player->GetVitality()->GetSuccessfulImpacts(); RecoveryBefore = Player->RecoveryCount;
    const auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if (!Progress->GetSaveSlotName().StartsWith(TEXT("Validation_")) || !ThunderHasExactRewards(Progress, {1,2,3,4,5,6,7})) { Error = TEXT("Rest test requires the actually earned all7 validation profile"); return; }
    Route = {FVector(-700,-1200,82), FVector(-700,-600,82), FVector(0,1100,82), FVector(600,1850,82)};
    for (int32 I = 0; I <= 11; ++I)
    {
        const FVector Foot = InStage->GetMountainFootPath()[I];
        Route.Add(Foot + (I >= 4 ? (Foot - FVector(2200,2100,0)).GetSafeNormal2D() * 150 : FVector::ZeroVector) + FVector(0,0,82));
        if (I == 3) Route.Add(Foot + (Foot - FVector(2200,2100,0)).GetSafeNormal2D() * 150 + FVector(0,0,82));
    }
    Route.Add(InStage->GetMountainRest()->GetActorLocation() + FVector(0,0,82));
}
bool UThunderRestTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* Rest = Stage.IsValid() ? Stage->GetMountainRest() : nullptr; auto* PC = Cast<APlayerController>(Player->GetController());
    auto* Vitality = Player->GetVitality(); auto* Move = Player->GetCharacterMovement(); const auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if (!Rest || !PC) { Error = TEXT("Rest stage/controller missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 160) { Error = FString::Printf(TEXT("Rest phase%d waypoint%d timeout at%s"), Phase, Waypoint, *Player->GetActorLocation().ToString()); return false; }
    if (!ThunderHasExactRewards(Progress, {1,2,3,4,5,6,7}) || !Progress->GetProgressProblem().IsEmpty()) { Error = TEXT("Rest changed actual saved all7 rewards"); return false; }
    if (Phase < 9 && Player->RecoveryCount != RecoveryBefore) { Error = TEXT("Rest route unexpectedly recovered"); return false; }
    auto Next = [this](int32 N) { Phase=N; PhaseTime=0; UE_LOG(LogTemp,Display,TEXT("REST TEST phase%d"),N); };
    auto Stop = [Move]() { if (Move->IsMovingOnGround()) Move->StopMovementImmediately(); };
    auto Walk = [Player,PC,Move,DeltaSeconds](FVector Goal)
    {
        const FVector Offset=Goal-Player->GetActorLocation();
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-24,Offset.Rotation().Yaw,0),DeltaSeconds,3));
        if (Offset.Size2D()<28 && FMath::Abs(Offset.Z)<70 && Move->IsMovingOnGround()) { Move->StopMovementImmediately(); return true; }
        Player->AddMovementInput(Offset.GetSafeNormal2D(),FMath::Clamp(Offset.Size2D()/100,.15f,1.f)); return false;
    };
    const FVector RestGoal=Rest->GetActorLocation()+FVector(0,0,82);
    const FVector Foot10=Stage->GetMountainFootPath()[10]; const FVector LeaveGoal=Foot10+(Foot10-FVector(2200,2100,0)).GetSafeNormal2D()*150+FVector(0,0,82);
    if (Phase == 0)
    {
        if (Vitality->GetSuccessfulImpacts()==HitsBefore+2)
        {
            if (Vitality->GetHealth()!=4) { Error=TEXT("Two real thistle contacts did not give health4"); return false; }
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderRestDamage.png"),false,false); Next(1);
        }
        else Walk(Stage->GetDamageThistle()->GetActorLocation()+FVector(0,0,82));
    }
    else if (Phase == 1)
    {
        if (Vitality->GetSuccessfulImpacts()!=HitsBefore+2) { Error=TEXT("Mountain recovery route took an extra hit"); return false; }
        // 入口の低い洞窟は中央を歩き、岩が下端で待ってから3へ進みます。
        if (Waypoint==6 && FVector::Dist2D(Player->GetActorLocation(),Route[6])<45 && !Stage->GetRollingBoulder()->IsWaiting()) Stop();
        else if (Waypoint<Route.Num() && Walk(Route[Waypoint])) { ++Waypoint; PhaseTime=0; }
        if (Rest->GetSuccessfulHeals()==1)
        {
            if (Vitality->GetHealth()!=6 || Vitality->IsInvulnerable()) { Error=TEXT("Real rest contact did not heal exactly2 without granting invulnerability"); return false; }
            Stop(); FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderRestFirst.png"),false,false); Next(2);
        }
    }
    else if (Phase == 2)
    {
        Stop(); if (Rest->GetSuccessfulHeals()!=1 || Vitality->GetHealth()!=6) { Error=TEXT("Staying on the flower bypassed leave/cooldown rule"); return false; }
        if (PhaseTime>6) Next(3);
    }
    else if (Phase == 3) { if (Walk(LeaveGoal)) { if (!Rest->IsReady()) { Error=TEXT("Flower did not rearm after five seconds and leaving"); return false; } Next(4); } }
    else if (Phase == 4)
    {
        Walk(RestGoal); if (Rest->GetSuccessfulHeals()==2) { Stop(); if (Vitality->GetHealth()!=8) { Error=TEXT("Second real contact did not reach maximum8"); return false; } FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderRestFull.png"),false,false); Next(5); }
    }
    else if (Phase == 5) { Stop(); if (PhaseTime>6) Next(6); }
    else if (Phase == 6) { if (Walk(LeaveGoal)) Next(7); }
    else if (Phase == 7) { if (Walk(RestGoal)) Next(8); }
    else if (Phase == 8 && PhaseTime>1)
    {
        if (Vitality->GetHealth()!=8 || Rest->GetSuccessfulHeals()!=2 || !Rest->IsReady()) { Error=TEXT("Full health consumed or exceeded a healing charge"); return false; }
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R,IE_Pressed,1)); PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R,IE_Released,0)); Next(9);
    }
    else if (Phase == 9 && PhaseTime>.5f)
    {
        if (Player->RecoveryCount!=RecoveryBefore+1 || Vitality->GetHealth()!=8 || !Rest->IsReady() || Rest->GetSuccessfulHeals()!=2) { Error=TEXT("R did not restore ready rest and health while retaining counters"); return false; }
        UE_LOG(LogTemp,Display,TEXT("REST TEST PASS: actual thistle health4, ordinary mountain walk, heal6, no repeated stay heal, leave/cooldown, heal8, full cap, R and actual saved all7 retained.")); return true;
    }
    return false;
}
