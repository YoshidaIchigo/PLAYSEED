#include "ThunderBoundSentinel.h"
#include "ThunderPrisonGate.h"
#include "ReferenceBattlefieldTerrain.h"
#include "WindseedCharacter.h"
#include "PlayseedVitalityComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AThunderBoundSentinel::AThunderBoundSentinel()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostPhysics;
    bReference=AReferenceBattlefieldTerrain::IsReferenceCourse();
    if(bReference){LeashRadius=900;BodyHeight=228;}
    Body = CreateDefaultSubobject<USphereComponent>(TEXT("SentinelBody")); RootComponent = Body;
    Body->InitSphereRadius(bReference?180:85); Body->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Body->SetCollisionObjectType(ECC_WorldDynamic); Body->SetCollisionResponseToAllChannels(ECR_Ignore);
    Body->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    auto Part = [&](const TCHAR* Name, UStaticMesh* Mesh, FVector At, FVector Size)
    {
        auto* Shape = CreateDefaultSubobject<UStaticMeshComponent>(Name); Shape->SetupAttachment(Body);
        Shape->SetStaticMesh(Mesh); Shape->SetRelativeLocation(At); Shape->SetRelativeScale3D(Size / 100);
        Shape->SetCollisionEnabled(ECollisionEnabled::NoCollision); Parts.Add(Shape); return Shape;
    };
    if(bReference)
    {
        // 黒い球、開いた口、白い歯と目を同梱形状で組み、鎖は4つの金属球にします。
        Part(TEXT("ReferenceBlackBody"),Sphere.Object,FVector::ZeroVector,FVector(360));
        Part(TEXT("ReferenceMouth"),Sphere.Object,FVector(181,0,-40),FVector(12,245,150));
        for(int32 Side:{-1,1})
        {
            Part(*FString::Printf(TEXT("ReferenceEyeWhite%d"),Side),Sphere.Object,FVector(154,Side*82,70),FVector(45,65,88));
            Part(*FString::Printf(TEXT("ReferencePupil%d"),Side),Sphere.Object,FVector(178,Side*82,70),FVector(12,30,50));
        }
        for(int32 I=0;I<5;++I)
        {
            auto* Upper=Part(*FString::Printf(TEXT("ReferenceToothUpper%d"),I),Cone.Object,FVector(190,(I-2)*42,12),FVector(28,45,55));
            Upper->SetRelativeRotation(FRotator(180,0,0));
            Part(*FString::Printf(TEXT("ReferenceToothLower%d"),I),Cone.Object,FVector(190,(I-2)*42,-94),FVector(28,45,55));
        }
    }
    else
    {
    Part(TEXT("MossBack"), Sphere.Object, FVector::ZeroVector, FVector(170, 120, 110));
    Part(TEXT("CopperFace"), Cube.Object, FVector(80, 0, 0), FVector(55, 85, 60));
    Part(TEXT("LeftEar"), Cone.Object, FVector(55, -34, 58), FVector(35, 35, 65));
    Part(TEXT("RightEar"), Cone.Object, FVector(55, 34, 58), FVector(35, 35, 65));
    for (int32 I = 0; I < 4; ++I) Part(*FString::Printf(TEXT("CopperFoot%d"), I), Cube.Object, FVector(I < 2 ? 48 : -48, I % 2 ? 43 : -43, -77.5f), FVector(30, 25, 45));
    Part(TEXT("LeftAmberEye"), Sphere.Object, FVector(110, -23, 12), FVector(12));
    Part(TEXT("RightAmberEye"), Sphere.Object, FVector(110, 23, 12), FVector(12));
    }
    Warning = Part(TEXT("LockedGroundWarning"), Cylinder.Object, FVector::ZeroVector, FVector(180, 180, 4));
    AnchorPlate = Part(TEXT("TetherAnchorPlate"), Cylinder.Object, FVector::ZeroVector, FVector(110, 110, 20));
    for (int32 I = 0; I < (bReference?4:8); ++I) Links.Add(Part(*FString::Printf(TEXT("CopperTether%d"), I), Sphere.Object, FVector::ZeroVector, FVector(bReference?40:19)));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Plane(TEXT("/Engine/BasicShapes/Plane.Plane"));
    Shadow=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChompShadow"));Shadow->SetupAttachment(Body);Shadow->SetStaticMesh(Plane.Object);
    Shadow->SetAbsolute(true,true,true);Shadow->SetCollisionEnabled(ECollisionEnabled::NoCollision);Shadow->SetCastShadow(false);
    Shadow->SetWorldScale3D(FVector(4.2f,4.2f,1));
}

void AThunderBoundSentinel::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, bReference?TEXT("/Game/Battlefield/M_Actor.M_Actor"):TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        for (int32 I = 0; I < Parts.Num(); ++I)
        {
            auto* Tint = UMaterialInstanceDynamic::Create(Palette, this);
            FLinearColor Color = I == 0 ? FLinearColor(.15f, .35f, .28f) : I == 2 || I == 3 ? FLinearColor(.03f, .55f, .47f) : I == 8 || I == 9 || I == 10 ? FLinearColor(1, .65f, .12f) : FLinearColor(.55f, .28f, .12f);
            if(bReference)
            {
                const FString Name=Parts[I]->GetName();
                Color=Name.Contains(TEXT("EyeWhite"))||Name.Contains(TEXT("Tooth"))?FLinearColor(.94f,.94f,.9f):Name.Contains(TEXT("Mouth"))?FLinearColor(.55f,.025f,.035f):Name.Contains(TEXT("Tether"))?FLinearColor(.3f,.32f,.35f):FLinearColor(.012f,.014f,.02f);
            }
            Tint->SetVectorParameterValue(TEXT("Tint"), Color); Parts[I]->SetMaterial(0, Tint); if (I == 0) BodyTint = Tint;
        }
    }
    Warning->SetVisibility(false);
    AnchorPlate->SetVisibility(!bReference);Shadow->SetVisibility(bReference);
    if(bReference)if(auto* Material=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Battlefield/M_Shadow.M_Shadow")))Shadow->SetMaterial(0,Material);
}

void AThunderBoundSentinel::Configure(FVector GroundAnchor) { Anchor = GroundAnchor; ResetSentinel(); }
void AThunderBoundSentinel::EnterState(EState Next)
{
    State = Next; StateTime = 0; Warning->SetVisibility(!bReference&&State == EState::Windup);
    if (BodyTint) BodyTint->SetVectorParameterValue(TEXT("Tint"), bReference?FLinearColor(.012f,.014f,.02f):State == EState::Windup ? FLinearColor(.7f, .5f, .15f) : FLinearColor(.15f, .35f, .28f));
}
void AThunderBoundSentinel::ResetSentinel()
{
    ++MotionRevision; PatrolTime = 0; LockedTarget = FVector::ZeroVector;
    ReleaseGate.Reset(); SetActorHiddenInGame(false); Body->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    for (UStaticMeshComponent* Link : Links) Link->SetVisibility(true);
    SetActorLocation(GetHomeLocation()); SetActorRotation(FRotator::ZeroRotator); EnterState(EState::Patrol); UpdateTether();
}
void AThunderBoundSentinel::UpdateTether()
{
    if(bReference)
    {
        FHitResult Floor;FCollisionQueryParams Params(SCENE_QUERY_STAT(ChompShadow),true,this);
        if(GetWorld()->LineTraceSingleByChannel(Floor,GetActorLocation(),GetActorLocation()-FVector(0,0,1500),ECC_Visibility,Params))
        {Shadow->SetWorldLocation(Floor.ImpactPoint+Floor.ImpactNormal*1.5f);Shadow->SetWorldRotation(FRotationMatrix::MakeFromZ(Floor.ImpactNormal).Rotator());}
    }
    AnchorPlate->SetWorldLocation(Anchor + FVector(0, 0, 10)); AnchorPlate->SetWorldRotation(FRotator::ZeroRotator);
    const FVector Start = Anchor + FVector(0, 0, 65), End = GetActorLocation() - GetActorForwardVector() * 70;
    for (int32 I = 0; I < Links.Num(); ++I) Links[I]->SetWorldLocation(FMath::Lerp(Start, End, (I + 1.f) / (Links.Num() + 1.f)));
}
void AThunderBoundSentinel::CheckContact(FVector Start, FVector End, AWindseedCharacter* Player)
{
    if (!Player) return;
    const auto* Capsule = Player->GetCapsuleComponent();
    const float Radius = Capsule->GetScaledCapsuleRadius(), AxisHalf = Capsule->GetScaledCapsuleHalfHeight() - Radius;
    const FVector Center = Player->GetActorLocation(); FVector EnemyPoint, HeroPoint;
    const FVector ContactOffset(0,0,bReference?115:0);
    FMath::SegmentDistToSegmentSafe(Start-ContactOffset,End-ContactOffset,Center-FVector(0,0,AxisHalf),Center+FVector(0,0,AxisHalf),EnemyPoint,HeroPoint);
    if (FVector::DistSquared(EnemyPoint, HeroPoint) <= FMath::Square((bReference?80:85) + Radius))
        if (Player->GetVitality()->TryTakeImpact(bReference?3:2, Center - EnemyPoint)) ++ConfirmedHits;
}
void AThunderBoundSentinel::MoveBounded(FVector Desired, AWindseedCharacter* Player)
{
    // 行動範囲は敵の位置にだけ適用します。プレイヤーの位置や入力は変更しません。
    Desired = Anchor + (Desired - Anchor).GetClampedToMaxSize2D(LeashRadius); Desired.Z = Anchor.Z + BodyHeight;
    const int32 Revision = MotionRevision; const FVector Start = GetActorLocation();
    SetActorLocation(Desired, true); CheckContact(Start, GetActorLocation(), Player);
    if (MotionRevision != Revision) return; // 致命傷によるRが、この古いTickに上書きされるのを防ぎます。
    MaxObservedReach = FMath::Max(MaxObservedReach, FVector::Dist2D(GetActorLocation(), Anchor));
}
void AThunderBoundSentinel::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)); if (!Player) return;
    const int32 Revision = MotionRevision; StateTime += DeltaSeconds; PatrolTime += DeltaSeconds;
    if (State == EState::Departed) return;
    if (State == EState::Stagger) { if (StateTime >= 1.4f) EnterState(EState::Return); UpdateTether(); return; }
    if (State == EState::ReleaseWindup)
    {
        // 参照版は解放後に4回跳ねてから格子へ進みます。敵自身の演出で、プレイヤーは動かしません。
        if(bReference&&StateTime>.9f)
        {const float Hop=FMath::Abs(FMath::Sin((StateTime-.9f)*PI/.6f))*100;FVector At=GetActorLocation();At.Z=Anchor.Z+BodyHeight+Hop;SetActorLocation(At,true);}
        if(StateTime>=(bReference?3.3f:.9f))EnterState(EState::Released);UpdateTether();return;
    }
    if (State == EState::Released)
    {
        if (!ReleaseGate.IsValid()) { ResetSentinel(); return; }
        FVector Goal = ReleaseGate->GetActorLocation() + FVector(80, 0, -100);
        if(bReference){Goal=ReleaseGate->GetActorLocation()+ReleaseGate->GetActorForwardVector()*80;Goal.Z=Anchor.Z+BodyHeight;}
        const FVector Start = GetActorLocation(), Offset = Goal - Start; FHitResult Hit;
        SetActorRotation(Offset.GetSafeNormal2D().Rotation());
        SetActorLocation(FMath::VInterpConstantTo(GetActorLocation(), Goal, DeltaSeconds, 700), true, &Hit);
        if (Hit.bBlockingHit) ReleaseGate->HandleReleasedImpact(this, Hit);
        CheckContact(Start, GetActorLocation(), Player); if (Revision != MotionRevision) return;
        if (FVector::Dist2D(GetActorLocation(), Goal) < 5) { EnterState(EState::Departed); SetActorHiddenInGame(true); Body->SetCollisionEnabled(ECollisionEnabled::NoCollision); }
        return;
    }
    const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
    if (State == EState::Patrol)
    {
        if (ToPlayer.Size2D() < (bReference?1300:700) && FMath::Abs(ToPlayer.Z) < 180)
        {
            LockedTarget = Player->GetActorLocation(); LockedTarget.Z = Anchor.Z + BodyHeight;
            LungeDirection = (LockedTarget - GetActorLocation()).GetSafeNormal2D();
            SetActorRotation(LungeDirection.Rotation()); EnterState(EState::Windup);
            Warning->SetWorldLocation(FVector(LockedTarget.X, LockedTarget.Y, Anchor.Z + 3)); Warning->SetWorldRotation(FRotator::ZeroRotator);
        }
        else MoveBounded(FMath::VInterpConstantTo(GetActorLocation(), GetHomeLocation() + FVector(0, FMath::Sin(PatrolTime) * 60, 0), DeltaSeconds, 80), Player);
    }
    else if (State == EState::Windup && StateTime >= .9f) { ++LungeCount; EnterState(EState::Lunge); UE_LOG(LogTemp, Display, TEXT("SENTINEL: actual lunge %d toward locked %s."), LungeCount, *LockedTarget.ToString()); }
    else if (State == EState::Lunge)
    {
        MoveBounded(GetActorLocation() + LungeDirection * (bReference?1800:950) * DeltaSeconds, Player); if (Revision != MotionRevision) return;
        if (StateTime >= (bReference?.7f:.35f) || FVector::Dist2D(GetActorLocation(), Anchor) >= LeashRadius-.5f) EnterState(EState::Recover);
    }
    else if (State == EState::Recover && StateTime > 1.1f) EnterState(EState::Return);
    else if (State == EState::Return)
    {
        MoveBounded(FMath::VInterpConstantTo(GetActorLocation(), GetHomeLocation(), DeltaSeconds, 180), Player); if (Revision != MotionRevision) return;
        if (FVector::Dist2D(GetActorLocation(), GetHomeLocation()) < 2) EnterState(EState::Patrol);
    }
    if (Revision != MotionRevision) return;
    CheckContact(GetActorLocation(), GetActorLocation(), Player); if (Revision != MotionRevision) return;
    UpdateTether();
}

void AThunderBoundSentinel::StaggerFromPost() { ++MotionRevision; EnterState(EState::Stagger); }
void AThunderBoundSentinel::ReleaseToward(AThunderPrisonGate* InGate)
{
    if (!InGate || IsReleased()) return;
    ++MotionRevision; ReleaseGate = InGate; EnterState(EState::ReleaseWindup);
    for (UStaticMeshComponent* Link : Links) Link->SetVisibility(false);
    UE_LOG(LogTemp, Display, TEXT("SENTINEL RELEASE: actual third post pound freed the copper tether."));
}
