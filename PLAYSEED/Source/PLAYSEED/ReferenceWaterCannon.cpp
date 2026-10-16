#include "ReferenceWaterCannon.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AReferenceWaterCannon::AReferenceWaterCannon()
{
    PrimaryActorTick.bCanEverTick=true;
    RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("WaterCannonRoot"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    Body=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterCannonBase"));Body->SetupAttachment(RootComponent);Body->SetStaticMesh(Cylinder.Object);Body->SetRelativeLocation(FVector(0,0,150));Body->SetRelativeScale3D(FVector(4.4f,4.4f,3));
    // 元の押し出し範囲と同じ半径220cm/高さ300cmを、普通のカプセル衝突で守ります。
    Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);Body->SetCollisionResponseToAllChannels(ECR_Block);
    Rim=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterCannonRim"));Rim->SetupAttachment(RootComponent);Rim->SetStaticMesh(Cylinder.Object);Rim->SetRelativeLocation(FVector(0,0,298));Rim->SetRelativeScale3D(FVector(4.5f,4.5f,.08f));
    Barrel=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterCannonBarrel"));Barrel->SetupAttachment(RootComponent);Barrel->SetStaticMesh(Cylinder.Object);Barrel->SetRelativeScale3D(FVector(1.5f,1.5f,2.8f));
    Muzzle=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterCannonMuzzle"));Muzzle->SetupAttachment(RootComponent);Muzzle->SetStaticMesh(Cylinder.Object);Muzzle->SetRelativeScale3D(FVector(1.24f,1.24f,.03f));
    Shot=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterCannonShot"));Shot->SetupAttachment(RootComponent);Shot->SetStaticMesh(Sphere.Object);Shot->SetRelativeScale3D(FVector(1.6f));
    for(UStaticMeshComponent* Part:{Rim.Get(),Barrel.Get(),Muzzle.Get(),Shot.Get()})Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    for(UStaticMeshComponent* Part:{Body.Get(),Rim.Get(),Barrel.Get(),Muzzle.Get(),Shot.Get()})Part->SetCastShadow(false);
}
void AReferenceWaterCannon::BeginPlay()
{
    Super::BeginPlay();
    if(auto* Palette=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Battlefield/M_Actor.M_Actor")))
    {
        auto Paint=[&](UStaticMeshComponent* Part,FLinearColor Color){auto* Tint=UMaterialInstanceDynamic::Create(Palette,this);Tint->SetVectorParameterValue(TEXT("Tint"),Color);Part->SetMaterial(0,Tint);};
        Paint(Body,FLinearColor(.004f,.006f,.01f));Paint(Rim,FLinearColor(.07f,.09f,.12f));Paint(Barrel,FLinearColor(.015f,.08f,.4f));Paint(Muzzle,FLinearColor(.001f,.0015f,.002f));Paint(Shot,FLinearColor(.02f,.18f,.65f));
    }
    ResetCannon();
}
void AReferenceWaterCannon::ResetCannon()
{
    AimYaw=TargetYaw=0;AimPitch=TargetPitch=39.375f;Clock=ShotAge=0;bShotActive=false;bVisible=false;Shot->SetVisibility(false);UpdateBarrel();
}
void AReferenceWaterCannon::UpdateBarrel()
{
    const FVector Direction=FRotator(AimPitch,AimYaw,0).Vector();const FVector Pivot(0,0,220);
    Barrel->SetRelativeLocation(Pivot+Direction*110);Barrel->SetRelativeRotation(FRotationMatrix::MakeFromZ(Direction).Rotator());
    Muzzle->SetRelativeLocation(Pivot+Direction*252);Muzzle->SetRelativeRotation(FRotationMatrix::MakeFromZ(Direction).Rotator());
}
void AReferenceWaterCannon::Fire()
{
    const FVector Direction=GetActorRotation().RotateVector(FRotator(AimPitch,AimYaw,0).Vector());
    ShotPosition=GetActorLocation()+FVector(0,0,220)+Direction*260;ShotVelocity=Direction*3000;ShotAge=0;bShotActive=true;++ShotCount;
    Shot->SetWorldLocation(ShotPosition);Shot->SetVisibility(true);
    UE_LOG(LogTemp,Display,TEXT("WATER CANNON: actual shot%d from%s direction%s intangible"),ShotCount,*GetActorLocation().ToString(),*Direction.ToString());
}
void AReferenceWaterCannon::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* Hero=UGameplayStatics::GetPlayerPawn(this,0);if(!Hero)return;
    const float Distance=FVector::Dist(Hero->GetActorLocation(),GetActorLocation());
    if(!bVisible&&Distance<2000)bVisible=true;else if(bVisible&&Distance>2500){bVisible=false;Clock=0;}
    for(UStaticMeshComponent* Part:{Body.Get(),Rim.Get(),Barrel.Get(),Muzzle.Get()})Part->SetVisibility(bVisible);
    if(bVisible&&bCanFire)
    {
        Clock+=DeltaSeconds;
        if(Clock<2)
        {AimYaw=FMath::FInterpConstantTo(AimYaw,TargetYaw,DeltaSeconds,42.1875f);AimPitch=FMath::FInterpConstantTo(AimPitch,TargetPitch,DeltaSeconds,21.09375f);}
        else if(Clock-DeltaSeconds<2)Fire();
        if(Clock>=130.f/30)
        {Clock=0;TargetYaw=-45+22.5f*(ShotCount%5);TargetPitch=22.5f+5.625f*(ShotCount%4);}
    }
    UpdateBarrel();
    if(bShotActive)
    {
        ShotAge+=DeltaSeconds;ShotPosition+=ShotVelocity*DeltaSeconds;Shot->SetWorldLocation(ShotPosition);
        const float Stretch=FMath::Max(1.2f,1.7f-ShotAge*3);Shot->SetWorldScale3D(FVector(2-Stretch,2-Stretch,Stretch)*1.6f);
        // 一次資料と同じく大砲の水玉は見た目だけで、当たり判定や被弾の依頼をしません。
        if(ShotAge>100.f/30){bShotActive=false;Shot->SetVisibility(false);}
    }
}
