#include "WindseedGameMode.h"
#include "WindseedCharacter.h"
#include "WindseedStage.h"
#include "AVIWriter.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void AWindseedHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;
    auto* Mode = Cast<AWindseedGameMode>(UGameplayStatics::GetGameMode(this));
    AWindseedStage* Garden = nullptr;
    TArray<AActor*> Stages;
    UGameplayStatics::GetAllActorsOfClass(this,AWindseedStage::StaticClass(),Stages);
    if (Stages.Num()) Garden=Cast<AWindseedStage>(Stages[0]);
    if (!Garden || !Mode) return;
    const float S=FMath::Clamp(Canvas->SizeX/1280.f,.6f,1.5f);
    auto Text=[&](const FString& Message,float X,float Y,float Scale,FLinearColor Color)
    { DrawText(Message,Color,X*S,Y*S,GEngine->GetMediumFont(),Scale*S,false); };
    DrawRect(FLinearColor(.025f,.07f,.1f,.92f),20*S,20*S,430*S,148*S);
    DrawRect(FLinearColor(1,.72f,.18f),20*S,20*S,5*S,148*S);
    Text(TEXT("WINDSEED GARDEN"),38,34,1.2f,FLinearColor(1,.88f,.58f));
    Text(FString::Printf(TEXT("WINDSEEDS   %d / 6"),Garden->CollectedCount),38,73,1.15f,FLinearColor::White);
    Text(Garden->CollectedCount==6 ? TEXT("BEACON READY - reach the flower tower") : TEXT("Explore the overlook and the beacon trail"),38,115,.7f,FLinearColor(.68f,.88f,.84f));
    const float Bottom=Canvas->SizeY-65*S;
    DrawRect(FLinearColor(.025f,.07f,.1f,.88f),20*S,Bottom,Canvas->SizeX-40*S,45*S);
    DrawText(TEXT("WASD / left stick: move    SPACE / A: jump    SHIFT / LB: run    Mouse / right stick: camera    R / B: restart"),FLinearColor::White,35*S,Bottom+13*S,GEngine->GetSmallFont(),.95f*S,false);
    DrawText(Garden->Hint,FLinearColor(1,.92f,.68f),30*S,Canvas->SizeY-100*S,GEngine->GetSmallFont(),1.05f*S,false);
    if (Garden->bFinished)
    {
        const float Width=540*S, X=(Canvas->SizeX-Width)*.5f, Y=Canvas->SizeY*.37f;
        DrawRect(FLinearColor(.02f,.1f,.12f,.96f),X,Y,Width,138*S);
        DrawText(TEXT("THE GARDEN IS AWAKE!"),FLinearColor(1,.8f,.24f),X+30*S,Y+25*S,GEngine->GetMediumFont(),1.4f*S,false);
        DrawText(TEXT("Six windseeds returned. Press R / B to explore again."),FLinearColor::White,X+30*S,Y+87*S,GEngine->GetSmallFont(),1.1f*S,false);
    }
}

AWindseedGameMode::AWindseedGameMode()
{
    PrimaryActorTick.bCanEverTick=true;
    DefaultPawnClass=AWindseedCharacter::StaticClass();
    HUDClass=AWindseedHUD::StaticClass();
}

void AWindseedGameMode::StartPlay()
{
    Stage=GetWorld()->SpawnActor<AWindseedStage>();
    Super::StartPlay();
    if (auto* Player=Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this,0)))
    {
        Player->ReturnToTrail();
        Player->RecoveryCount=0;
    }
#if !UE_BUILD_SHIPPING
    bSmokeTest=FParse::Param(FCommandLine::Get(),TEXT("GardenSmokeTest"));
    bCaptureRequested=FParse::Param(FCommandLine::Get(),TEXT("GardenCapture"));
    Route={FVector(-100,0,82),FVector(0,1100,182),FVector(400,1450,282),
        FVector(0,1100,182),FVector(0,400,82),FVector(450,100,82),
        FVector(1050,100,182),FVector(1500,-100,282),FVector(1950,150,382),FVector(2250,650,522)};
#endif
}

void AWindseedGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
#if !UE_BUILD_SHIPPING
    if (!bSmokeTest || !Stage) return;
    auto* Player=Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
    if (!Player) { FinishTest(false,TEXT("Player did not spawn")); return; }
    PhaseTime+=DeltaSeconds;
    if (GetWorld()->GetTimeSeconds()>2) CaptureFrame(DeltaSeconds);
    if (TestPhase==0 && PhaseTime>4)
    {
        if (Stage->CollectedCount!=0 || Stage->bFinished) { FinishTest(false,TEXT("Initial state is invalid")); return; }
        Player->SetActorLocation(FVector(-450,-250,-1000));
        TestPhase=1;
        PhaseTime=0;
    }
    else if (TestPhase==1 && PhaseTime>1)
    {
        if (Player->RecoveryCount!=1 || Player->GetActorLocation().Z<0) { FinishTest(false,TEXT("Fall recovery failed")); return; }
        Player->SetSprinting(true);
        if (Player->GetCharacterMovement()->MaxWalkSpeed<600) { FinishTest(false,TEXT("Sprint failed")); return; }
        Player->SetSprinting(false);
        TestPhase=2;
        PhaseTime=0;
        UE_LOG(LogTemp,Display,TEXT("WINDSEED TEST: recovery and sprint passed; walking the route using movement input."));
    }
    else if (TestPhase==2)
    {
        LegTime+=DeltaSeconds;
        if (LegTime>20 || Player->RecoveryCount>1) { FinishTest(false,FString::Printf(TEXT("Route failed at waypoint %d, position %s"),Waypoint,*Player->GetActorLocation().ToString())); return; }
        const FVector Offset=Route[Waypoint]-Player->GetActorLocation();
        if (Offset.Size2D()<55 && FMath::Abs(Offset.Z)<65 && Player->GetCharacterMovement()->IsMovingOnGround())
        {
            ++Waypoint; LegTime=0;
            Player->StopJumping();
            Player->GetCharacterMovement()->StopMovementImmediately();
            if (Waypoint==Route.Num()) { TestPhase=3; PhaseTime=0; return; }
            UE_LOG(LogTemp,Display,TEXT("WINDSEED TEST: reached waypoint %d"),Waypoint);
        }
        else
        {
            Player->AddMovementInput(Offset.GetSafeNormal2D());
            if (Offset.Z>50 && Offset.Size2D()<480 && Player->GetCharacterMovement()->IsMovingOnGround()) { Player->Jump(); ++JumpCount; }
            else if (Player->GetVelocity().Z<0) Player->StopJumping();
            if (auto* PC=Cast<APlayerController>(Player->GetController()))
            {
                const float Yaw=Offset.Rotation().Yaw;
                PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-23,Yaw,0),DeltaSeconds,2));
            }
        }
    }
    else if (TestPhase==3 && PhaseTime>3)
    {
        if (Stage->CollectedCount!=6 || !Stage->bFinished || JumpCount<4) { FinishTest(false,TEXT("Collection, jumping, or goal completion failed")); return; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/GardenComplete.png"),false,false);
        TestPhase=4; PhaseTime=0;
    }
    else if (TestPhase==4 && PhaseTime>2)
    {
        Stage->ResetTrail(); Player->ReturnToTrail();
        if (Stage->CollectedCount!=0 || Stage->bFinished) { FinishTest(false,TEXT("Restart failed")); return; }
        FinishTest(true,TEXT("Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed."));
    }
    if (PhaseTime>100) FinishTest(false,TEXT("Test timed out"));
#endif
}

void AWindseedGameMode::CaptureFrame(float DeltaSeconds)
{
#if !UE_BUILD_SHIPPING
    if (!bCaptureRequested || !GEngine || !GEngine->GameViewport || !GEngine->GameViewport->Viewport) return;
    FViewport* Viewport=GEngine->GameViewport->Viewport;
    const FIntPoint Size=Viewport->GetSizeXY();
    if (Size.X<=0 || Size.Y<=0) return;
    if (!CaptureWriter)
    {
        FAVIWriterOptions Options;
        Options.OutputFilename=FPaths::ProjectSavedDir()/TEXT("Validation/WindseedPlaythrough.avi");
        IFileManager::Get().MakeDirectory(*(FPaths::ProjectSavedDir()/TEXT("Validation")),true);
        Options.Width=Size.X; Options.Height=Size.Y;
        Options.CaptureFramerateNumerator=15;
        Options.CompressionQuality=.75f;
        Options.bSynchronizeFrames=true;
        CaptureWriter=FAVIWriter::CreateInstance(Options);
        if (CaptureWriter) CaptureWriter->Initialize();
        if (!CaptureWriter || !CaptureWriter->IsCapturing())
        {
            UE_LOG(LogTemp,Error,TEXT("WINDSEED CAPTURE: AVI capture could not start."));
            bCaptureRequested=false;
            return;
        }
    }
    CaptureTime+=DeltaSeconds;
    if (CaptureTime<NextCapture) return;
    TArray<FColor> Pixels;
    if (Viewport->ReadPixels(Pixels)) CaptureWriter->Update(CaptureTime,MoveTemp(Pixels));
    NextCapture=CaptureTime+1.f/15.f;
#endif
}

void AWindseedGameMode::FinishTest(bool bPassed,const FString& Message)
{
    bSmokeTest=false;
    if (CaptureWriter) { CaptureWriter->Finalize(); delete CaptureWriter; CaptureWriter=nullptr; }
    const FString Report=FString::Printf(TEXT("%s\n%s\nJumps: %d\n"),bPassed?TEXT("PASS"):TEXT("FAIL"),*Message,JumpCount);
    IFileManager::Get().MakeDirectory(*(FPaths::ProjectSavedDir()/TEXT("Validation")),true);
    FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectSavedDir()/TEXT("Validation/Result.txt")));
    UE_LOG(LogTemp,Display,TEXT("WINDSEED TEST %s: %s"),bPassed?TEXT("PASS"):TEXT("FAIL"),*Message);
    FPlatformMisc::RequestExitWithStatus(false,bPassed?0:1);
}

void AWindseedGameMode::EndPlay(const EEndPlayReason::Type Reason)
{
    if (CaptureWriter) { CaptureWriter->Finalize(); delete CaptureWriter; CaptureWriter=nullptr; }
    Super::EndPlay(Reason);
}
