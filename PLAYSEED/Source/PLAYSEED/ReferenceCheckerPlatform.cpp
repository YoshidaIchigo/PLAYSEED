#include "ReferenceCheckerPlatform.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AReferenceCheckerPlatform::AReferenceCheckerPlatform()
{
    PrimaryActorTick.bCanEverTick=true;
    RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("CheckerRoot"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Surface=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CheckerCollision"));Surface->SetupAttachment(RootComponent);
    Surface->SetStaticMesh(Cube.Object);Surface->SetRelativeLocation(FVector(.35f,-.35f,.75f));
    Surface->SetRelativeScale3D(FVector(3.577f,2.149f,.765f));
    Surface->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);Surface->SetCollisionResponseToAllChannels(ECR_Block);
    Surface->CanCharacterStepUpOn=ECB_Yes;Surface->SetMobility(EComponentMobility::Movable);
    for(int32 X=0;X<4;++X)for(int32 Y=0;Y<4;++Y)for(int32 Side:{-1,1})
    {
        auto* Tile=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Checker%d_%d_%d"),X,Y,Side));
        Tile->SetupAttachment(RootComponent);Tile->SetStaticMesh(Cube.Object);
        Tile->SetRelativeLocation(FVector(-178.5f+(X+.5f)*89.425f,-107.8f+(Y+.5f)*53.725f,Side>0?39.6f:-38.1f));
        Tile->SetRelativeScale3D(FVector(.89425f,.53725f,.012f));Tile->SetCollisionEnabled(ECollisionEnabled::NoCollision);Tiles.Add(Tile);
    }
}
void AReferenceCheckerPlatform::BeginPlay()
{
    Super::BeginPlay();
    if(auto* Palette=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Battlefield/M_Actor.M_Actor")))
    {
        auto* Edge=UMaterialInstanceDynamic::Create(Palette,this);Edge->SetVectorParameterValue(TEXT("Tint"),FLinearColor(.18f,.24f,.25f));Surface->SetMaterial(0,Edge);
        for(int32 I=0;I<Tiles.Num();++I)
        {
            auto* Material=UMaterialInstanceDynamic::Create(Palette,this);
            const bool bDark=((I/8)+(I/2)%4)%2!=0;
            Material->SetVectorParameterValue(TEXT("Tint"),bDark?FLinearColor(.18f,.24f,.25f):FLinearColor(.85f,.6f,.12f));Tiles[I]->SetMaterial(0,Material);
        }
    }
}
void AReferenceCheckerPlatform::Configure(int32 Index)
{
    if(bConfigured)return;
    PlatformIndex=FMath::Clamp(Index,0,1);Origin=GetActorLocation();bConfigured=true;ResetMotion();
}
void AReferenceCheckerPlatform::ResetMotion()
{
    Accumulator=0;Timer=0;MoveAngle=FaceAngle=0;Action=PlatformIndex==0?1:3;
    Offset=FVector(0,PlatformIndex==0?145:-145,PlatformIndex*400);
    SetActorLocationAndRotation(Origin+Offset,FRotator::ZeroRotator,false,nullptr,ETeleportType::None);
}
void AReferenceCheckerPlatform::StepMotion()
{
    const int32 PreviousAction=Action;double Forward=0,Vertical=0;
    if(Action==1||Action==3)
    {
        MoveAngle=0;Vertical=Action==1?10:-10;
        if(Timer>40)++Action;
    }
    else
    {
        const int32 Direction=Action==2?1:-1;
        MoveAngle+=512;FaceAngle+=512;
        const double Angle=MoveAngle*(2*PI/65536.);
        Forward=Direction*FMath::Sin(Angle)*7;Vertical=Direction*FMath::Cos(Angle)*7;
        if(Timer+1==64){Action=Action==2?3:1;if(Action==1)FaceAngle&=~0x7FFF;}
    }
    // 元の前方+ZはUEの-Y、高さYはUEのZです。プレイヤーの座標は変更しません。
    Offset+=FVector(0,-Forward,Vertical);Timer=Action==PreviousAction?Timer+1:0;
}
void AReferenceCheckerPlatform::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);if(!bConfigured)return;
    Accumulator+=DeltaSeconds;
    while(Accumulator>=1./30.){StepMotion();Accumulator-=1./30.;}
    // N64の右方向Xのまわりの回転をUEのRollへ変換します。
    SetActorLocationAndRotation(Origin+Offset,FRotator(0,0,-FaceAngle*(360./65536.)),false,nullptr,ETeleportType::None);
}
