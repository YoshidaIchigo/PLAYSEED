#include "ThunderTiltingBridge.h"
#include "ReferenceBattlefieldTerrain.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AThunderTiltingBridge::AThunderTiltingBridge()
{
    PrimaryActorTick.bCanEverTick = true;
    Surface = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BalanceDeck"));
    RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("BalancePivot"));
    Surface->SetupAttachment(RootComponent);
    Surface->SetMobility(EComponentMobility::Movable);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Surface->SetStaticMesh(Cube.Object);
    bReferenceBridge=AReferenceBattlefieldTerrain::IsReferenceCourse();
    Surface->SetRelativeScale3D(bReferenceBridge?FVector(18.09f,3.61f,.51f):FVector(12,3.2f,.4f));
    if(bReferenceBridge)Surface->SetRelativeLocation(FVector(.5f,.5f,25.5f));
    Surface->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Surface->SetCollisionResponseToAllChannels(ECR_Block);
    Surface->CanCharacterStepUpOn = ECB_Yes;
}

void AThunderTiltingBridge::BeginPlay()
{
    Super::BeginPlay();BaseYaw=GetActorRotation().Yaw;
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, bReferenceBridge?TEXT("/Game/Battlefield/M_Actor.M_Actor"):TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        auto* Material = UMaterialInstanceDynamic::Create(Palette, this);
        Material->SetVectorParameterValue(TEXT("Tint"), FLinearColor(.55f, .24f, .12f));
        Surface->SetMaterial(0, Material);
    }
}

void AThunderTiltingBridge::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if(bReferenceBridge)
    {
        auto* Hero=UGameplayStatics::GetPlayerCharacter(this,0);ReferenceAccumulator+=DeltaSeconds;
        while(ReferenceAccumulator>=1./30.)
        {
            ReferenceAccumulator-=1./30.;ReferencePitch+=static_cast<int32>(ReferenceVelocity);
            if(Hero&&Hero->GetMovementBaseObject()==Surface)
            {
                const float Along=FVector::DotProduct(Hero->GetActorLocation()-GetActorLocation(),FRotator(0,BaseYaw,0).Vector());
                ReferenceVelocity=FMath::Clamp(ReferenceVelocity+Along*(ReferenceVelocity*Along<0?.04f:.02f),-50.f,50.f);
            }
            else
            {
                // 乗っていない時は、元の小さな減衰振動で水平へ戻します。
                const int32 Previous=ReferencePitch;ReferencePitch+=static_cast<int32>(ReferenceVelocity);
                if(ReferencePitch==0||(static_cast<int64>(ReferencePitch)*Previous<0&&FMath::Abs(ReferenceVelocity)<6))
                {ReferencePitch=0;ReferenceVelocity=0;}
                else {float Acceleration=ReferencePitch>=0?-3.f:3.f;if(ReferenceVelocity*Acceleration<0)Acceleration*=3;ReferenceVelocity+=Acceleration;}
            }
        }
        SetActorRotation(FRotator(-ReferencePitch*(360./65536.),BaseYaw,0),ETeleportType::None);return;
    }
    float TargetPitch = 0;
    auto* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (Player && Player->GetMovementBaseObject() == Surface)
    {
        // スケールを含めない回転の逆変換で、橋の中心から何 cm 離れたかを求めます。
        // 立っている側を下げ、最大10度に制限して歩ける傾斜を保ちます。
        const float AlongDeck = GetActorRotation().UnrotateVector(Player->GetActorLocation() - GetActorLocation()).X;
        TargetPitch = FMath::Clamp(-AlongDeck / 600.f * 10.f, -10.f, 10.f);
    }
    // 補間に DeltaSeconds を渡すと、描画頻度によらずゆっくり傾きます。
    const float Pitch = FMath::FInterpTo(GetActorRotation().Pitch, TargetPitch, DeltaSeconds, 2.f);
    SetActorRotation(FRotator(Pitch, 0, 0), ETeleportType::None);
}

FVector AThunderTiltingBridge::GetDeckPoint(float LongitudinalOffset) const
{
    return GetActorLocation() + GetActorRotation().RotateVector(FVector(LongitudinalOffset, 0, bReferenceBridge?51:20));
}

void AThunderTiltingBridge::ResetTilt()
{
    ReferencePitch=0;ReferenceVelocity=0;ReferenceAccumulator=0;
    SetActorRotation(FRotator(0,bReferenceBridge?BaseYaw:0,0),ETeleportType::None);
}
