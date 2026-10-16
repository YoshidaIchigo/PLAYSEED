#include "ReferenceWorldMechanisms.h"
#include "ReferenceBattlefieldTerrain.h"
#include "ThunderWarpPair.h"
#include "WindseedCharacter.h"
#include "PlayseedVitalityComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ProceduralMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AReferenceWorldMechanisms::AReferenceWorldMechanisms()
{
    PrimaryActorTick.bCanEverTick=true;
    RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("ReferenceWorldRoot"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    for(int32 Side:{-1,1})
    {
        auto* Hinge=CreateDefaultSubobject<USceneComponent>(*FString::Printf(TEXT("GrillHinge%d"),Side));Hinge->SetupAttachment(RootComponent);Hinge->SetRelativeLocation(FVector(0,Side*320,0));Hinges.Add(Hinge);
        auto Bar=[&](FVector Position,FVector Dimensions,FRotator Rotation=FRotator::ZeroRotator)
        {
            auto* Part=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("GrillBar%d"),Bars.Num()));Part->SetupAttachment(Hinge);Part->SetStaticMesh(Cube.Object);
            Part->SetRelativeLocation(Position);Part->SetRelativeScale3D(Dimensions/100);Part->SetRelativeRotation(Rotation);
            // 隙間26cmは主人公の直径68cmより狭いため、閉じた実際の格子を通り抜けられません。
            Part->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);Part->SetCollisionResponseToAllChannels(ECR_Block);Part->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);Part->SetCastShadow(false);Bars.Add(Part);
        };
        for(int32 I=0;I<=8;++I)
        {const float D=I*40.f,H=384+256*FMath::Min(D/128,1.f);Bar(FVector(0,-Side*D,H*.5f),FVector(14,14,H));}
        for(float Z:{24.f,320.f})Bar(FVector(0,-Side*160,Z),FVector(14,334,14));
        Bar(FVector(0,-Side*224,640),FVector(14,206,14));
        const FVector Slope(0,-Side*128,256);Bar(FVector(0,-Side*64,512),FVector(14,Slope.Size()+14,14),FRotationMatrix::MakeFromY(Slope).Rotator());
    }
    PressurePad=CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("PurpleFloorSwitch"));PressurePad->SetupAttachment(RootComponent);PressurePad->bUseComplexAsSimpleCollision=true;PressurePad->bUseAsyncCooking=false;
    PressurePad->SetCollisionObjectType(ECC_WorldStatic);PressurePad->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);PressurePad->SetCollisionResponseToAllChannels(ECR_Block);
    Heart=CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("RotatingRecoveryHeart"));Heart->SetupAttachment(RootComponent);Heart->SetCollisionEnabled(ECollisionEnabled::NoCollision);Heart->SetCastShadow(false);
}
void AReferenceWorldMechanisms::BeginPlay()
{
    Super::BeginPlay();
    // 自作の斜面付きスイッチ。元の寸法を1.5倍し、実際に立てる54cmの上面を作ります。
    TArray<FVector> P={FVector(-152,-152,0),FVector(152,-152,0),FVector(152,152,0),FVector(-152,152,0),FVector(-91.5f,-91.5f,54),FVector(91.5f,-91.5f,54),FVector(91.5f,91.5f,54),FVector(-91.5f,91.5f,54)};
    TArray<int32> I={4,6,5,4,7,6,0,5,1,0,4,5,1,6,2,1,5,6,2,7,3,2,6,7,3,4,0,3,7,4};
    PressurePad->CreateMeshSection(0,P,I,TArray<FVector>(),TArray<FVector2D>(),TArray<FColor>(),TArray<FProcMeshTangent>(),true);
    PressurePad->SetWorldLocation(FVector(-2283,3682,0));PressurePad->SetWorldRotation(FRotator(0,-63,0));
    // 輪郭式で作る、厚み12cmの独自ハート。画像や既存ゲームのモデルは使いません。
    TArray<FVector> V;TArray<int32> T;constexpr int32 Count=32;
    for(int32 Face=0;Face<2;++Face)
    {
        const float X=Face==0?-6:6;const int32 Start=V.Num();V.Add(FVector(X,0,0));
        for(int32 N=0;N<Count;++N){const float A=N*2*PI/Count;V.Add(FVector(X,48*FMath::Pow(FMath::Sin(A),3),3*(13*FMath::Cos(A)-5*FMath::Cos(2*A)-2*FMath::Cos(3*A)-FMath::Cos(4*A))));}
        for(int32 N=0;N<Count;++N){const int32 A=Start+1+N,B=Start+1+(N+1)%Count;T.Append({Start,Face==0?B:A,Face==0?A:B});}
    }
    for(int32 N=0;N<Count;++N){const int32 A=1+N,B=1+(N+1)%Count,C=A+Count+1,D=B+Count+1;T.Append({A,B,C,B,D,C});}
    Heart->CreateMeshSection(0,V,T,TArray<FVector>(),TArray<FVector2D>(),TArray<FColor>(),TArray<FProcMeshTangent>(),false);Heart->SetWorldLocation(FVector(3603,7070,3659));
    if(auto* Palette=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Battlefield/M_Actor.M_Actor")))
    {
        auto Paint=[&](UMeshComponent* Part,FLinearColor Color){auto* Tint=UMaterialInstanceDynamic::Create(Palette,this);Tint->SetVectorParameterValue(TEXT("Tint"),Color);Part->SetMaterial(0,Tint);return Tint;};
        PadTint=Paint(PressurePad,FLinearColor(.4f,.025f,.6f));Paint(Heart,FLinearColor(.9f,.008f,.04f));for(UStaticMeshComponent* Part:Bars)Paint(Part,FLinearColor(.4f,.42f,.45f));
    }
    auto* Terrain=Cast<AReferenceBattlefieldTerrain>(UGameplayStatics::GetActorOfClass(this,AReferenceBattlefieldTerrain::StaticClass()));
    const FVector Nodes[]={FVector(583,5387,2683),FVector(1680,5523,3835),FVector(-6612,3351,1024),FVector(1980,-6618,768)};
    const float Yaws[]={116,117,17,119};
    for(int32 Pair=0;Pair<2;++Pair)
    {
        FVector Floors[2];bool bValid=true;
        for(int32 N=0;N<2;++N)if(!Terrain||!Terrain->FindFloor(Nodes[Pair*2+N],Floors[N])){++FloorFailures;bValid=false;}
        if(bValid){auto* Warp=GetWorld()->SpawnActor<AThunderWarpPair>();Warp->Configure(Floors[0],Floors[1],Yaws[Pair*2],Yaws[Pair*2+1]);Warp->ConfigureReferenceTrigger();WarpPairs.Add(Warp);}
    }
    ResetMechanisms();
}
FVector AReferenceWorldMechanisms::GetSwitchTop() const{return PressurePad->GetComponentLocation()+FVector(0,0,54*PressurePad->GetComponentScale().Z);}
FVector AReferenceWorldMechanisms::GetHeartLocation() const{return Heart->GetComponentLocation();}
void AReferenceWorldMechanisms::ResetMechanisms()
{
    bGateOpen=false;GateActivations=0;PressedSeconds=Opening=0;PressurePad->SetWorldScale3D(FVector::OneVector);
    for(USceneComponent* Hinge:Hinges)Hinge->SetRelativeRotation(FRotator::ZeroRotator);
    for(AThunderWarpPair* Warp:WarpPairs)Warp->ResetWarps();HeartSpinUnits=400;HeartTotalSpin=0;
    if(PadTint)PadTint->SetVectorParameterValue(TEXT("Tint"),FLinearColor(.4f,.025f,.6f));
}
void AReferenceWorldMechanisms::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);auto* Hero=Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this,0));if(!Hero)return;
    const float Feet=Hero->GetActorLocation().Z-Hero->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    if(!bGateOpen&&Hero->GetMovementBaseObject()==PressurePad&&FVector::Dist2D(Hero->GetActorLocation(),PressurePad->GetComponentLocation())<127.5f&&FMath::Abs(Feet-GetSwitchTop().Z)<12)
    {
        PressedSeconds+=DeltaSeconds;
        if(PressedSeconds>=3.f/30)
        {bGateOpen=true;++GateActivations;PressurePad->SetWorldScale3D(FVector(1,1,2.f/15));UE_LOG(LogTemp,Display,TEXT("REFERENCE WORLD: actual floor switch opened double grill once%d"),GateActivations);}
    }
    else if(!bGateOpen)PressedSeconds=0;
    if(bGateOpen)Opening=FMath::Min(1.f,Opening+DeltaSeconds/(64.f/30));
    for(int32 N=0;N<Hinges.Num();++N)Hinges[N]->SetRelativeRotation(FRotator(0,(N==0?-90:90)*Opening,0));
    const FVector HeartPosition=Heart->GetComponentLocation(),HeroPosition=Hero->GetActorLocation();
    const float Segment=Hero->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()-Hero->GetCapsuleComponent()->GetScaledCapsuleRadius();
    const FVector Closest(HeroPosition.X,HeroPosition.Y,FMath::Clamp(HeartPosition.Z,HeroPosition.Z-Segment,HeroPosition.Z+Segment));
    const bool bTouch=FVector::DistSquared(HeartPosition,Closest)<FMath::Square(50+Hero->GetCapsuleComponent()->GetScaledCapsuleRadius());
    if(bTouch)HeartSpinUnits=1000+200*FMath::Max(0.f,FVector::DotProduct(Hero->GetVelocity(),Hero->GetActorForwardVector()))/30;
    else{HeartSpinUnits=FMath::Max(400.f,HeartSpinUnits-50*DeltaSeconds*30);if(HeartSpinUnits<=400)HeartTotalSpin=0;}
    const float Turn=HeartSpinUnits*DeltaSeconds*30;HeartTotalSpin+=Turn;HeartAngle=FMath::Fmod(HeartAngle+Turn*360/65536,360.f);Heart->SetRelativeRotation(FRotator(0,HeartAngle,0));
    while(HeartTotalSpin>=65536)
    {
        HeartTotalSpin-=65536;
        // 実接触で回ったハートだけが通常の回復窓口を呼びます。QAは体力を設定しません。
        if(Hero->GetVitality()->TryHeal(1)){++HeartHeals;UE_LOG(LogTemp,Display,TEXT("REFERENCE HEART: actual full spin healed1 health%d count%d"),Hero->GetVitality()->GetHealth(),HeartHeals);}
    }
}
