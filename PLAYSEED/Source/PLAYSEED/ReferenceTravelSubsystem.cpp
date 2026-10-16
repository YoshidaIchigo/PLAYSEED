#include "ReferenceTravelSubsystem.h"
#include "ReferenceHero.h"
#include "PlayseedLantern.h"
#include "PlayseedActionComponent.h"
#include "PlayseedCarryComponent.h"
#include "PlayseedClimbComponent.h"
#include "PlayseedCombatComponent.h"
#include "PlayseedFlightComponent.h"
#include "PlayseedLaunchComponent.h"
#include "PlayseedRideComponent.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"

void UReferenceTravelSubsystem::ArriveAtPainting()
{
    bEntranceFlow=true;State=EReferenceTravelState::AtPainting;
    CapturedHero.Reset();ShotCamera.Reset();DisplayStar.Reset();bPoseStarted=false;CelebrationTime=PoseTime=0;
}
bool UReferenceTravelSubsystem::BeginPainting()
{
    if(State!=EReferenceTravelState::AtPainting||IsTransitionLocked())return false;
    LastRewardNumber=0;
    State=EReferenceTravelState::EnteringPainting;return true;
}
bool UReferenceTravelSubsystem::FinishPainting()
{
    if(State!=EReferenceTravelState::EnteringPainting)return false;
    State=EReferenceTravelState::SelectingStar;return true;
}
bool UReferenceTravelSubsystem::BeginCourse()
{
    if(IsTransitionLocked()||(bEntranceFlow&&State!=EReferenceTravelState::SelectingStar))return false;
    if(!BeginFadeTo(FName(TEXT("/Game/ThunderMeadow/ThunderMeadow"))))return false;
    bEntranceFlow=true;State=EReferenceTravelState::Dropping;return true;
}
bool UReferenceTravelSubsystem::FinishDrop()
{
    if(State!=EReferenceTravelState::Dropping||IsFading())return false;
    State=EReferenceTravelState::InCourse;return true;
}
bool UReferenceTravelSubsystem::BeginReturn()
{
    if(!bEntranceFlow||State==EReferenceTravelState::Returning||IsFading())return false;
    if(!BeginFadeTo(FName(TEXT("/Game/Battlefield/Entrance"))))return false;
    State=EReferenceTravelState::Returning;return true;
}
bool UReferenceTravelSubsystem::IsTransitionLocked() const
{
    return IsFading()||State==EReferenceTravelState::EnteringPainting||State==EReferenceTravelState::Dropping||State==EReferenceTravelState::Celebrating||State==EReferenceTravelState::Returning;
}

bool UReferenceTravelSubsystem::StartPaintingFade()
{
    return State==EReferenceTravelState::EnteringPainting&&BeginFadeTo(FName(TEXT("/Game/ThunderMeadow/MissionSelect")));
}
bool UReferenceTravelSubsystem::BeginFadeTo(FName Destination)
{
    if(IsFading()||!GetWorld())return false;
    FadeDestination=Destination;FadeSourceWorld=GetWorld();FadeClock=FadeOpacity=0;FadePhase=EReferenceFadePhase::Out;++FadeSequence;
    HoldIncomingInput();
    UE_LOG(LogTemp,Display,TEXT("REFERENCE FADE %d: OUT white rectangle 1.00s to %s"),FadeSequence,*Destination.ToString());
    return true;
}
void UReferenceTravelSubsystem::HoldIncomingInput()
{
    if(!IsFading())return;
    auto* PC=UGameplayStatics::GetPlayerController(GetWorld(),0);if(!PC||FadeLockedController.Get()==PC)return;
    // Controllerの入力も止めます。新Worldで作られたControllerには改めて一度だけ設定します。
    FadeLockedController=PC;PC->SetIgnoreMoveInput(true);PC->SetIgnoreLookInput(true);
    if(auto* Hero=Cast<AWindseedCharacter>(PC->GetPawn()))
    {Hero->StopJumping();Hero->GetCharacterMovement()->StopMovementImmediately();Hero->GetCharacterMovement()->ClearAccumulatedForces();Hero->DisableInput(PC);}
}
void UReferenceTravelSubsystem::UpdateFade(float DeltaSeconds)
{
    if(!IsFading())return;
    if(FadePhase==EReferenceFadePhase::Out)
    {
        FadeClock+=DeltaSeconds;FadeOpacity=FMath::Clamp(FadeClock/1.f,0.f,1.f);
        if(FadeOpacity<1)return;
        // 完全に白くしてから一度だけOpenLevelします。白はGameInstanceが次のWorldへ持ち越します。
        FadePhase=EReferenceFadePhase::Covered;FadeClock=0;
        if(FadeDestination==FName(TEXT("/Game/ThunderMeadow/MissionSelect")))FinishPainting();
        else if(FadeDestination==FName(TEXT("/Game/Battlefield/Entrance")))State=EReferenceTravelState::Returning;
        UE_LOG(LogTemp,Display,TEXT("REFERENCE FADE %d: OPAQUE OpenLevel once, alpha %.2f"),FadeSequence,FadeOpacity);
        UGameplayStatics::OpenLevel(this,FadeDestination);return;
    }
    if(FadePhase==EReferenceFadePhase::Covered)
    {
        if(GetWorld()==FadeSourceWorld.Get()||UGameplayStatics::GetCurrentLevelName(GetWorld(),true)!=FPackageName::GetShortName(FadeDestination.ToString()))return;
        auto* PC=UGameplayStatics::GetPlayerController(GetWorld(),0);
        if(!PC||!PC->GetHUD()||(State!=EReferenceTravelState::SelectingStar&&!PC->GetPawn()))return;
        HoldIncomingInput();FadeClock+=DeltaSeconds;
        // 新しいカメラとHUDが揃ってから、白いフレームを少なくとも0.15秒表示します。
        if(FadeClock>=.15f){FadePhase=EReferenceFadePhase::In;FadeClock=0;UE_LOG(LogTemp,Display,TEXT("REFERENCE FADE %d: IN new World ready, white hold >=0.15s, reveal 0.75s"),FadeSequence);}
        return;
    }
    FadeClock+=DeltaSeconds;FadeOpacity=1-FMath::Clamp(FadeClock/.75f,0.f,1.f);
    if(FadeOpacity>0)return;
    FadePhase=EReferenceFadePhase::None;++CompletedFades;
    if(auto* PC=FadeLockedController.Get())
    {
        PC->SetIgnoreMoveInput(false);PC->SetIgnoreLookInput(false);
        if(State==EReferenceTravelState::AtPainting)if(auto* Hero=Cast<AWindseedCharacter>(PC->GetPawn()))Hero->EnableInput(PC);
    }
    FadeLockedController.Reset();FadeSourceWorld.Reset();FadeDestination=NAME_None;
    UE_LOG(LogTemp,Display,TEXT("REFERENCE FADE %d: COMPLETE alpha0, input released; completed%d"),FadeSequence,CompletedFades);
}

bool UReferenceTravelSubsystem::Celebrate(AWindseedCharacter* Hero,int32 RewardNumber)
{
    if(!Hero||State!=EReferenceTravelState::InCourse||RewardNumber<1||RewardNumber>7)return false;
    // 保存が成功した呼び出しからだけ開始します。100枚コインを含む全IDで同じ帰還経路です。
    State=EReferenceTravelState::Celebrating;bEntranceFlow=true;LastRewardNumber=RewardNumber;
    CapturedHero=Hero;CelebrationTime=PoseTime=0;bPoseStarted=false;
    Hero->GetLaunch()->CancelAim();Hero->GetFlight()->CancelFlight();Hero->GetRide()->CancelRide();
    Hero->GetClimb()->CancelClimb();Hero->GetActions()->CancelGroundPound();Hero->GetCarry()->DropHeld();Hero->GetCombat()->CancelMelee();
    Hero->StopJumping();Hero->GetCharacterMovement()->StopMovementImmediately();Hero->GetCharacterMovement()->ClearAccumulatedForces();
    if(auto* PC=Cast<APlayerController>(Hero->GetController()))Hero->DisableInput(PC);
    return true;
}

void UReferenceTravelSubsystem::StartStarPose()
{
    auto* Hero=CapturedHero.Get();if(!Hero)return;
    bPoseStarted=true;PoseTime=0;Hero->GetCharacterMovement()->StopMovementImmediately();Hero->GetCharacterMovement()->DisableMovement();
    const FVector Target=Hero->GetActorLocation()+FVector(0,0,60);
    FVector CameraPoint=Hero->GetActorLocation()-Hero->GetActorForwardVector()*900+FVector(0,0,150);float BestDistance=-1;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(StarPoseCamera),true,Hero);
    // スター6のような壁際でも撮れるよう、入口側から順にカメラの見通しを調べます。
    for(float Offset:{180.f,135.f,225.f,90.f,270.f,0.f})
    {
        const FVector Direction=FRotator(0,Hero->GetActorRotation().Yaw+Offset,0).Vector();
        const FVector Desired=Hero->GetActorLocation()+Direction*900+FVector(0,0,150);FHitResult Wall;
        const bool bHit=GetWorld()->LineTraceSingleByChannel(Wall,Target,Desired,ECC_Camera,Params);
        const FVector Candidate=bHit?Wall.ImpactPoint-(Desired-Target).GetSafeNormal()*55:Desired;
        const float Distance=FVector::Dist(Candidate,Target);
        if(Distance>BestDistance){BestDistance=Distance;CameraPoint=Candidate;}if(!bHit)break;
    }
    Hero->SetActorRotation(FRotator(0,(CameraPoint-Hero->GetActorLocation()).Rotation().Yaw,0));
    if(auto* Model=Cast<AReferenceHero>(Hero))Model->SetStarPose(true);
    auto* Camera=GetWorld()->SpawnActor<ACameraActor>(CameraPoint,(Target-CameraPoint).Rotation());ShotCamera=Camera;
    if(Camera)
    {
        auto* Lens=Camera->GetCameraComponent();Lens->FieldOfView=60;Lens->SetAspectRatio(4.f/3.f);Lens->bConstrainAspectRatio=true;
        if(auto* PC=Cast<APlayerController>(Hero->GetController()))PC->SetViewTargetWithBlend(Camera,.35f,VTBlend_Cubic);
    }
    // 番号0の表示専用Actorは収集しません。左手に小さなスター、右手はピースのポーズです。
    DisplayStar=GetWorld()->SpawnActor<APlayseedLantern>(Hero->GetActorTransform().TransformPosition(FVector(35,-80,15)),FRotator(0,Hero->GetActorRotation().Yaw,0));
    if(DisplayStar.IsValid())DisplayStar->SetActorScale3D(FVector(.45f));
}

float UReferenceTravelSubsystem::GetCelebrationFade() const{return GetTransitionOpacity();}
void UReferenceTravelSubsystem::Tick(float DeltaSeconds)
{
    if(!GetWorld()||!GetWorld()->IsGameWorld())return;
    UpdateFade(DeltaSeconds);if(!IsCelebrating())return;
    auto* Hero=CapturedHero.Get();
    if(!Hero){BeginReturn();return;}
    CelebrationTime+=DeltaSeconds;
    if(!bPoseStarted)
    {
        // 足元へは通常の重力で着地します。空中の100枚スター等では長く落とさず、その場で演出します。
        if(Hero->GetCharacterMovement()->IsMovingOnGround()||CelebrationTime>.9f)StartStarPose();
        return;
    }
    PoseTime+=DeltaSeconds;
    if(DisplayStar.IsValid())DisplayStar->SetActorRotation(FRotator(0,Hero->GetActorRotation().Yaw+FMath::Sin(PoseTime*3)*10,0));
    if(PoseTime>2.4f&&!IsFading())BeginFadeTo(FName(TEXT("/Game/Battlefield/Entrance")));
}
