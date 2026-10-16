#include "ReferenceHero.h"
#include "PlayseedActionComponent.h"
#include "PlayseedCarryComponent.h"
#include "PlayseedClimbComponent.h"
#include "PlayseedCombatComponent.h"
#include "PlayseedFlightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

UStaticMeshComponent* AReferenceHero::Part(USceneComponent* Parent,UStaticMesh* Mesh,FVector Position,FVector Dimensions,FLinearColor Color)
{
    auto* Shape=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("ReferencePart%d"),Parts.Num()));
    Shape->SetupAttachment(Parent);Shape->SetStaticMesh(Mesh);Shape->SetRelativeLocation(Position);
    // 見た目の部品に衝突は付けません。ジャンプや壁判定は既存のカプセル一つに任せます。
    Shape->SetRelativeScale3D(Dimensions/100);Shape->SetCollisionEnabled(ECollisionEnabled::NoCollision);Shape->SetCastShadow(false);
    Parts.Add(Shape);Colors.Add(Color);return Shape;
}

AReferenceHero::AReferenceHero()
{
    Tags.Add(TEXT("ReferenceHero"));
    Visual=CreateDefaultSubobject<USceneComponent>(TEXT("ReferenceVisual"));Visual->SetupAttachment(GetRootComponent());
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Plane(TEXT("/Engine/BasicShapes/Plane.Plane"));
    const FLinearColor Red(.85f,.025f,.02f),Blue(.035f,.12f,.8f),Skin(.85f,.47f,.25f),White(.92f,.92f,.88f),Brown(.22f,.075f,.025f),Black(.012f,.009f,.006f),Hair(.12f,.045f,.012f);
    Part(Visual,Sphere.Object,FVector(-3,0,6),FVector(69,69,67),Red);
    Part(Visual,Sphere.Object,FVector(0,0,-22),FVector(63,66,47),Blue);
    Part(Visual,Sphere.Object,FVector(24,0,1),FVector(20,48,44),Blue);
    for(int32 Side:{-1,1})
    {
        Part(Visual,Cube.Object,FVector(26,Side*20,17),FVector(6,11,31),Blue);
        Part(Visual,Cube.Object,FVector(-27,Side*20,16),FVector(5,11,34),Blue);
        Part(Visual,Sphere.Object,FVector(32,Side*20,11),FVector(6,9,9),FLinearColor(1,.76f,.025f));
    }
    Part(Visual,Sphere.Object,FVector(-20,0,49),FVector(40,60,49),Hair);
    Part(Visual,Sphere.Object,FVector(2,0,43),FVector(58,63,58),Skin);
    Part(Visual,Sphere.Object,FVector(34,0,44),FVector(29,30,27),Skin);
    Part(Visual,Sphere.Object,FVector(33,0,32),FVector(12,54,13),Black);
    Part(Visual,Sphere.Object,FVector(32,0,25),FVector(5,20,6.5f),Black);
    for(int32 Side:{-1,1})
    {
        Part(Visual,Sphere.Object,FVector(0,Side*34,46),FVector(22,15,30),Skin);
        Part(Visual,Sphere.Object,FVector(28,Side*13,54),FVector(12,18,25),White);
        Part(Visual,Sphere.Object,FVector(34,Side*13,54),FVector(3.5f,9,16),FLinearColor(.025f,.24f,.85f));
        Part(Visual,Sphere.Object,FVector(36,Side*13,54),FVector(2,4.5f,11),Black);
        Part(Visual,Sphere.Object,FVector(28,Side*14,65),FVector(9,22,5.5f),Black);
        for(int32 I=0;I<3;++I)Part(Visual,Sphere.Object,FVector(36,Side*(6+I*7),28+I%2),FVector(10,14,11),Black);
    }
    Part(Visual,Sphere.Object,FVector(-1,0,72),FVector(69,72,28),Red);
    Part(Visual,Sphere.Object,FVector(30,0,66),FVector(58,70,8),Red);
    Part(Visual,Sphere.Object,FVector(30,0,76),FVector(4,30,20),White);
    auto* Letter=CreateDefaultSubobject<UTextRenderComponent>(TEXT("ReferenceCapLetter"));Letter->SetupAttachment(Visual);
    Letter->SetText(FText::FromString(TEXT("M")));Letter->SetRelativeLocation(FVector(33,0,76));Letter->SetWorldSize(17);
    Letter->SetHorizontalAlignment(EHTA_Center);Letter->SetVerticalAlignment(EVRTA_TextCenter);Letter->SetTextRenderColor(FColor(235,15,10));Letter->SetCastShadow(false);
    LeftArm=CreateDefaultSubobject<USceneComponent>(TEXT("ReferenceLeftArm"));LeftArm->SetupAttachment(Visual);LeftArm->SetRelativeLocation(FVector(-1,-37,9));
    RightArm=CreateDefaultSubobject<USceneComponent>(TEXT("ReferenceRightArm"));RightArm->SetupAttachment(Visual);RightArm->SetRelativeLocation(FVector(-1,37,9));
    LeftLeg=CreateDefaultSubobject<USceneComponent>(TEXT("ReferenceLeftLeg"));LeftLeg->SetupAttachment(Visual);LeftLeg->SetRelativeLocation(FVector(-1,-15,-36));
    RightLeg=CreateDefaultSubobject<USceneComponent>(TEXT("ReferenceRightLeg"));RightLeg->SetupAttachment(Visual);RightLeg->SetRelativeLocation(FVector(-1,15,-36));
    for(USceneComponent* Arm:{LeftArm.Get(),RightArm.Get()})
    {
        Part(Arm,Sphere.Object,FVector(0,0,-16),FVector(28,28,49),Red);
        Part(Arm,Sphere.Object,FVector(4,0,-43),FVector(31,31,30),White);
    }
    for(USceneComponent* Leg:{LeftLeg.Get(),RightLeg.Get()})
    {
        Part(Leg,Sphere.Object,FVector(0,0,-15),FVector(28,28,55),Blue);
        Part(Leg,Sphere.Object,FVector(12,0,-33),FVector(55,34,28),Brown);
    }
    for(int32 Side:{-1,1})
    {
        VictoryFingers.Add(Part(RightArm,Sphere.Object,FVector(4,Side*5,-62),FVector(6,6,28),White));
        for(int32 I=0;I<4;++I)CapFeathers.Add(Part(Visual,Sphere.Object,FVector(-5,Side*(40+I*8),74+I*5),FVector(16,9,48+I*4),White));
    }
    Shadow=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ReferenceShadow"));Shadow->SetupAttachment(GetRootComponent());Shadow->SetStaticMesh(Plane.Object);
    Shadow->SetAbsolute(true,true,true);Shadow->SetCollisionEnabled(ECollisionEnabled::NoCollision);Shadow->SetCastShadow(false);
}

void AReferenceHero::BeginPlay()
{
    Super::BeginPlay();
    TArray<UStaticMeshComponent*> AllMeshes;GetComponents(AllMeshes);
    for(auto* Mesh:AllMeshes)if(!Mesh->GetName().StartsWith(TEXT("Reference"))){Mesh->SetVisibility(false);Mesh->SetHiddenInGame(true);}
    auto* Palette=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Battlefield/M_Actor.M_Actor"));
    for(int32 I=0;I<Parts.Num();++I)if(Palette)
    {
        auto* Tint=UMaterialInstanceDynamic::Create(Palette,this);Tint->SetVectorParameterValue(TEXT("Tint"),Colors[I]);
        for(int32 Slot=0;Slot<Parts[I]->GetNumMaterials();++Slot)Parts[I]->SetMaterial(Slot,Tint);
    }
    if(auto* Material=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Battlefield/M_Shadow.M_Shadow")))Shadow->SetMaterial(0,Material);
    for(UStaticMeshComponent* Finger:VictoryFingers)Finger->SetVisibility(false);
    for(UStaticMeshComponent* Feather:CapFeathers)Feather->SetVisibility(false);
}

void AReferenceHero::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const float Speed=GetVelocity().Size2D();const float Time=GetWorld()->GetTimeSeconds();
    const float Stride=FMath::Sin(Time*13)*FMath::Clamp(Speed/470,0.f,1.f)*35;
    FRotator LeftArmPose(Stride,0,0),RightArmPose(-Stride,0,0),LeftLegPose(-Stride,0,0),RightLegPose(Stride,0,0);
    const bool bClimbing=GetClimb()->IsClimbing(),bHolding=GetCarry()->GetHeldObject()!=nullptr,bPounding=GetActions()->IsGroundPounding();
    if(bClimbing || bHolding){LeftArmPose=RightArmPose=FRotator(-100,0,0);}
    if(GetCombat()->IsAttacking())
    {
        if(GetCombat()->GetComboStage()==1)LeftArmPose=FRotator(-95,0,0);
        if(GetCombat()->GetComboStage()==2)RightArmPose=FRotator(-95,0,0);
        if(GetCombat()->GetComboStage()==3)RightLegPose=FRotator(-85,0,0);
    }
    if(bStarPose){LeftArmPose=FRotator(-15,0,20);RightArmPose=FRotator(-160,0,-20);LeftLegPose=FRotator(-12,0,0);RightLegPose=FRotator::ZeroRotator;}
    LeftArm->SetRelativeRotation(LeftArmPose);RightArm->SetRelativeRotation(RightArmPose);LeftLeg->SetRelativeRotation(LeftLegPose);RightLeg->SetRelativeRotation(RightLegPose);
    Visual->SetRelativeScale3D(bPounding?FVector(1.15f,1.15f,.7f):FVector::OneVector);
    Visual->SetRelativeLocation(FVector(0,0,bStarPose?0:FMath::Sin(Time*12)*FMath::Min(Speed/180,3.f)));
    Visual->SetRelativeRotation(bClimbing?FRotator(-12,0,0):FRotator::ZeroRotator);
    for(UStaticMeshComponent* Finger:VictoryFingers)Finger->SetVisibility(bStarPose);
    for(UStaticMeshComponent* Feather:CapFeathers)Feather->SetVisibility(GetFlight()->HasPower());
    // 足元へ影だけを投影します。読み取った床の位置でプレイヤーを動かすことはありません。
    FHitResult Floor;FCollisionQueryParams Params(SCENE_QUERY_STAT(ReferenceHeroShadow),true,this);
    const bool bFloor=GetWorld()->LineTraceSingleByChannel(Floor,GetActorLocation()+FVector(0,0,40),GetActorLocation()-FVector(0,0,2500),ECC_Visibility,Params) && Floor.ImpactNormal.Z>.5f;
    Shadow->SetVisibility(bFloor);
    if(bFloor)
    {
        Shadow->SetWorldLocation(Floor.ImpactPoint+Floor.ImpactNormal*1.5f);Shadow->SetWorldRotation(FRotationMatrix::MakeFromZ(Floor.ImpactNormal).Rotator());
        const float Size=FMath::Clamp(1.15f-(GetActorLocation().Z-82-Floor.ImpactPoint.Z)*.0004f,.65f,1.15f);Shadow->SetWorldScale3D(FVector(Size,Size,1));
    }
}
