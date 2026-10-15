#include "ThunderTiltingBridge.h"
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
    RootComponent = Surface;
    Surface->SetMobility(EComponentMobility::Movable);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Surface->SetStaticMesh(Cube.Object);
    Surface->SetRelativeScale3D(FVector(12, 3.2f, .4f));
    Surface->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Surface->SetCollisionResponseToAllChannels(ECR_Block);
    Surface->CanCharacterStepUpOn = ECB_Yes;
}

void AThunderTiltingBridge::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        auto* Material = UMaterialInstanceDynamic::Create(Palette, this);
        Material->SetVectorParameterValue(TEXT("Tint"), FLinearColor(.55f, .24f, .12f));
        Surface->SetMaterial(0, Material);
    }
}

void AThunderTiltingBridge::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
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
    return GetActorLocation() + GetActorRotation().RotateVector(FVector(LongitudinalOffset, 0, 20));
}

void AThunderTiltingBridge::ResetTilt()
{
    SetActorRotation(FRotator::ZeroRotator, ETeleportType::None);
}
