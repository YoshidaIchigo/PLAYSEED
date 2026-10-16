#include "ThunderLivesTestSubsystem.h"
#include "ThunderLivesComponent.h"
#include "ThunderLifeseed.h"
#include "ThunderMeadowStage.h"
#include "ThunderMeadowGameMode.h"
#include "ThunderDamageThistle.h"
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
void UThunderLivesTestSubsystem::BeginTest(AWindseedCharacter* Player,AThunderMeadowStage* InStage)
{
    Stage=InStage; Error.Reset(); Phase=Waypoint=FallIndex=0; PhaseTime=0; bObservedGameOver=false;
    HitsBefore=Player->GetVitality()->GetSuccessfulImpacts(); DefeatsBefore=Player->GetVitality()->GetDefeatCount(); RecoveryBefore=Player->RecoveryCount;
    auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if(!Progress->GetSaveSlotName().StartsWith(TEXT("Validation_")) || !ThunderHasExactRewards(Progress,{1,2,3,4,5,6,7})) { Error=TEXT("Lives test requires actually earned all7 validation save"); return; }
    Route={FVector(-2800,-600,82),FVector(-2800,800,82),FVector(-2200,800,82)};
}
void UThunderLivesTestSubsystem::UpdateTest(AWindseedCharacter* Player,float DeltaSeconds)
{
    if(!Error.IsEmpty()) return;
    auto* Mode=Cast<AThunderMeadowGameMode>(Player->GetWorld()->GetAuthGameMode()); auto* PC=Cast<APlayerController>(Player->GetController());
    auto* Item=Stage.IsValid()?Stage->GetLifeseed():nullptr; auto* Vitality=Player->GetVitality(); auto* Move=Player->GetCharacterMovement();
    const auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if(!Mode || !PC || !Item) { Error=TEXT("Lives stage/controller/item missing"); return; }
    auto* Lives=Mode->GetLives(); PhaseTime+=DeltaSeconds;
    if(PhaseTime>55) { Error=FString::Printf(TEXT("Lives phase%d waypoint%d timeout at%s"),Phase,Waypoint,*Player->GetActorLocation().ToString()); return; }
    if(!ThunderHasExactRewards(Progress,{1,2,3,4,5,6,7}) || !Progress->GetProgressProblem().IsEmpty()) { Error=TEXT("Life pickup/death changed actual all7 rewards"); return; }
    auto Next=[this](int32 N){Phase=N;PhaseTime=0;UE_LOG(LogTemp,Display,TEXT("LIVES TEST phase%d fall%d"),N,FallIndex);};
    auto Stop=[Move](){if(Move->IsMovingOnGround())Move->StopMovementImmediately();};
    auto TapR=[PC](){PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R,IE_Pressed,1));PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R,IE_Released,0));};
    auto Walk=[Player,Move,PC,DeltaSeconds](FVector Goal)
    {
        const FVector Offset=Goal-Player->GetActorLocation(); PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-20,Offset.Rotation().Yaw,0),DeltaSeconds,3));
        if(Offset.Size2D()<35 && FMath::Abs(Offset.Z)<65 && Move->IsMovingOnGround()){Move->StopMovementImmediately();return true;}
        Player->AddMovementInput(Offset.GetSafeNormal2D(),FMath::Clamp(Offset.Size2D()/100,.2f,1.f));return false;
    };
    if(Phase==0)
    {
        if(Item->IsCollected())
        {
            Stop(); if(Lives->GetRemaining()!=4 || Item->GetSuccessfulCollections()!=1 || Vitality->GetHealth()!=8){Error=TEXT("Actual lifeseed contact did not increase3 to4 once");return;}
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderLifePickup.png"),false,false);Next(1);
        }
        else if(Waypoint<Route.Num() && Walk(Route[Waypoint])){++Waypoint;PhaseTime=0;}
    }
    else if(Phase==1 && PhaseTime>1)
    {
        if(Lives->GetRemaining()!=4 || Item->GetSuccessfulCollections()!=1){Error=TEXT("Repeated overlap duplicated a life");return;} TapR();Next(2);
    }
    else if(Phase==2 && PhaseTime>.5f)
    {
        if(Lives->GetRemaining()!=4 || !Item->IsCollected() || Item->GetSuccessfulCollections()!=1 || Player->RecoveryCount!=RecoveryBefore+1){Error=TEXT("R consumed a life or respawned an already collected lifeseed");return;}
        Next(3);
    }
    else if(Phase==3)
    {
        if(Vitality->GetDefeatCount()==DefeatsBefore+1)
        {
            Stop();if(Lives->GetRemaining()!=3 || Lives->GetConsumedLives()!=1 || Vitality->GetSuccessfulImpacts()!=HitsBefore+4 || Vitality->GetHealth()!=8 || Player->RecoveryCount!=RecoveryBefore+2 || !Item->IsCollected()){Error=TEXT("Four real thistle hits did not consume exactly one life and restart safely");return;}
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderLifeDefeat.png"),false,false);Next(4);
        }
        else Walk(Stage->GetDamageThistle()->GetActorLocation()+FVector(0,0,82));
    }
    else if(Phase==4 && PhaseTime>.6f){Waypoint=0;Route={FVector(-3100,-600,82),FVector(-4100,-600,82)};Next(5);}
    else if(Phase==5)
    {
        if(Vitality->GetDefeatCount()==DefeatsBefore+2+FallIndex)
        {
            const int32 Expected=2-FallIndex;
            if(Lives->GetRemaining()!=Expected || Lives->GetConsumedLives()!=2+FallIndex || Vitality->GetSuccessfulImpacts()!=HitsBefore+4){Error=TEXT("Actual walk off cliff consumed the wrong number of lives");return;}
            if(Expected>0)
            {
                Stop();if(Player->RecoveryCount!=RecoveryBefore+3+FallIndex || Vitality->GetHealth()!=8){Error=TEXT("Surviving cliff defeat did not restart once");return;}
                ++FallIndex;Next(4);
            }
            else
            {
                if(!Lives->IsGameOver() || Vitality->GetHealth()!=0 || Player->RecoveryCount!=RecoveryBefore+4){Error=TEXT("Last life did not enter game over without a free restart");return;} Next(6);
            }
        }
        // 実際の床の縁から歩いて落ちます。落下確認にも座標置換を使いません。
        else if(Waypoint<Route.Num() && Walk(Route[Waypoint])){++Waypoint;PhaseTime=0;}
    }
    else if(Phase==6 && PhaseTime>.2f)
    {
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderGameOver.png"),false,false);TapR();Next(7);
    }
    else if(Phase==7 && PhaseTime>1.7f)
    {
        if(!Lives->IsGameOver() || Lives->GetRemaining()!=0 || Lives->GetConsumedLives()!=4 || Vitality->GetDefeatCount()!=DefeatsBefore+4 || Vitality->GetHealth()!=0 || Player->RecoveryCount!=RecoveryBefore+4){Error=TEXT("Game over allowed duplicate deaths or an R bypass");return;}
        bObservedGameOver=true;Next(8);
        UE_LOG(LogTemp,Display,TEXT("LIVES TEST: actual pickup3to4 once, R free/no item respawn, four real thistle hits4to3, three actual walk-off falls3to2to1to0, gameover holds0/no R bypass, actual all7 retained; awaiting real menu and retry."));
    }
}
