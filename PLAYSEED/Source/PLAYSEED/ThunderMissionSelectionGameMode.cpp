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
namespace
{
void DrawCourseBadge(const FBattlefieldUI& UI)
{
    const FVector2D Center(160,157);
    UI.Ellipse(Center+FVector2D(2,2),FVector2D(32,32),FLinearColor(0,0,0,.2f));
    UI.Ellipse(Center,FVector2D(32,32),FLinearColor(.12f,.055f,.01f));
    UI.Ellipse(Center,FVector2D(30.5,30.5),FLinearColor(.75f,.45f,.08f));
    UI.Ellipse(Center,FVector2D(28.5,28.5),FLinearColor(.3f,.12f,.035f));
    // 元画像を素材へ取り込まず、円と三角形で木目風のCOURSE表示を自作します。
    auto Inside=[Center](FVector2D Point){const FVector2D Delta=Point-Center;return Center+Delta.GetSafeNormal()*FMath::Min(Delta.Size(),27.);};
    for(int32 I=0;I<22;++I)
    {
        const float A=I*2.399963f,D=27*FMath::Sqrt((I+.5f)/22.f),Shade=.12f+.1f*((I*7)%6);
        const FVector2D Point=Center+FVector2D(FMath::Cos(A)*D,FMath::Sin(A)*D);
        UI.Triangle(Inside(Point+FVector2D(-9,-7)),Inside(Point+FVector2D(11,-2)),Inside(Point+FVector2D(-3,10)),FLinearColor(Shade,Shade*.52f,Shade*.17f));
    }
    const FString Course=TEXT("COURSE");
    for(int32 I=0;I<Course.Len();++I)UI.Text(Course.Mid(I,1),137+I*9,126+FMath::Abs(I-2.5f)*4,11,FLinearColor(1,.83f,.05f),true);
    UI.Text(TEXT("1"),160,153,18,FLinearColor(1,.28f,.015f),true);
}
}
void AThunderMissionSelectionHUD::DrawHUD()
{
    Super::DrawHUD();if(!Canvas)return;
    DrawRect(FLinearColor::Black,0,0,Canvas->SizeX,Canvas->SizeY);
    FBattlefieldUI UI(Canvas);UI.Rect(0,0,320,240,FLinearColor::White);
    auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    const auto* Mode=Cast<AThunderMissionSelectionGameMode>(GetWorld()->GetAuthGameMode());const int32 Selected=Mode?Mode->GetHighlightedMission():1;
    const int32 Visible=Mode?Mode->GetVisibleMissionCount():1;
    for(int32 N=1;N<=Visible;++N)
    {
        const float X=160-(Visible-1)*17+(N-1)*34;const bool bSelected=N==Selected,bEarned=Progress->HasEarnedReward(N);
        const float Turn=FMath::Cos(GetWorld()->GetTimeSeconds()*2*PI/1.066667f),Width=bSelected?FMath::Max(.12f,FMath::Abs(Turn)):1;
        UI.Text(FString::FromInt(N),X,36,8,FLinearColor::Black,true,false);
        UI.Star(FVector2D(X,64),bSelected?18:13.8f,bEarned?FLinearColor(1,.8f,.015f):FLinearColor(.2f,.18f,.45f,.85f),bEarned&&(!bSelected||Turn>.15f),Width);
    }
    UI.Text(UThunderExpeditionSubsystem::GetMissionTitle(Selected),158,77,7,FLinearColor::Black,true,false,true);
    DrawCourseBadge(UI);
    UI.Text(TEXT("ボムへいの せんじょう"),160,191,7.5f,FLinearColor::Black,true,false,true);
    UI.Text(TEXT("← → えらぶ   Enter / A けってい"),160,228,6,FLinearColor(.3f,.3f,.3f),true,false,true);
    if(Progress->HasEarnedReward(7))UI.Star(FVector2D(265,127),7,FLinearColor(1,.78f,.02f));
    if(!Progress->GetProgressProblem().IsEmpty())UI.Text(Progress->GetProgressProblem(),160,226,6,FLinearColor(.8f,.05f,.025f),true,true,true);
    const float Fade=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>()->GetTransitionOpacity();
    if(Fade>0)UI.Rect(0,0,320,240,FLinearColor(1,1,1,Fade));
}
AThunderMissionSelectionGameMode::AThunderMissionSelectionGameMode(){DefaultPawnClass=nullptr;HUDClass=AThunderMissionSelectionHUD::StaticClass();}
void AThunderMissionSelectionGameMode::StartPlay()
{
    Super::StartPlay();GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>()->HoldIncomingInput();
    HighlightedMission=GetFirstUnfinishedMission();
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
int32 AThunderMissionSelectionGameMode::GetFirstUnfinishedMission() const
{
    auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    for(int32 N=1;N<=6;++N)if(!Progress->HasEarnedReward(N))return N;
    return 6;
}
int32 AThunderMissionSelectionGameMode::GetVisibleMissionCount() const
{
    auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();int32 Highest=0,Obtained=0;
    for(int32 N=1;N<=6;++N)if(Progress->HasEarnedReward(N)){Highest=N;++Obtained;}
    // 順に取った場合は次の1個を追加。先のスターを取った場合はそこまでの未取得も表示します。
    if(Highest==Obtained&&Highest<6)++Highest;
    return FMath::Max(1,Highest);
}
bool AThunderMissionSelectionGameMode::IsMissionSelectable(int32 Number) const
{
    return Number>=1&&Number<=GetVisibleMissionCount()&&(GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->HasEarnedReward(Number)||Number==GetFirstUnfinishedMission());
}
void AThunderMissionSelectionGameMode::Navigate(int32 Direction)
{
    if(bTravelRequested||GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>()->IsTransitionLocked())return;
    int32 Candidate=HighlightedMission;
    for(int32 I=0;I<6;++I){Candidate=(Candidate-1+Direction+6)%6+1;if(IsMissionSelectable(Candidate)){HighlightedMission=Candidate;return;}}
}
void AThunderMissionSelectionGameMode::Stick(float Value)
{
    if(FMath::Abs(Value)<.2f){NextStickTime=0;return;}
    if(FMath::Abs(Value)>.6f && GetWorld()->GetTimeSeconds()>=NextStickTime){Navigate(Value>0?1:-1);NextStickTime=GetWorld()->GetTimeSeconds()+.3f;}
}
void AThunderMissionSelectionGameMode::EnterMission(int32 Number)
{
    if(bTravelRequested||GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>()->IsTransitionLocked()||!IsMissionSelectable(Number))return;
    if(!GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->SelectMission(Number))return;
    if(!GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>()->BeginCourse())return;
    bTravelRequested=true; // マップ切替は完全な白になってから共通フローが一度だけ行います。
}
