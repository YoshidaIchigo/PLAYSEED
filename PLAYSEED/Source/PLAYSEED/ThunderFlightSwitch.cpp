#include "ThunderFlightSwitch.h"
#include "ReferenceBattlefieldTerrain.h"
#include "ThunderExpeditionSubsystem.h"
#include "PlayseedActionComponent.h"
#include "WindseedCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AThunderFlightSwitch::AThunderFlightSwitch()
{
    PrimaryActorTick.bCanEverTick = true;bReferenceSwitch=AReferenceBattlefieldTerrain::IsReferenceCourse();
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SunSwitchRoot"));
    Button = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SunButton")); Button->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    Button->SetStaticMesh(Mesh.Object); Button->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); Button->SetCollisionResponseToAllChannels(ECR_Block);
}
void AThunderFlightSwitch::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, bReferenceSwitch?TEXT("/Game/Battlefield/M_Actor.M_Actor"):TEXT("/Game/Windseed/M_Palette.M_Palette"))) { ButtonTint = UMaterialInstanceDynamic::Create(Palette, this); Button->SetMaterial(0, ButtonTint); }
    RefreshFromProgress();
}
void AThunderFlightSwitch::RefreshFromProgress()
{
    bActivated = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->IsFlightUnlocked();
    Button->SetRelativeLocation(FVector(0, 0, bActivated ? 10 : 20)); Button->SetRelativeScale3D(FVector(2, 2, bActivated ? .2f : .4f));
    if (ButtonTint) ButtonTint->SetVectorParameterValue(TEXT("Tint"), bReferenceSwitch?(bActivated?FLinearColor(.38f,.008f,.004f):FLinearColor(.9f,.015f,.008f)):(bActivated ? FLinearColor(.06f, .65f, .42f) : FLinearColor(1, .72f, .1f)));
}
void AThunderFlightSwitch::HandlePound(FVector ImpactPoint)
{
    const FVector Local = GetActorTransform().InverseTransformPosition(ImpactPoint);
    if (bActivated || Local.Size2D() > 100 || FMath::Abs(Local.Z - 40) > 15) return;
    if (GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->TryUnlockFlight())
    {
        ++ActivationCount; RefreshFromProgress();
        UE_LOG(LogTemp, Display, TEXT("WING SWITCH: actual ground pound saved leafwing unlock."));
    }
}
void AThunderFlightSwitch::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!BoundActions.IsValid())
        if (auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
        { BoundActions = Player->GetActions(); BoundActions->OnPoundImpact.AddUObject(this, &AThunderFlightSwitch::HandlePound); }
    if (bActivated != GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->IsFlightUnlocked()) RefreshFromProgress();
}
void AThunderFlightSwitch::EndPlay(const EEndPlayReason::Type Reason)
{
    if (BoundActions.IsValid()) BoundActions->OnPoundImpact.RemoveAll(this);
    Super::EndPlay(Reason);
}
