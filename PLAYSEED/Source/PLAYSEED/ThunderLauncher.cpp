#include "ThunderLauncher.h"
#include "ThunderExpeditionSubsystem.h"
#include "PlayseedInteractionComponent.h"
#include "PlayseedLaunchComponent.h"
#include "WindseedCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AThunderLauncher::AThunderLauncher()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SkybloomRoot"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    auto Part = [this](FName Name, UStaticMesh* Mesh, FVector Position, FVector Scale, FLinearColor Color)
    {
        auto* Shape = CreateDefaultSubobject<UStaticMeshComponent>(Name);
        Shape->SetupAttachment(RootComponent); Shape->SetStaticMesh(Mesh);
        Shape->SetRelativeLocation(Position); Shape->SetRelativeScale3D(Scale);
        Shape->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Parts.Add(Shape); Colors.Add(Color); return Shape;
    };
    Part(TEXT("Foundation"), Cylinder.Object, FVector(0, 0, 15), FVector(2, 2, .3f), FLinearColor(.06f, .18f, .24f));
    Barrel = Part(TEXT("CopperStem"), Cylinder.Object, FVector(0, 0, 100), FVector(.55f, .55f, 1.5f), FLinearColor(.85f, .45f, .1f));
    for (int32 Index = 0; Index < 8; ++Index)
    {
        const float Angle = Index * PI / 4;
        auto* Petal = Part(*FString::Printf(TEXT("Petal%d"), Index), Sphere.Object, FVector(FMath::Cos(Angle) * 90, FMath::Sin(Angle) * 90, 35), FVector(.9f, .35f, .16f), FLinearColor(1, .8f, .14f));
        Petal->SetRelativeRotation(FRotator(0, FMath::RadiansToDegrees(Angle), 0));
    }
}
void AThunderLauncher::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette")))
        for (int32 Index = 0; Index < Parts.Num(); ++Index)
        {
            auto* Tint = UMaterialInstanceDynamic::Create(Palette, this);
            Tint->SetVectorParameterValue(TEXT("Tint"), Colors[Index]); Parts[Index]->SetMaterial(0, Tint);
        }
    RefreshOpenState();
}
void AThunderLauncher::RefreshOpenState()
{
    bOpen = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->IsLauncherUnlocked();
    Barrel->SetRelativeRotation(FRotator(bOpen ? 20 : 90, bOpen ? GetDefaultAimYaw() : 0, 0));
}
void AThunderLauncher::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bOpen != GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->IsLauncherUnlocked()) RefreshOpenState();
}
void AThunderLauncher::SetAimPose(float Pitch, float WorldYaw) { Barrel->SetRelativeRotation(FRotator(90 - Pitch, WorldYaw - GetActorRotation().Yaw, 0)); }
void AThunderLauncher::Interact(AWindseedCharacter* Player)
{
    if (!Player) return;
    if (Player->GetLaunch()->IsAiming()) { Player->GetLaunch()->CancelAim(); return; }
    if (!bOpen) { Player->GetInteraction()->ShowMessage(TEXT("The Skybloom is folded. Ask Aeri nearby to open it.")); return; }
    Player->GetLaunch()->TryBeginAim(this);
}
