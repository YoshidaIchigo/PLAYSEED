#include "ReferencePaintingTestSubsystem.h"
#include "ReferenceHundredWalk.h"
#include "ReferenceCoinField.h"
#include "ThunderMeadowStage.h"
#include "ThunderMeadowGameMode.h"
#include "ThunderCoinLedger.h"
#include "ThunderHundredLantern.h"
#include "ThunderSkyChimes.h"
#include "ThunderExpeditionSubsystem.h"
#include "ReferenceTravelSubsystem.h"
#include "PlayseedCoin.h"
#include "PlayseedLantern.h"
#include "PlayseedFlightComponent.h"
#include "PlayseedLaunchComponent.h"
#include "WindseedCharacter.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void UReferencePaintingTestSubsystem::UpdateReferenceHundred(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    auto* Stage=Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(GetWorld(),AThunderMeadowStage::StaticClass()));
    auto* Field=Stage?Stage->GetReferenceCoinField():nullptr;auto* Bonus=Stage?Stage->GetHundredLantern():nullptr;
    auto* Mode=Cast<AThunderMeadowGameMode>(GetWorld()->GetAuthGameMode());auto* Ledger=Mode?Mode->GetCoinLedger():nullptr;
    if(!Hero||Hero->RecoveryCount!=0||!Field||!Bonus||!Ledger||Field->GetCoins().Num()!=23)
    {Finish(false,TEXT("Hundred ordinary route lost the actual hero, field or bonus mechanism"));return;}
    auto* Move=Hero->GetCharacterMovement();
    if(JumpHold>0){JumpHold-=DeltaSeconds;if(JumpHold<=0)PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Released,0));}
    const auto* Points=Phase==120?ReferenceHundredReturn:Phase==121?ReferenceHundredMountain:Phase==122?ReferenceHundredHighGold:Phase==123?ReferenceHundredIslandCap:ReferenceHundredIslandCannon;
    const int32 Count=Phase==120?UE_ARRAY_COUNT(ReferenceHundredReturn):Phase==121?UE_ARRAY_COUNT(ReferenceHundredMountain):Phase==122?UE_ARRAY_COUNT(ReferenceHundredHighGold):Phase==123?UE_ARRAY_COUNT(ReferenceHundredIslandCap):UE_ARRAY_COUNT(ReferenceHundredIslandCannon);
    if(HundredWaypoint>=Count)
    {
        if(!Move->IsMovingOnGround())return;Move->StopMovementImmediately();
        if(Phase==120){Phase=70;RedWaypoint=0;RedLegElapsed=0;}
        else if(Phase==121||Phase==122)
        {
            const int32 First=Phase==121?13:18;
            for(int32 I=First;I<First+5;++I)if(!Field->GetCoins()[I]->IsCollected()){Finish(false,TEXT("Mountain detour did not physically collect all five real golds"));return;}
            if(Phase==121){bHundredMountainGold=true;Phase=62;CannonLegElapsed=0;}
            else{bHundredHighGold=true;Phase=63;}
        }
        else if(Phase==123)
        {
            for(APlayseedCoin* Coin:Field->GetCoins())if(!Coin->IsCollected()){Finish(false,TEXT("A ground gold was still uncollected before the island wing flight"));return;}
            if(!bGroundCoinsVerified||!bHundredMountainGold||!bHundredHighGold||Ledger->GetTotal()!=62||Bonus->HasReachedHundred())
            {Finish(false,FString::Printf(TEXT("Actual ground total before wings was not62: %d"),Ledger->GetTotal()));return;}
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderHundredGroundTotal.png"),false,false);Phase=94;
        }
        else Phase=96;
        HundredWaypoint=0;HundredLegElapsed=0;Elapsed=0;return;
    }
    const auto& Point=Points[HundredWaypoint];FVector Target(Point.X,Point.Y,Point.Z);const FVector Delta=Target-Hero->GetActorLocation();
    HundredLegElapsed+=DeltaSeconds;
    if(HundredLegElapsed>22){Finish(false,FString::Printf(TEXT("Hundred ordinary walk blocked phase%d point%d hero%s target%s grounded%d floor%s"),Phase,HundredWaypoint,*Hero->GetActorLocation().ToString(),*Target.ToString(),Move->IsMovingOnGround(),*Move->CurrentFloor.HitResult.ImpactPoint.ToString()));return;}
    const float HeightTolerance=Move->IsMovingOnGround()?160.f:80.f;
    if(Delta.Size2D()<25&&FMath::Abs(Hero->GetActorLocation().Z-82-Target.Z)<HeightTolerance)
    {
        if(Point.Coin<0||Field->GetCoins()[Point.Coin]->IsCollected())
        {UE_LOG(LogTemp,Display,TEXT("HUNDRED WALK phase%d point%d actual score%d"),Phase,HundredWaypoint,Ledger->GetTotal());++HundredWaypoint;HundredLegElapsed=0;return;}
        Target=Field->GetCoins()[Point.Coin]->GetActorLocation();
    }
    const FVector Wanted=Target-Hero->GetActorLocation();
    if(Wanted.Size2D()>8)
    {
        const FVector Desired=(Wanted.GetSafeNormal2D()*Wanted.Size2D()*4.5f).GetClampedToMaxSize2D(470);
        const FVector Correction=Desired-FVector(Hero->GetVelocity().X,Hero->GetVelocity().Y,0);
        Hero->AddMovementInput(Correction.GetSafeNormal2D(),FMath::Clamp(Correction.Size2D()/150,0.f,1.f));
    }
    else if(Move->IsMovingOnGround())Move->StopMovementImmediately();
    PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(),FRotator(-18,Wanted.Rotation().Yaw,0),DeltaSeconds,4));
    if(Move->IsMovingOnGround()&&Wanted.Size2D()<350&&Target.Z>Hero->GetActorLocation().Z-82+38&&JumpHold<=0)
    {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::SpaceBar,IE_Pressed,1));JumpHold=.14f;}
}

void UReferencePaintingTestSubsystem::UpdateHundredFlight(float DeltaSeconds,AWindseedCharacter* Hero,APlayerController* PC)
{
    auto* Stage=Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(GetWorld(),AThunderMeadowStage::StaticClass()));
    auto* Bonus=Stage?Stage->GetHundredLantern():nullptr;auto* Rings=Stage?Stage->GetSkyChimes():nullptr;
    auto* Mode=Cast<AThunderMeadowGameMode>(GetWorld()->GetAuthGameMode());auto* Ledger=Mode?Mode->GetCoinLedger():nullptr;
    auto* Flow=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>();
    if(!Hero||Hero->RecoveryCount!=0||!Bonus||!Rings||!Ledger||Rings->GetCoins().Num()!=5||Rings->GetOuterCoins().Num()!=40)
    {Finish(false,TEXT("Actual hundred flight lost its hero, five rings or bonus"));return;}
    auto* Flight=Hero->GetFlight();auto* Launch=Hero->GetLaunch();
    auto Release=[&]()
    {PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::W,IE_Released,0));PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::S,IE_Released,0));};
    if(Flow->IsCelebrating())
    {
        Release();
        if(!bHundredSkyComplete||!Bonus->HasReachedHundred()||Bonus->GetGenerationCount()!=1||Bonus->GetTriggerTotal()!=100||Ledger->GetTotal()<100||!Bonus->GetReward())
        {Finish(false,TEXT("Star7 celebration did not follow the one actual100 threshold"));return;}
        ObservedStar=Bonus->GetReward();Phase=26;Elapsed=0;return;
    }
    if(Flight->IsFlying())bWingFlightObserved=true;
    if(Elapsed<.2f&&!Flight->IsFlying())return;
    if(!Flight->IsFlying()||!Flight->HasPower()||Launch->GetLaunchCount()!=2||Flight->GetFlightStarts()!=1)
    {Finish(false,FString::Printf(TEXT("Actual hundred flight ended before star7: point%d score%d power%.2f hero%s"),HundredFlightPoint,Ledger->GetTotal(),Flight->GetRemaining(),*Hero->GetActorLocation().ToString()));return;}
    FVector Target;
    if(Phase==98)
    {
        if(Bonus->HasReachedHundred())
        {
            int32 SkyCoins=0;for(APlayseedCoin* Coin:Rings->GetCoins())if(Coin->IsCollected())++SkyCoins;
            for(APlayseedCoin* Coin:Rings->GetOuterCoins())if(Coin->IsCollected())++SkyCoins;
            if(Ledger->GetTotal()<100||SkyCoins<38||Bonus->GetGenerationCount()!=1||Bonus->GetTriggerTotal()!=100||!Bonus->GetReward())
            {Finish(false,TEXT("Actual100 did not follow62 ground points and at least38 real sky coins"));return;}
            bHundredSkyComplete=true;Phase=99;Elapsed=0;
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Validation/ThunderHundredThreshold.png"),false,false);
            UE_LOG(LogTemp,Display,TEXT("HUNDRED QA: actual threshold100 from coin%s, sky%d, score%d, power%.2f"),*Bonus->GetTriggerCoinId().ToString(),SkyCoins,Ledger->GetTotal(),Flight->GetRemaining());
        }
        else
        {
            while(HundredFlightPoint<45)
            {
                const int32 Group=HundredFlightPoint/9,Position=HundredFlightPoint%9;
                APlayseedCoin* Coin=Position==0?Rings->GetCoins()[Group].Get():Rings->GetOuterCoins()[Group*8+Position-1].Get();
                if(!Coin->IsCollected()){Target=Coin->GetActorLocation();break;}
                UE_LOG(LogTemp,Display,TEXT("HUNDRED SKY: actual point%d score%d power%.2f"),HundredFlightPoint,Ledger->GetTotal(),Flight->GetRemaining());++HundredFlightPoint;Elapsed=0;
            }
            if(HundredFlightPoint>=45){Finish(false,TEXT("All45 sky coins passed without a real100 threshold"));return;}
        }
    }
    if(Phase==99)Target=Bonus->GetReward()->GetRestLocation();
    const FVector Delta=Target-Hero->GetActorLocation();
    // 実速度を読んで曲がりすぎを抑えます。向きは普通のマウス軸だけへ渡し、速度や位置を書き換えません。
    const FVector Aim=Delta.Size()<600?Delta-Hero->GetVelocity()*.25f:Delta;
    const FRotator Desired=Aim.Rotation();const float Yaw=FRotator::NormalizeAxis(Desired.Yaw-Flight->GetFlightYaw()),Pitch=FMath::Clamp(Desired.Pitch,-60.f,70.f)-Flight->GetFlightPitch();
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseX,IE_Axis,FMath::Clamp(Yaw*.6f,-40.f,40.f)));
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseY,IE_Axis,FMath::Clamp(-Pitch*.6f,-40.f,40.f)));
    const bool bSlow=Delta.Size()<800;
    if(!bWingFlightKeysSet||bSlow!=bWingSlow)
    {
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::W,bSlow?IE_Released:IE_Pressed,bSlow?0:1));
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::S,bSlow?IE_Pressed:IE_Released,bSlow?1:0));bWingSlow=bSlow;bWingFlightKeysSet=true;
    }
    if(Elapsed>(Phase==99?22:12))Finish(false,FString::Printf(TEXT("Ordinary mouse flight missed hundred target: phase%d point%d score%d hero%s target%s power%.2f"),Phase,HundredFlightPoint,Ledger->GetTotal(),*Hero->GetActorLocation().ToString(),*Target.ToString(),Flight->GetRemaining()));
}
