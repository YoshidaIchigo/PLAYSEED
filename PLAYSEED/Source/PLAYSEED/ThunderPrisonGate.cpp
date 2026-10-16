#include "ThunderPrisonGate.h"
#include "ThunderBoundSentinel.h"
#include "PlayseedLantern.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
AThunderPrisonGate::AThunderPrisonGate()
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Bars = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CopperGateCollision")); RootComponent = Bars;
    Bars->SetStaticMesh(Cube.Object); Bars->SetRelativeScale3D(FVector(.35f, 3.8f, 4));
    Bars->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); Bars->SetCollisionObjectType(ECC_WorldStatic); Bars->SetCollisionResponseToAllChannels(ECR_Block);
    // 見通せる格子は描画用。薄い衝突面で扉を越えられないようにします。
    Bars->SetVisibility(false);
    for (int32 I = 0; I < 7; ++I)
    {
        auto* Rail = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("CopperRail%d"), I));
        Rail->SetupAttachment(Bars); Rail->SetStaticMesh(Cube.Object); Rail->SetAbsolute(false, false, true);
        Rail->SetRelativeLocation(FVector(0, (I - 3) * 54 / 3.8f, 0)); Rail->SetRelativeScale3D(FVector(.35f, .14f, 4)); Rail->SetCollisionEnabled(ECollisionEnabled::NoCollision); Rails.Add(Rail);
    }
}
void AThunderPrisonGate::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, bReferenceReward?TEXT("/Game/Battlefield/M_Actor.M_Actor"):TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        auto* Tint = UMaterialInstanceDynamic::Create(Palette, this); Tint->SetVectorParameterValue(TEXT("Tint"), bReferenceReward?FLinearColor(.3f,.32f,.35f):FLinearColor(.55f, .3f, .13f));
        for (UStaticMeshComponent* Rail : Rails) Rail->SetMaterial(0, Tint);
    }
}
void AThunderPrisonGate::Configure(AThunderBoundSentinel* InSentinel) { Sentinel = InSentinel; ResetGate(); }
void AThunderPrisonGate::HandleReleasedImpact(AThunderBoundSentinel* Caller, const FHitResult& Hit)
{
    if (bBroken || !Sentinel.IsValid() || Caller != Sentinel.Get() || !Caller->IsReleased() || Hit.GetComponent() != Bars) return;
    bBroken = true; ++BreakCount; Bars->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    for (int32 I = 0; I < Rails.Num(); ++I) { Rails[I]->SetRelativeRotation(FRotator(0, I % 2 ? 25 : -25, 70)); Rails[I]->SetRelativeLocation(FVector(0, (I - 3) * 54 / 3.8f, -45)); }
    // World所有の一個の報酬。門を壊した事実だけでは保存せず、実際に触れて取得します。
    if(!bReferenceReward)
    {Reward = GetWorld()->SpawnActor<APlayseedLantern>(GetRewardLocation(), FRotator::ZeroRotator);if(Reward.IsValid())Reward->ConfigureReward(6);}
    if(bReferenceReward)for(UStaticMeshComponent* Rail:Rails)Rail->SetVisibility(false);
    UE_LOG(LogTemp, Display, TEXT("PRISON GATE: actual released sentinel sweep broke the bars once."));
}
void AThunderPrisonGate::ResetGate()
{
    bBroken = false; if (Reward.IsValid()) Reward->Destroy(); Reward.Reset(); Bars->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    for(UStaticMeshComponent* Rail:Rails)Rail->SetVisibility(true);
    // 参照のスターは最初から格子の奥に見えます。壊した後に別の位置へ出すことはしません。
    if(bReferenceReward)
    {Reward=GetWorld()->SpawnActor<APlayseedLantern>(GetRewardLocation(),FRotator::ZeroRotator);if(Reward.IsValid())Reward->ConfigureReward(6);}
    for (int32 I = 0; I < Rails.Num(); ++I) { Rails[I]->SetRelativeRotation(FRotator::ZeroRotator); Rails[I]->SetRelativeLocation(FVector(0, (I - 3) * 54 / 3.8f, 0)); }
}
