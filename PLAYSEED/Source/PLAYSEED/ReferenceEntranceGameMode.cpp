#include "ReferenceEntranceGameMode.h"
#include "ReferenceHero.h"
#include "ReferenceTravelSubsystem.h"
#include "ThunderExpeditionSubsystem.h"
#include "BattlefieldUI.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Canvas.h"
#include "Engine/GameInstance.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

AReferenceEntranceRoom::AReferenceEntranceRoom()
{
    RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("EntranceRoot"));
    Painting=CreateDefaultSubobject<USceneComponent>(TEXT("PaintingSurface"));Painting->SetupAttachment(RootComponent);
    Painting->SetRelativeLocation(FVector(400,0,300));
}
void AReferenceEntranceRoom::Shape(UStaticMesh* Asset,FVector Position,FVector Size,FLinearColor Color,bool bSolid,USceneComponent* Parent)
{
    auto* Part=NewObject<UStaticMeshComponent>(this);Part->SetupAttachment(Parent?Parent:RootComponent);Part->SetStaticMesh(Asset);
    Part->SetRelativeLocation(Position);Part->SetRelativeScale3D(Size/100);
    Part->SetCollisionEnabled(bSolid?ECollisionEnabled::QueryAndPhysics:ECollisionEnabled::NoCollision);Part->SetCollisionResponseToAllChannels(ECR_Block);Part->SetCastShadow(false);
    if(auto* Palette=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Battlefield/M_Actor.M_Actor")))
    {auto* Tint=UMaterialInstanceDynamic::Create(Palette,this);Tint->SetVectorParameterValue(TEXT("Tint"),Color);Part->SetMaterial(0,Tint);}
    Part->RegisterComponent();
}
void AReferenceEntranceRoom::BeginPlay()
{
    Super::BeginPlay();
    auto* Cube=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));
    auto* Sphere=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    const FLinearColor Cream(.9f,.85f,.66f),Gold(1,.65f,.06f);
    Shape(Cube,FVector(-600,0,-60),FVector(2300,2000,120),Cream,true);
    for(int32 X=0;X<10;++X)for(int32 Y=0;Y<10;++Y)if((X+Y)%2==0)
        Shape(Cube,FVector(-1500+X*200,-900+Y*200,.4f),FVector(199,199,1),FLinearColor(.18f,.18f,.18f));
    Shape(Cube,FVector(-600,0,2),FVector(1800,300,3),FLinearColor(.65f,.035f,.025f));
    Shape(Cube,FVector(500,0,420),FVector(100,2000,840),Cream,true);
    Shape(Cube,FVector(-600,0,890),FVector(2300,2000,100),Cream,true);
    for(float Side:{-1.f,1.f})Shape(Cube,FVector(-600,Side*1050,420),FVector(2300,100,840),Cream,true);
    Shape(Cube,FVector(10,0,0),FVector(8,420,420),FLinearColor(.13f,.46f,.9f),false,Painting);
    Shape(Cube,FVector(3,0,-120),FVector(8,420,180),FLinearColor(.1f,.57f,.035f),false,Painting);
    // 黒いボム兵の絵を平たい球・足・目で描きます。奥行きを薄くして額の中へ収めます。
    for(int32 I=0;I<2;++I)
    {
        const float Y=I==0?-90:85,Z=I==0?5:-45,Scale=I==0?1.f:.65f;
        Shape(Sphere,FVector(-6,Y,Z),FVector(9,155*Scale,175*Scale),FLinearColor(.015f,.018f,.025f),false,Painting);
        for(float Side:{-1.f,1.f})
        {
            Shape(Sphere,FVector(-12,Y+Side*23*Scale,Z+20*Scale),FVector(4,15*Scale,38*Scale),FLinearColor::White,false,Painting);
            Shape(Sphere,FVector(-9,Y+Side*41*Scale,Z-80*Scale),FVector(9,63*Scale,34*Scale),FLinearColor(.9f,.55f,.02f),false,Painting);
        }
        Shape(Cube,FVector(-7,Y,Z+103*Scale),FVector(8,15*Scale,40*Scale),FLinearColor(.8f,.73f,.5f),false,Painting);
    }
    for(float Side:{-1.f,1.f})
    {
        Shape(Cube,FVector(385,Side*226,300),FVector(35,32,484),Gold);
        Shape(Cube,FVector(385,0,300+Side*226),FVector(35,484,32),Gold);
    }
    auto* Post=GetWorld()->SpawnActor<APostProcessVolume>();Post->bUnbound=true;
    Post->Settings.bOverride_AutoExposureMethod=true;Post->Settings.AutoExposureMethod=AEM_Manual;
    Post->Settings.bOverride_AutoExposureBias=true;Post->Settings.AutoExposureBias=0;
    Post->Settings.bOverride_ToneCurveAmount=true;Post->Settings.ToneCurveAmount=0;
}
void AReferenceEntranceRoom::SetRipple(float Time)
{
    const float Wave=FMath::Sin(Time*24)*.035f;Painting->SetRelativeScale3D(FVector(1,1+Wave,1-Wave));
}
AReferenceEntranceGameMode::AReferenceEntranceGameMode(){DefaultPawnClass=AReferenceHero::StaticClass();HUDClass=AReferenceEntranceHUD::StaticClass();}
void AReferenceEntranceGameMode::StartPlay()
{
    Room=GetWorld()->SpawnActor<AReferenceEntranceRoom>();Super::StartPlay();
    GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>()->ArriveAtPainting();
    if(auto* Hero=Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this,0)))
    {
        Hero->SetTrailStartLocation(FVector(-600,0,84));Hero->ReturnToTrail();Hero->RecoveryCount=0;Hero->SetActorRotation(FRotator::ZeroRotator);
        Hero->GetCameraBoom()->TargetArmLength=700;
        if(auto* PC=Cast<APlayerController>(Hero->GetController()))PC->SetControlRotation(FRotator(-10,0,0));
    }
}
void AReferenceEntranceGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);auto* Hero=Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this,0));if(!Hero)return;
    auto* PC=Cast<APlayerController>(Hero->GetController());auto* Flow=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>();
    if(!bEntering)
    {
        const FVector Position=Hero->GetActorLocation();
        if(Position.X>285 && Position.X<440 && FMath::Abs(Position.Y)<175 && Position.Z>155 && Position.Z<490 && Hero->GetCharacterMovement()->IsFalling() && Flow->BeginPainting())
        {
            bEntering=true;DiveStart=Position;Hero->StopJumping();Hero->GetCharacterMovement()->StopMovementImmediately();Hero->GetCharacterMovement()->DisableMovement();
            // 入場開始を一度だけ確定してから入力を止めます。連打で複数のマップを開きません。
            if(PC)Hero->DisableInput(PC);
        }
        return;
    }
    EntryTime+=DeltaSeconds;Room->SetRipple(EntryTime);
    const float Alpha=FMath::Clamp(EntryTime/.7f,0.f,1.f);
    // これは製品の入場演出です。通常探索や検証の移動を座標で代用する処理ではありません。
    Hero->SetActorLocation(FMath::Lerp(DiveStart,FVector(430,0,300),Alpha));Hero->SetActorRotation(FRotator(-60*Alpha,0,0));
    if(EntryTime>1.1f && Flow->FinishPainting())UGameplayStatics::OpenLevel(this,FName(TEXT("/Game/ThunderMeadow/MissionSelect")));
}
void AReferenceEntranceGameMode::RestartStage(AWindseedCharacter* Player){if(!bEntering)Super::RestartStage(Player);}
void AReferenceEntranceGameMode::HandlePlayerDefeat(AWindseedCharacter* Player){if(!bEntering)Super::HandlePlayerDefeat(Player);}
float AReferenceEntranceGameMode::GetEntryFade() const{return bEntering?FMath::Clamp((EntryTime-.55f)/.5f,0.f,1.f):0;}
void AReferenceEntranceHUD::DrawHUD()
{
    Super::DrawHUD();if(!Canvas)return;FBattlefieldUI UI(Canvas);
    auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    UI.Star(FVector2D(265,20),10,FLinearColor(1,.78f,.02f));UI.Text(TEXT("×"),276,9,19,FLinearColor(.55f,.53f,.85f));UI.Text(FString::FromInt(Progress->GetEarnedRewardCount()),293,8,20,FLinearColor(1,.37f,.025f));
    UI.Text(TEXT("えに むかって ジャンプ！"),160,214,9,FLinearColor::White,true,false,true);
    if(auto* Mode=Cast<AReferenceEntranceGameMode>(GetWorld()->GetAuthGameMode()))if(Mode->GetEntryFade()>0)UI.Rect(0,0,320,240,FLinearColor(1,1,1,Mode->GetEntryFade()));
}
