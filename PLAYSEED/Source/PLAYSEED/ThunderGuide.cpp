#include "ThunderGuide.h"
#include "ThunderExpeditionSubsystem.h"
#include "PlayseedInteractionComponent.h"
#include "WindseedCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AThunderGuide::AThunderGuide()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("AeriRoot"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
    auto Part = [this](FName Name, UStaticMesh* Mesh, FVector Position, FVector Scale, FLinearColor Color)
    {
        auto* Shape = CreateDefaultSubobject<UStaticMeshComponent>(Name);
        Shape->SetupAttachment(RootComponent); Shape->SetStaticMesh(Mesh);
        Shape->SetRelativeLocation(Position); Shape->SetRelativeScale3D(Scale);
        Shape->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Parts.Add(Shape); Colors.Add(Color);
    };
    Part(TEXT("CopperBody"), Sphere.Object, FVector(0, 0, 65), FVector(.8f, .8f, 1.2f), FLinearColor(.8f, .43f, .14f));
    Part(TEXT("TealHat"), Cone.Object, FVector(0, 0, 140), FVector(1.1f, 1.1f, .7f), FLinearColor(.06f, .58f, .48f));
    for (int32 Side : {-1, 1})
    {
        Part(*FString::Printf(TEXT("Boot%d"), Side), Sphere.Object, FVector(10, Side * 24, 17), FVector(.6f, .38f, .3f), FLinearColor(.04f, .15f, .21f));
        Part(*FString::Printf(TEXT("Eye%d"), Side), Sphere.Object, FVector(-36, Side * 17, 90), FVector(.15f, .15f, .2f), FLinearColor(.9f, 1, .75f));
    }
}
void AThunderGuide::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette")))
        for (int32 Index = 0; Index < Parts.Num(); ++Index)
        {
            auto* Tint = UMaterialInstanceDynamic::Create(Palette, this);
            Tint->SetVectorParameterValue(TEXT("Tint"), Colors[Index]); Parts[Index]->SetMaterial(0, Tint);
        }
}
void AThunderGuide::Interact(AWindseedCharacter* Player)
{
    if (!Player) return;
    auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    auto* Dialogue = Player->GetInteraction();
    if (Progress->IsLauncherUnlocked()) { bAwaitingConfirmation = false; Dialogue->ShowMessage(TEXT("AERI: The Skybloom is open. Follow the golden petals.")); return; }
    if (!bAwaitingConfirmation)
    {
        bAwaitingConfirmation = true;
        Dialogue->ShowMessage(TEXT("AERI: I can unfold the Skybloom. Press G / D-pad Down again."));
        return;
    }
    if (Progress->TryUnlockLauncher())
    {
        bAwaitingConfirmation = false;
        Dialogue->ShowMessage(TEXT("AERI: Skybloom unlocked. Your trail journal remembers."));
        UE_LOG(LogTemp, Display, TEXT("THUNDER GUIDE: Skybloom unlocked through actual dialogue."));
    }
    else Dialogue->ShowMessage(TEXT("AERI: The trail journal could not be saved. Please try again."));
}
