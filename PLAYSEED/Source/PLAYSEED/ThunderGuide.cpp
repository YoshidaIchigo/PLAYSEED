#include "ThunderGuide.h"
#include "ThunderExpeditionSubsystem.h"
#include "ReferenceBattlefieldTerrain.h"
#include "Kismet/GameplayStatics.h"
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
    bReferenceGuide=AReferenceBattlefieldTerrain::IsReferenceCourse();
    if(bReferenceGuide)
    {
        static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
        Part(TEXT("PinkBomb"),Sphere.Object,FVector(0,0,85),FVector(1.35f),FLinearColor(1,.26f,.5f));
        Part(TEXT("WhiteFuse"),Cylinder.Object,FVector(0,0,165),FVector(.11f,.11f,.35f),FLinearColor(.95f,.93f,.8f));
        for(int32 Side:{-1,1})
        {
            Part(*FString::Printf(TEXT("GoldFoot%d"),Side),Sphere.Object,FVector(15,Side*30,20),FVector(.6f,.4f,.3f),FLinearColor(1,.65f,.03f));
            Part(*FString::Printf(TEXT("WhiteEye%d"),Side),Sphere.Object,FVector(62,Side*20,98),FVector(.08f,.15f,.3f),FLinearColor(.95f,.95f,.85f));
        }
        return;
    }
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
    if(bReferenceGuide)
    {
        auto* Terrain=Cast<AReferenceBattlefieldTerrain>(UGameplayStatics::GetActorOfClass(this,AReferenceBattlefieldTerrain::StaticClass()));
        FVector Floor;if(Terrain&&Terrain->FindFloor(GetActorLocation(),Floor))SetActorLocation(Floor);
    }
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr,bReferenceGuide?TEXT("/Game/Battlefield/M_Actor.M_Actor"):TEXT("/Game/Windseed/M_Palette.M_Palette")))
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
    if(bReferenceGuide)SetActorRotation(FRotator(0,(Player->GetActorLocation()-GetActorLocation()).Rotation().Yaw,0));
    if(bReferenceGuide&&!bCanOpenCannon)
    {
        Dialogue->ShowMessage(Progress->GetSelectedMissionNumber()==1
            ?TEXT("まずは山頂の大きなボスをたおしてみよう。\n大砲は、そのあとの探検に使えるよ。")
            :TEXT("山道と空の島を探検してみよう。\n大砲を開けるのは、もうひとりの仲間だよ。"));return;
    }
    if (Progress->IsLauncherUnlocked()) { bAwaitingConfirmation = false; Dialogue->ShowMessage(bReferenceGuide?TEXT("大砲はもう使えるよ。\n空の島をねらってみてね。"):TEXT("AERI: The Skybloom is open. Follow the golden petals.")); return; }
    if (!bAwaitingConfirmation)
    {
        bAwaitingConfirmation = true;
        Dialogue->ShowMessage(bReferenceGuide?TEXT("大砲のふたを開けてあげよう！\nもういちどGで開けるよ。"):TEXT("AERI: I can unfold the Skybloom. Press G / D-pad Down again."));
        return;
    }
    if (Progress->TryUnlockLauncher())
    {
        bAwaitingConfirmation = false;
        Dialogue->ShowMessage(bReferenceGuide?TEXT("大砲が使えるようになったよ！\nGで入って、マウスでねらい、左クリックで発射。"):TEXT("AERI: Skybloom unlocked. Your trail journal remembers."));
        UE_LOG(LogTemp, Display, TEXT("THUNDER GUIDE: Skybloom unlocked through actual dialogue."));
    }
    else Dialogue->ShowMessage(bReferenceGuide?TEXT("保存できなかったよ。\nもういちどGで試してね。"):TEXT("AERI: The trail journal could not be saved. Please try again."));
}
