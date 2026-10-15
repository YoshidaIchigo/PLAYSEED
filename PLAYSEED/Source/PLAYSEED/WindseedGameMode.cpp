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
#include "InputKeyEventArgs.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#include "RHICommandList.h"
#include "RenderingThread.h"

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
    Text(TEXT("WINDSEED GARDEN"),38,34,1.8f,FLinearColor(1,.88f,.58f));
    Text(FString::Printf(TEXT("WINDSEEDS   %d / 6"),Garden->CollectedCount),38,73,1.7f,FLinearColor::White);
    Text(Garden->CollectedCount==6 ? TEXT("BEACON READY - reach the flower tower") : TEXT("Explore the overlook and the beacon trail"),38,121,1.05f,FLinearColor(.68f,.88f,.84f));
    const float Bottom=Canvas->SizeY-90*S;
    DrawRect(FLinearColor(.025f,.07f,.1f,.88f),20*S,Bottom,Canvas->SizeX-40*S,70*S);
    DrawText(TEXT("WASD / left stick: move     SPACE / A: jump     SHIFT / LB: run"),FLinearColor::White,35*S,Bottom+10*S,GEngine->GetMediumFont(),1.3f*S,false);
    DrawText(TEXT("Mouse / right stick: camera     Wheel: zoom     R / B: restart     TAB: release mouse"),FLinearColor::White,35*S,Bottom+37*S,GEngine->GetMediumFont(),1.3f*S,false);
    DrawText(Garden->Hint,FLinearColor(1,.92f,.68f),30*S,Canvas->SizeY-123*S,GEngine->GetMediumFont(),1.35f*S,false);
    if (Garden->bFinished)
    {
        const float Width=540*S, X=(Canvas->SizeX-Width)*.5f, Y=Canvas->SizeY*.37f;
        DrawRect(FLinearColor(.02f,.1f,.12f,.96f),X,Y,Width,138*S);
        DrawText(TEXT("THE GARDEN IS AWAKE!"),FLinearColor(1,.8f,.24f),X+30*S,Y+25*S,GEngine->GetMediumFont(),1.9f*S,false);
        DrawText(TEXT("Six seeds returned. Press R / B to explore again."),FLinearColor::White,X+30*S,Y+87*S,GEngine->GetMediumFont(),1.3f*S,false);
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
        // GameMode は現在のステージのルールを担当します。
        // Super::StartPlay() で Actor の BeginPlay が始まった後、存在するプレイヤーへ開始位置を渡します。
        Player->SetTrailStartLocation(FVector(-450,-250,100));
        Player->ReturnToTrail();
        Player->RecoveryCount=0;
    }
#if !UE_BUILD_SHIPPING
    bSmokeTest=FParse::Param(FCommandLine::Get(),TEXT("GardenSmokeTest"));
    bCaptureRequested=FParse::Param(FCommandLine::Get(),TEXT("GardenCapture"));
    if (bCaptureRequested && GEngine && GEngine->GameViewport)
        CaptureDrawHandle=GEngine->GameViewport->OnEndDraw().AddUObject(this,&AWindseedGameMode::CaptureViewport);
    Route={FVector(-100,0,82),FVector(0,1100,182),FVector(400,1450,282),
        FVector(0,1100,182),FVector(0,400,82),FVector(450,100,82),
        FVector(1050,100,182),FVector(1500,-100,282),FVector(1950,150,382),FVector(2150,650,506)};
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
    if (TestPhase==0 && PhaseTime>4)
    {
        if (Stage->CollectedCount!=0 || Stage->bFinished) { FinishTest(false,TEXT("Initial state is invalid")); return; }
        if (FVector::Dist2D(Player->GetActorLocation(),FVector(-450,-250,100))>5)
        { FinishTest(false,TEXT("The original stage start position changed")); return; }
        // 落下復帰の設定を別の安全な座標に変えて、本当にその場所へ戻るかを確認します。
        // 意図的に落とすのはこの復帰検証だけで、後の収集ルートは通常の移動・ジャンプで進みます。
        Player->SetTrailStartLocation(FVector(-650,-350,100));
        Player->SetActorLocation(FVector(-450,-250,-1000));
        TestPhase=1;
        PhaseTime=0;
    }
    else if (TestPhase==1 && PhaseTime>1)
    {
        if (Player->RecoveryCount!=1 || Player->GetActorLocation().Z<0) { FinishTest(false,TEXT("Fall recovery failed")); return; }
        if (FVector::Dist2D(Player->GetActorLocation(),FVector(-650,-350,100))>5)
        { FinishTest(false,TEXT("The configured recovery position was not used")); return; }
        UE_LOG(LogTemp,Display,TEXT("WINDSEED TEST: configured recovery position passed."));
        // 以後は従来の開始位置へ戻し、既存のステージ全体を回帰検証します。
        Player->SetTrailStartLocation(FVector(-450,-250,100));
        Player->ReturnToTrail();
        auto* PC=Cast<APlayerController>(Player->GetController());
        if (!PC) { FinishTest(false,TEXT("Player controller missing")); return; }
        InputTestStart=Player->GetActorLocation();
        PC->SetControlRotation(FRotator(-18,0,0));
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::W,IE_Pressed,1));
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftShift,IE_Pressed,1));
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseX,IE_Axis,10));
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseWheelAxis,IE_Axis,1));
        TestPhase=10;
        PhaseTime=0;
    }
    else if (TestPhase==10 && PhaseTime>.45f)
    {
        auto* PC=Cast<APlayerController>(Player->GetController());
        const bool bInputWorked=Player->GetActorLocation().X>InputTestStart.X+100 && Player->GetActorLocation().Z>InputTestStart.Z+40 &&
            Player->GetCharacterMovement()->MaxWalkSpeed>600 && FMath::Abs(PC->GetControlRotation().Yaw)>.1f && Player->GetCameraBoom()->TargetArmLength<560;
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::W,IE_Released,0));
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftShift,IE_Released,0));
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Released,0));
        if (!bInputWorked) { FinishTest(false,TEXT("Keyboard movement, sprint, jump, mouse look or zoom binding failed")); return; }
        Player->ReturnToTrail();
        Player->GetCameraBoom()->TargetArmLength=560;
        RouteRecoveryBaseline=Player->RecoveryCount;
        TestPhase=2; PhaseTime=0;
        UE_LOG(LogTemp,Display,TEXT("WINDSEED TEST: recovery and simulated W/Shift/Space/mouse/zoom input bindings passed; walking the route."));
    }
    else if (TestPhase==2)
    {
        LegTime+=DeltaSeconds;
        if (LegTime>20 || Player->RecoveryCount!=RouteRecoveryBaseline) { FinishTest(false,FString::Printf(TEXT("Route failed at waypoint %d, position %s"),Waypoint,*Player->GetActorLocation().ToString())); return; }
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
        if (auto* PC=Cast<APlayerController>(Player->GetController())) PC->SetControlRotation(FRotator(-25,-90,0));
        TestPhase=4; PhaseTime=0;
    }
    else if (TestPhase==4 && PhaseTime>2)
    {
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/GardenComplete.png"),false,false);
        TestPhase=5; PhaseTime=0;
    }
    else if (TestPhase==5 && PhaseTime>1)
    {
        Stage->ResetTrail(); Player->ReturnToTrail();
        if (Stage->CollectedCount!=0 || Stage->bFinished) { FinishTest(false,TEXT("Restart failed")); return; }
        FinishTest(true,TEXT("Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed."));
    }
    if (PhaseTime>100) FinishTest(false,TEXT("Test timed out"));
#endif
}

void AWindseedGameMode::CaptureViewport()
{
#if !UE_BUILD_SHIPPING
    if (bSmokeTest && GetWorld()->GetTimeSeconds()>2) CaptureFrame(GetWorld()->GetDeltaSeconds());
#endif
}

void AWindseedGameMode::CaptureFrame(float DeltaSeconds)
{
#if !UE_BUILD_SHIPPING
    if (!bCaptureRequested || bCaptureFailed || !GEngine || !GEngine->GameViewport || !GEngine->GameViewport->Viewport) return;
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
            bCaptureFailed=true;
            return;
        }
    }
    CaptureTime+=DeltaSeconds;
    if (CaptureTime<NextCapture) return;
    TArray<FColor> Pixels;
    ENQUEUE_RENDER_COMMAND(WindseedReadback)([Viewport,Size,&Pixels](FRHICommandListImmediate& RHICmdList)
    {
        const FTextureRHIRef& Texture=Viewport->GetShaderResourceTexture();
        if (Texture.IsValid())
            RHICmdList.ReadSurfaceData(Texture,FIntRect(0,0,Size.X,Size.Y),Pixels,FReadSurfaceDataFlags(RCM_UNorm));
    });
    FlushRenderingCommands();
    if (Pixels.Num()==Size.X*Size.Y) CaptureWriter->Update(CaptureTime,MoveTemp(Pixels));
    NextCapture=CaptureTime+1.f/15.f;
#endif
}

void AWindseedGameMode::FinishTest(bool bPassed,const FString& Message)
{
    bSmokeTest=false;
    const int32 CapturedFrames=CaptureWriter ? CaptureWriter->GetFrameNumber() : 0;
    FString ResultMessage=Message;
    if (bCaptureRequested && (bCaptureFailed || CapturedFrames<150))
    {
        bPassed=false;
        ResultMessage+=TEXT(" Capture failed or contains fewer than 150 frames.");
    }
    if (CaptureWriter) { CaptureWriter->Finalize(); delete CaptureWriter; CaptureWriter=nullptr; }
    const FString Report=FString::Printf(TEXT("%s\n%s\nJumps: %d\nCaptureFrames: %d\nUTC: %s\n"),bPassed?TEXT("PASS"):TEXT("FAIL"),*ResultMessage,JumpCount,CapturedFrames,*FDateTime::UtcNow().ToIso8601());
    IFileManager::Get().MakeDirectory(*(FPaths::ProjectSavedDir()/TEXT("Validation")),true);
    FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectSavedDir()/TEXT("Validation/Result.txt")));
    UE_LOG(LogTemp,Display,TEXT("WINDSEED TEST %s: %s"),bPassed?TEXT("PASS"):TEXT("FAIL"),*ResultMessage);
    FPlatformMisc::RequestExitWithStatus(false,bPassed?0:1);
}

void AWindseedGameMode::EndPlay(const EEndPlayReason::Type Reason)
{
    if (GEngine && GEngine->GameViewport) GEngine->GameViewport->OnEndDraw().Remove(CaptureDrawHandle);
    if (CaptureWriter) { CaptureWriter->Finalize(); delete CaptureWriter; CaptureWriter=nullptr; }
    Super::EndPlay(Reason);
}
