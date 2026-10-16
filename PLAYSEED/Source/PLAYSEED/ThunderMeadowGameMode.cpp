#include "ThunderMeadowGameMode.h"
#include "ReferenceBattlefieldTerrain.h"
#include "ReferenceHero.h"
#include "ReferenceTravelSubsystem.h"
#include "BattlefieldUI.h"
#include "ThunderCoinLedger.h"
#include "ThunderLivesComponent.h"
#include "ThunderMeadowStage.h"
#include "ThunderCloudflagRace.h"
#include "ThunderExpeditionSubsystem.h"
#include "WindseedCharacter.h"
#include "PlayseedVitalityComponent.h"
#include "PlayseedInteractionComponent.h"
#include "PlayseedLaunchComponent.h"
#include "Components/InputComponent.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void AThunderMeadowHUD::DrawHUD()
{
    Super::DrawHUD();if(!Canvas)return;
    FBattlefieldUI UI(Canvas);
    auto* Flow=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>();
    if(Flow->GetTransitionOpacity()>=1){UI.Rect(0,0,320,240,FLinearColor::White);return;}
    auto* Mode=Cast<AThunderMeadowGameMode>(GetWorld()->GetAuthGameMode());
    auto* Player=Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
    if(!Mode || !Player)return;
    if(AReferenceBattlefieldTerrain::IsReferenceCourse()&&Player->GetLaunch()->IsAiming())
    {
        // 同心円の境界を横帯で描き、HUDの手前ではなく背景だけを丸く切り取ります。
        for(int32 Y=0;Y<240;Y+=2)
        {
            const float Dy=Y+1-120;const float Outer=FMath::Abs(Dy)<104?FMath::Sqrt(104*104-Dy*Dy):0;
            UI.Rect(0,Y,160-Outer,2,FLinearColor::Black);UI.Rect(160+Outer,Y,160-Outer,2,FLinearColor::Black);
            for(int32 Ring=0;Ring<5;++Ring)
            {
                const float R0=94+Ring*2,R1=R0+2;
                const float A=FMath::Abs(Dy)<R0?FMath::Sqrt(R0*R0-Dy*Dy):0,B=FMath::Abs(Dy)<R1?FMath::Sqrt(R1*R1-Dy*Dy):0;
                if(B>A){const FLinearColor Shade(0,0,0,(Ring+1)/5.f);UI.Rect(160-B,Y,B-A,2,Shade);UI.Rect(160+A,Y,B-A,2,Shade);}
            }
        }
    }
    auto* Progress=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    const FLinearColor Number(1,.37f,.025f),Multiply(.55f,.53f,.85f);
    UI.Face(FVector2D(29,20));UI.Text(TEXT("×"),42,9,19,Multiply);UI.Text(FString::FromInt(Mode->GetLives()->GetRemaining()),59,8,20,Number);
    UI.Coin(FVector2D(181,20));UI.Text(TEXT("×"),191,9,19,Multiply);UI.Text(FString::FromInt(Mode->GetCoinLedger()->GetTotal()),208,8,20,Number);
    UI.Star(FVector2D(265,20),10,FLinearColor(1,.78f,.025f));UI.Text(TEXT("×"),276,9,19,Multiply);UI.Text(FString::FromInt(Progress->GetEarnedRewardCount()),293,8,20,Number);
    const int32 Health=Player->GetVitality()->GetHealth();
    if(Health!=LastHealth){LastHealth=Health;PowerUntil=GetWorld()->GetTimeSeconds()+2;}
    if(Health<8 || GetWorld()->GetTimeSeconds()<PowerUntil)UI.Power(FVector2D(160,61),Health);
    UI.Camera();
    if(Flow->IsCelebrating())
    {
        // 参照と同じくHUDを残し、操作できないカメラを赤い×で示します。
        const FLinearColor Red(1,.04f,.02f);
        UI.Triangle(FVector2D(298,215),FVector2D(300,215),FVector2D(308,224),Red);
        UI.Triangle(FVector2D(298,215),FVector2D(308,224),FVector2D(306,224),Red);
        UI.Triangle(FVector2D(306,215),FVector2D(308,215),FVector2D(300,224),Red);
        UI.Triangle(FVector2D(306,215),FVector2D(300,224),FVector2D(298,224),Red);
        if(Flow->GetCelebrationFade()>0)UI.Rect(0,0,320,240,FLinearColor(1,1,1,Flow->GetCelebrationFade()));
        return;
    }
    if(Player->GetLaunch()->IsAiming())
    {
        if(AReferenceBattlefieldTerrain::IsReferenceCourse())
        {
            const FLinearColor Sight(.2f,.2f,.3f,.9f);
            UI.Triangle(FVector2D(154,100),FVector2D(166,100),FVector2D(160,108),Sight);
            UI.Triangle(FVector2D(154,140),FVector2D(166,140),FVector2D(160,132),Sight);
            UI.Triangle(FVector2D(140,114),FVector2D(140,126),FVector2D(148,120),Sight);
            UI.Triangle(FVector2D(180,114),FVector2D(180,126),FVector2D(172,120),Sight);
        }
        else
        {
            UI.Rect(153,119,14,1,FLinearColor::Black);UI.Rect(159,113,1,14,FLinearColor::Black);
            UI.Rect(155,120,10,1,FLinearColor::White);UI.Rect(160,115,1,10,FLinearColor::White);
        }
    }
    if(AReferenceBattlefieldTerrain::IsReferenceCourse())
    {
        auto* Stage=Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(GetWorld(),AThunderMeadowStage::StaticClass()));
        auto* Race=Stage?Stage->GetCloudflagRace():nullptr;
        if(Race&&Race->GetRaceState()!=ECloudflagRaceState::Idle)
        {
            // 自分が旗に着いた時刻で表示を止め、相手を待つ時間を記録に加えません。
            const float ClockTime=Race->HasPlayerReachedFlag()?Race->GetPlayerTime():Race->GetElapsed();
            const int32 Tenths=FMath::FloorToInt(ClockTime*10);
            const FLinearColor TimeColors[]={FLinearColor(1,.1f,.7f),FLinearColor(.65f,1,.1f),FLinearColor(.1f,1,.25f),FLinearColor(1,.8f,.05f)};
            for(int32 Index=0;Index<4;++Index)UI.Text(FString::Chr(TEXT("TIME")[Index]),174+Index*12,35,14,TimeColors[Index],false,true);
            const FString TimeText=FString::FromInt(Tenths/600)+FString::Chr(39)+FString::Printf(TEXT("%02d"),(Tenths/10)%60)+FString::Chr(34)+FString::FromInt(Tenths%10);
            UI.Text(TimeText,225,33,16,Number,false,true);
            if(Race->GetRaceState()!=ECloudflagRaceState::Running)
                UI.Text(Race->GetStateLabel(),160,161,7,FLinearColor::White,true,true,true);
        }
    }
    if(!Player->GetInteraction()->GetMessage().IsEmpty())
    {
        UI.Rect(44,77,232,64,FLinearColor(0,0,0,.75f));
        if(AReferenceBattlefieldTerrain::IsReferenceCourse())
        {
            // 日本語は空白で単語を分けないため、改行と23文字幅で読みやすく折り返します。
            FString Line;float Y=84;
            for(TCHAR Character:Player->GetInteraction()->GetMessage())
            {
                if(Character==TEXT('\n')||Line.Len()>=23)
                {UI.Text(Line,52,Y,8,FLinearColor::White,false,false,true);Line.Reset();Y+=12;}
                if(Character!=TEXT('\n'))Line.AppendChar(Character);
            }
            UI.Text(Line,52,Y,8,FLinearColor::White,false,false,true);
        }
        else
        {
        TArray<FString> Words;Player->GetInteraction()->GetMessage().ParseIntoArray(Words,TEXT(" "),true);
        FString Line;float Y=84;
        for(const FString& Word:Words){if(Line.Len()+Word.Len()>38){UI.Text(Line,52,Y,8,FLinearColor::White,false,false);Line.Reset();Y+=12;}Line+=Word+TEXT(" ");}
        UI.Text(Line,52,Y,8,FLinearColor::White,false,false);
        }
    }
    if(Mode->ShouldShowControls())
    {
        UI.Rect(30,82,260,114,FLinearColor(0,0,0,.85f));
        const TCHAR* BackHint=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>()->UsesEntrance()?TEXT("R/B: やりなおし   M: えにもどる"):TEXT("R/B: やりなおし   M: スターえらび");
        const TCHAR* Lines[]={TEXT("そうさ"),TEXT("WASD / 左スティック: いどう"),TEXT("Space / A: ジャンプ   Shift / LB: はしる"),TEXT("マウス / 右スティック: カメラ"),TEXT("E/X: もつ   F/Y: のぼる   G/下: はなす"),TEXT("Ctrl/RB: ヒップドロップ  LMB/RT: こうげき"),BackHint,TEXT("はね: 空中Space とぶ/おりる  W/S そくど"),TEXT("H: このひょうじを とじる")};
        for(int32 I=0;I<UE_ARRAY_COUNT(Lines);++I)UI.Text(Lines[I],42,89+I*12,7.5f,FLinearColor::White,false,false,true);
    }
    // 実保存失敗の案内は日本語です。Latin書体の代わりに日本語対応書体を明示します。
    if(!Progress->GetProgressProblem().IsEmpty()){UI.Rect(20,180,280,28,FLinearColor(0,0,0,.8f));UI.Text(Progress->GetProgressProblem(),25,187,7,FLinearColor(1,.35f,.25f),false,true,true);}
    if(Mode->GetLives()->IsGameOver())
    {
        UI.Rect(0,0,320,240,FLinearColor(0,0,0,.85f));UI.Text(TEXT("GAME OVER"),160,91,29,FLinearColor(1,.12f,.025f),true);
        UI.Text(TEXT("スターは そのまま。もういちど ちょうせん！"),160,145,8,FLinearColor::White,true,false,true);
    }
    if(Flow->GetTransitionOpacity()>0)UI.Rect(0,0,320,240,FLinearColor(1,1,1,Flow->GetTransitionOpacity()));
}

AThunderMeadowGameMode::AThunderMeadowGameMode()
{
    Lives = CreateDefaultSubobject<UThunderLivesComponent>(TEXT("VisitLives"));
    CoinLedger = CreateDefaultSubobject<UThunderCoinLedger>(TEXT("VisitCoinLedger"));
    DefaultPawnClass = AReferenceHero::StaticClass();
    HUDClass = AThunderMeadowHUD::StaticClass();
}

void AThunderMeadowGameMode::StartPlay()
{
    // 地形 Actor を先に登録してから World の BeginPlay を開始します。
    // 各 Actor の準備が終わってから最初の Tick が進みます。
    Stage = GetWorld()->SpawnActor<AThunderMeadowStage>();
    Super::StartPlay();
    if(InputComponent)InputComponent->BindKey(EKeys::H,IE_Pressed,this,&AThunderMeadowGameMode::ToggleControls);
    if (auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
    {
        Player->SetTrailStartLocation(AReferenceBattlefieldTerrain::IsReferenceCourse() ? FVector(-6558,-6464,100) : FVector(-600,-600,100));
        Player->ReturnToTrail();
        Player->RecoveryCount = 0;
        if(GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>()->GetState()==EReferenceTravelState::Dropping)
        {
            // 製品の入場位置だけを上空へ置きます。その後の落下・床との接触はCharacterMovementの重力です。
            Player->SetActorLocation(Player->GetActorLocation()+FVector(0,0,1000));
            Player->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
            if(auto* PC=Cast<APlayerController>(Player->GetController()))Player->DisableInput(PC);
            bDropActive=true;GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>()->HoldIncomingInput();
        }
    }
}

void AThunderMeadowGameMode::RestartStage(AWindseedCharacter* Player)
{
    if(GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>()->IsTransitionLocked())return;
    if (Lives->IsGameOver()) return; // 最後の残機を失った後のRでは再開しません。
    // このステージが生成した仕掛けだけを初期化し、永続の獲得済み報酬には触れません。
    CoinLedger->ResetVisit();
    if (Stage) Stage->ResetMechanisms();
    Super::RestartStage(Player);
}

void AThunderMeadowGameMode::HandlePlayerDefeat(AWindseedCharacter* Player)
{
    if (!Player || Lives->IsGameOver()) return;
    auto* Flow=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>();
    if(bDropActive)
    {
        if(Flow->BeginReturn())Lives->ConsumeLife();
        return;
    }
    if (Lives->ConsumeLife()) RestartStage(Player);
    else
    {
        // 0の間は入力と物理移動を止め、残機Componentの3秒後の遷移を待ちます。
        Player->StopJumping(); Player->GetCharacterMovement()->StopMovementImmediately();
        Player->GetCharacterMovement()->ClearAccumulatedForces(); Player->GetCharacterMovement()->DisableMovement();
        if(auto* PC=Cast<APlayerController>(Player->GetController())) Player->DisableInput(PC);
    }
}

bool AThunderMeadowGameMode::CanLeaveStage() const
{
    return !GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>()->IsTransitionLocked();
}
void AThunderMeadowGameMode::ToggleControls(){if(CanLeaveStage())bShowControls=!bShowControls;}
float AThunderMeadowGameMode::GetDropFade() const{return GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>()->GetTransitionOpacity();}
void AThunderMeadowGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);if(!bDropActive)return;DropTime+=DeltaSeconds;
    auto* Hero=Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this,0));if(!Hero)return;
    auto* Flow=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>();
    if(Hero->GetCharacterMovement()->IsMovingOnGround())
    {
        if(DropLandedTime<0)DropLandedTime=DropTime;
        if(auto* Model=Cast<AReferenceHero>(Hero))Model->SetLandingPose(true);
        if(DropTime-DropLandedTime>.28f&&Flow->FinishDrop())
        {
            if(auto* Model=Cast<AReferenceHero>(Hero))Model->SetLandingPose(false);
            if(auto* PC=Cast<APlayerController>(Hero->GetController()))Hero->EnableInput(PC);
            bDropActive=false;
        }
    }
    // 何らかの障害で着地しない場合も、入力を止めたまま残さず入口へ戻します。
    if(DropTime>8)Flow->BeginReturn();
}

void AThunderMeadowGameMode::HandlePlayerFall(AWindseedCharacter* Player)
{
    // 雷鳴の丘では場外落下も死亡として扱います。保存済み報酬は再開で消しません。
    if (Player && !Lives->IsGameOver()) Player->GetVitality()->Defeat();
}
