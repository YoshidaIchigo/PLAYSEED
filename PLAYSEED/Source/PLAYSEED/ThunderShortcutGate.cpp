#include "ThunderShortcutGate.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AThunderShortcutGate::AThunderShortcutGate()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ShortcutRoot"));
    Barrier = CreateDefaultSubobject<UBoxComponent>(TEXT("ClosedBarrier"));
    Barrier->SetupAttachment(RootComponent);
    Barrier->SetRelativeLocation(FVector(0, 0, 170));
    Barrier->SetBoxExtent(FVector(30, 160, 170));
    Barrier->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Barrier->SetCollisionResponseToAllChannels(ECR_Block);
    GateVisual = CreateDefaultSubobject<USceneComponent>(TEXT("RisingGate"));
    GateVisual->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    for (int32 Index = 0; Index < 6; ++Index)
    {
        auto* Bar = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("GateBar%d"), Index));
        Bar->SetupAttachment(GateVisual);
        Bar->SetStaticMesh(Cube.Object);
        Bar->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Bar->SetRelativeLocation(Index < 5 ? FVector(0, -140 + Index * 70, 170) : FVector(0, 0, 325));
        Bar->SetRelativeScale3D(Index < 5 ? FVector(.24f, .24f, 3.4f) : FVector(.24f, 3.2f, .24f));
        Bars.Add(Bar);
    }
    PressurePad = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PressurePad"));
    PressurePad->SetupAttachment(RootComponent);
    PressurePad->SetStaticMesh(Cube.Object);
    PressurePad->SetRelativeLocation(FVector(-450, -300, 10));
    PressurePad->SetRelativeScale3D(FVector(2, 2, .2f));
    PressurePad->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    PressurePad->SetCollisionResponseToAllChannels(ECR_Block);
    PressurePad->CanCharacterStepUpOn = ECB_Yes;
}

void AThunderShortcutGate::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        PadMaterial = UMaterialInstanceDynamic::Create(Palette, this);
        PadMaterial->SetVectorParameterValue(TEXT("Tint"), FLinearColor(.72f, .16f, .65f));
        PressurePad->SetMaterial(0, PadMaterial);
        auto* Metal = UMaterialInstanceDynamic::Create(Palette, this);
        Metal->SetVectorParameterValue(TEXT("Tint"), FLinearColor(.18f, .25f, .3f));
        for (UStaticMeshComponent* Bar : Bars) Bar->SetMaterial(0, Metal);
    }
}

void AThunderShortcutGate::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
    // 近くを通っただけでは押されません。実際にこの床へ立ったことを床参照と足元で確かめます。
    if (!bOpen && Player && Player->GetMovementBaseObject() == PressurePad &&
        FMath::Abs(Player->GetActorLocation().Z - Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - GetSwitchTop().Z) < 10)
        OpenGate();
    const float Height = FMath::FInterpTo(GateVisual->GetRelativeLocation().Z, bOpen ? 370.f : 0.f, DeltaSeconds, 5);
    GateVisual->SetRelativeLocation(FVector(0, 0, Height));
}

void AThunderShortcutGate::OpenGate()
{
    if (bOpen) return; // 同じスイッチに立ち続けても、一度しか作動しません。
    bOpen = true;
    ++ActivationCount;
    Barrier->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (PadMaterial) PadMaterial->SetVectorParameterValue(TEXT("Tint"), FLinearColor(.12f, .7f, .45f));
}

void AThunderShortcutGate::ResetGate()
{
    bOpen = false;
    ActivationCount = 0;
    GateVisual->SetRelativeLocation(FVector::ZeroVector);
    Barrier->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    if (PadMaterial) PadMaterial->SetVectorParameterValue(TEXT("Tint"), FLinearColor(.72f, .16f, .65f));
}
