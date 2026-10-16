#include "ThunderBindingPost.h"
#include "ThunderBoundSentinel.h"
#include "ThunderPrisonGate.h"
#include "PlayseedActionComponent.h"
#include "WindseedCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
AThunderBindingPost::AThunderBindingPost()
{
    PrimaryActorTick.bCanEverTick = true; RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("BindingRoot"));
    Post = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BindingStump")); Post->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    Post->SetStaticMesh(Cylinder.Object); Post->SetCollisionObjectType(ECC_WorldDynamic); Post->SetCollisionResponseToAllChannels(ECR_Block);
    RefreshShape();
}
void AThunderBindingPost::BeginPlay()
{
    Super::BeginPlay(); if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette"))) { Tint = UMaterialInstanceDynamic::Create(Palette, this); Post->SetMaterial(0, Tint); } RefreshShape();
}
void AThunderBindingPost::Configure(AThunderBoundSentinel* InSentinel, AThunderPrisonGate* InGate) { Sentinel = InSentinel; Gate = InGate; ResetPost(); }
void AThunderBindingPost::RefreshShape()
{
    const float Height = GetTopHeight(); Post->SetRelativeLocation(FVector(0, 0, Height * .5f)); Post->SetRelativeScale3D(FVector(1.6f, 1.6f, FMath::Max(Height, 1.f) / 100));
    Post->SetVisibility(PoundCount < 3); Post->SetCollisionEnabled(PoundCount < 3 ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
    if (Tint) Tint->SetVectorParameterValue(TEXT("Tint"), FLinearColor(.7f - PoundCount * .12f, .42f + PoundCount * .1f, .16f));
}
void AThunderBindingPost::ResetPost() { PoundCount = 0; RefreshShape(); }
void AThunderBindingPost::HandlePound(FVector ImpactPoint)
{
    const FVector Local = GetActorTransform().InverseTransformPosition(ImpactPoint);
    if (PoundCount >= 3 || !Sentinel.IsValid() || !Gate.IsValid() || Local.Size2D() > 80 || FMath::Abs(Local.Z - GetTopHeight()) > 15) return;
    ++PoundCount; RefreshShape();
    if (PoundCount < 3) Sentinel->StaggerFromPost(); else Sentinel->ReleaseToward(Gate.Get());
    UE_LOG(LogTemp, Display, TEXT("BINDING POST: actual top pound %d/3, remaining height %.0f."), PoundCount, GetTopHeight());
}
void AThunderBindingPost::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!Actions.IsValid()) if (auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0))) { Actions = Player->GetActions(); Actions->OnPoundImpact.AddUObject(this, &AThunderBindingPost::HandlePound); }
}
void AThunderBindingPost::EndPlay(const EEndPlayReason::Type Reason) { if (Actions.IsValid()) Actions->OnPoundImpact.RemoveAll(this); Super::EndPlay(Reason); }
