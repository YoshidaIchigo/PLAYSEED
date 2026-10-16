#include "ThunderMissionSelectionGameMode.h"
#include "ThunderExpeditionSubsystem.h"
#include "ReferenceTravelSubsystem.h"
#include "BattlefieldUI.h"
#include "Components/InputComponent.h"
#include "Engine/Canvas.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
void AThunderMissionSelectionHUD::DrawHUD()
{
    Super::DrawHUD();if(!Canvas)return;
    DrawRect(FLinearColor::Black,0,0,Canvas->SizeX,Canvas->SizeY);
    FBattlefieldUI UI(Canvas);UI.Rect(0,0,320,240,FLinearColor(.78f,.8f,.82f));
    // 参照画面の明るい背景と横並びの6スターです。候補の表示だけでは獲得にしません。
    for(int32 Y=0;Y<24;++Y)for(int32 X=0;X<32;++X)if((X+Y)%2==0)UI.Triangle(FVector2D(X*10,Y*10),FVector2D(X*10+10,Y*10),FVector2D(X*10,Y*10+10),FLinearColor(.82f,.84f,.86f));
    auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    const auto* Mode=Cast<AThunderMissionSelectionGameMode>(GetWorld()->GetAuthGameMode());const int32 Selected=Mode?Mode->GetHighlightedMission():1;
    UI.Text(TEXT("COURSE 1"),160,25,12,FLinearColor(.95f,.68f,.015f),true);
    UI.Text(TEXT("ボムへいの せんじょう"),160,51,14,FLinearColor::Black,true,false,true);
    for(int32 N=1;N<=6;++N)
    {
        const float X=35+(N-1)*50;const bool bSelected=N==Selected,bEarned=Progress->HasEarnedReward(N);
        UI.Text(FString::FromInt(N),X,87,10,FLinearColor::White,true);
        UI.Star(FVector2D(X,119),bSelected?22:14,bEarned?FLinearColor(1,.78f,.02f):FLinearColor(.15f,.25f,.8f));
        if(bSelected)UI.Triangle(FVector2D(X,148),FVector2D(X-5,155),FVector2D(X+5,155),FLinearColor(.85f,.05f,.025f));
    }
    UI.Text(UThunderExpeditionSubsystem::GetMissionTitle(Selected),160,169,10,FLinearColor::Black,true,false,true);
    UI.Text(TEXT("← →  スターをえらぶ   Enter / A  けってい"),160,205,6.5f,FLinearColor(.18f,.18f,.2f),true,false,true);
    if(Progress->HasEarnedReward(7))UI.Star(FVector2D(300,217),7,FLinearColor(1,.78f,.02f));
    if(!Progress->GetProgressProblem().IsEmpty())UI.Text(Progress->GetProgressProblem(),160,226,6,FLinearColor(.8f,.05f,.025f),true,true,true);
    const float Fade=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>()->GetTransitionOpacity();
    if(Fade>0)UI.Rect(0,0,320,240,FLinearColor(1,1,1,Fade));
}
AThunderMissionSelectionGameMode::AThunderMissionSelectionGameMode(){DefaultPawnClass=nullptr;HUDClass=AThunderMissionSelectionHUD::StaticClass();}
void AThunderMissionSelectionGameMode::StartPlay()
{
    Super::StartPlay();GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>()->HoldIncomingInput();auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    HighlightedMission=1;while(HighlightedMission<6 && Progress->HasEarnedReward(HighlightedMission))++HighlightedMission;
    if(!InputComponent)return;
    InputComponent->BindKey(EKeys::One,IE_Pressed,this,&AThunderMissionSelectionGameMode::ChooseOne);
    InputComponent->BindKey(EKeys::Two,IE_Pressed,this,&AThunderMissionSelectionGameMode::ChooseTwo);
    InputComponent->BindKey(EKeys::Three,IE_Pressed,this,&AThunderMissionSelectionGameMode::ChooseThree);
    InputComponent->BindKey(EKeys::Four,IE_Pressed,this,&AThunderMissionSelectionGameMode::ChooseFour);
    InputComponent->BindKey(EKeys::Five,IE_Pressed,this,&AThunderMissionSelectionGameMode::ChooseFive);
    InputComponent->BindKey(EKeys::Six,IE_Pressed,this,&AThunderMissionSelectionGameMode::ChooseSix);
    for(FKey Key:{EKeys::Left,EKeys::A,EKeys::Gamepad_DPad_Left})InputComponent->BindKey(Key,IE_Pressed,this,&AThunderMissionSelectionGameMode::Left);
    for(FKey Key:{EKeys::Right,EKeys::D,EKeys::Gamepad_DPad_Right})InputComponent->BindKey(Key,IE_Pressed,this,&AThunderMissionSelectionGameMode::Right);
    for(FKey Key:{EKeys::Enter,EKeys::SpaceBar,EKeys::Gamepad_FaceButton_Bottom})InputComponent->BindKey(Key,IE_Pressed,this,&AThunderMissionSelectionGameMode::Confirm);
    InputComponent->BindAxisKey(EKeys::Gamepad_LeftX,this,&AThunderMissionSelectionGameMode::Stick);
    if(auto* PC=UGameplayStatics::GetPlayerController(this,0)){PC->SetInputMode(FInputModeGameOnly());PC->bShowMouseCursor=false;}
}
void AThunderMissionSelectionGameMode::Navigate(int32 Direction){if(!bTravelRequested&&!GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>()->IsTransitionLocked())HighlightedMission=(HighlightedMission-1+Direction+6)%6+1;}
void AThunderMissionSelectionGameMode::Stick(float Value)
{
    if(FMath::Abs(Value)<.2f){NextStickTime=0;return;}
    if(FMath::Abs(Value)>.6f && GetWorld()->GetTimeSeconds()>=NextStickTime){Navigate(Value>0?1:-1);NextStickTime=GetWorld()->GetTimeSeconds()+.3f;}
}
void AThunderMissionSelectionGameMode::EnterMission(int32 Number)
{
    if(bTravelRequested||GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>()->IsTransitionLocked())return;
    if(!GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->SelectMission(Number))return;
    if(!GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>()->BeginCourse())return;
    bTravelRequested=true; // マップ切替は完全な白になってから共通フローが一度だけ行います。
}
