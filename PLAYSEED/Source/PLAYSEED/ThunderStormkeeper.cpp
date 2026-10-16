#include "ThunderStormkeeper.h"
#include "PlayseedLantern.h"
#include "ReferenceBattlefieldTerrain.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AThunderStormkeeper::AThunderStormkeeper()
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
    bReferenceBoss = AReferenceBattlefieldTerrain::IsReferenceCourse();
    GetBody()->SetStaticMesh(Sphere.Object);
    if (bReferenceBoss)
    {
        // 親の球は直径400cm。子の位置・大きさにも親の4倍スケールが掛かります。
        static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
        GetBody()->SetRelativeScale3D(FVector(4));
        const FLinearColor White(.95f,.94f,.85f), Black(.015f,.018f,.025f), Gold(.95f,.57f,.04f);
        for (float Side : {-1.f, 1.f})
        {
            AddPart(Sphere.Object,FVector(46,Side*16,15),FVector(.1f,.1f,.16f),White);
            AddPart(Sphere.Object,FVector(50,Side*16,15),FVector(.035f,.045f,.1f),Black);
            AddPart(Sphere.Object,FVector(48,Side*9,-1),FVector(.11f,.2f,.06f),White);
            AddPart(Sphere.Object,FVector(0,Side*47,-8),FVector(.23f),White);
            AddPart(Sphere.Object,FVector(8,Side*22,-41),FVector(.35f,.27f,.16f),Gold);
        }
        AddPart(Cylinder.Object,FVector(0,0,50),FVector(.65f,.65f,.13f),Gold);
        for (int32 Index=0; Index<5; ++Index)
        {
            const float Angle=Index*2*PI/5;
            AddPart(Cone.Object,FVector(FMath::Cos(Angle)*24,FMath::Sin(Angle)*24,63),FVector(.2f,.2f,.34f),Gold);
        }
        return;
    }
    GetBody()->SetRelativeScale3D(FVector(1.8f));
    // 子の形は見た目専用です。物理は親の球一つに集約し、角の引っ掛かりを防ぎます。
    AddPart(Cone.Object, FVector(0, 0, 55), FVector(.65f, .65f, .65f), FLinearColor(.8f, .43f, .12f));
    AddPart(Sphere.Object, FVector(46, -22, 12), FVector(.11f, .11f, .15f), FLinearColor(.04f, .08f, .13f));
    AddPart(Sphere.Object, FVector(46, 22, 12), FVector(.11f, .11f, .15f), FLinearColor(.04f, .08f, .13f));
}
UStaticMeshComponent* AThunderStormkeeper::AddPart(UStaticMesh* Mesh, FVector Position, FVector Scale, FLinearColor Color)
{
    auto* Part = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("KeeperPart%d"), Parts.Num()));
    Part->SetupAttachment(GetBody()); Part->SetStaticMesh(Mesh); Part->SetRelativeLocation(Position); Part->SetRelativeScale3D(Scale);
    Part->SetCollisionEnabled(ECollisionEnabled::NoCollision); Parts.Add(Part); PartColors.Add(Color); return Part;
}
void AThunderStormkeeper::PaintBody()
{
    auto* Palette = LoadObject<UMaterialInterface>(nullptr, bReferenceBoss?TEXT("/Game/Battlefield/M_Actor.M_Actor"):TEXT("/Game/Windseed/M_Palette.M_Palette"));
    if (!Palette) return;
    auto* BodyMaterial = UMaterialInstanceDynamic::Create(Palette, this);
    BodyMaterial->SetVectorParameterValue(TEXT("Tint"), (bReferenceBoss?FLinearColor(.035f,.04f,.055f):FLinearColor(.42f, .25f, .65f))); GetBody()->SetMaterial(0, BodyMaterial);
    for (int32 Index = 0; Index < Parts.Num(); ++Index)
    {
        auto* Material = UMaterialInstanceDynamic::Create(Palette, this);
        Material->SetVectorParameterValue(TEXT("Tint"), PartColors[Index]); Parts[Index]->SetMaterial(0, Material);
    }
}
void AThunderStormkeeper::BeginPlay()
{
    Super::BeginPlay(); ArenaFloor = GetActorLocation() - FVector(0, 0, bReferenceBoss?200:100);
    GetBody()->SetSimulatePhysics(false); GetBody()->SetMassOverrideInKg(NAME_None, 80, true); PaintBody();
    if(bReferenceBoss)AlignWithFloor();
}
bool AThunderStormkeeper::CanBePickedUp(AWindseedCharacter* Player) const
{
    // 正面からの E も TryPickup まで届かせるため、方向はここでは除外しません。
    if (bReferenceBoss)
        return !bDefeated&&!bRecovering&&Player&&!IsHeld()&&!IsInThrownFlight()
            &&Player->GetCharacterMovement()->IsMovingOnGround()
            &&FVector::Dist(Player->GetActorLocation(),GetActorLocation())<=340;
    return !bDefeated && !bRecovering && Super::CanBePickedUp(Player);
}
bool AThunderStormkeeper::TryPickup(AWindseedCharacter* Player)
{
    if (!CanBePickedUp(Player)) return false;
    const FVector TowardPlayer = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
    if (FVector::DotProduct(TowardPlayer, GetActorForwardVector()) > -.35f)
    {
        if (Player->GetVitality()->TryTakeImpact(2, Player->GetActorLocation() - GetActorLocation())) ++FrontCounters;
        return false;
    }
    return Super::TryPickup(Player);
}
FVector AThunderStormkeeper::GetCarryOffset() const
{
    // 大きい相手だけ頭上に持ち上げ、通常の箱を持つ位置は保ちます。
    return bReferenceBoss?FVector(0,0,300):FVector(150,0,90);
}
void AThunderStormkeeper::AlignWithFloor()
{
    auto* Terrain=Cast<AReferenceBattlefieldTerrain>(UGameplayStatics::GetActorOfClass(this,AReferenceBattlefieldTerrain::StaticClass()));
    FVector Floor;
    if(!Terrain||!Terrain->FindFloor(GetActorLocation()-FVector(0,0,200),Floor))return;
    // 見た目の半径ではなく実Boundsで底を合わせ、床との微小な重なりでSweepが止まるのを避けます。
    const float Radius=GetBody()->Bounds.BoxExtent.Z;
    FVector Location=GetActorLocation();Location.Z=Floor.Z+Radius+3;
    UE_LOG(LogTemp,Display,TEXT("REFERENCE BOSS: floor alignment rootZ %.2f -> %.2f, radius %.2f, floor %.2f"),GetActorLocation().Z,Location.Z,Radius,Floor.Z);
    SetActorLocation(Location);ArenaFloor.Z=Floor.Z;
}
bool AThunderStormkeeper::CanStandAt(FVector Position)
{
    auto* Terrain=Cast<AReferenceBattlefieldTerrain>(UGameplayStatics::GetActorOfClass(this,AReferenceBattlefieldTerrain::StaticClass()));
    if(!Terrain)return false;
    // 球の中心だけでなく四方の足元を調べ、崖の先まで追い掛けないようにします。
    const FVector Samples[]={FVector::ZeroVector,FVector(210,0,0),FVector(-210,0,0),FVector(0,210,0),FVector(0,-210,0)};
    for(const auto& Offset:Samples)
    {
        FVector Floor;
        // 地形自身の歩行面を使い、相手や持ち物を床として拾わないようにします。
        const bool bFloorFound=Terrain->FindFloor(Position+Offset-FVector(0,0,200),Floor);
        if(!bFloorFound||FMath::Abs(Floor.Z-ArenaFloor.Z)>25)
        {
            if(!bFloorBlockReported)
            {
                UE_LOG(LogTemp,Display,TEXT("REFERENCE BOSS: chase floor rejected at %s offset%s found%d floor%s homeZ%.2f"),*Position.ToString(),*Offset.ToString(),bFloorFound,*Floor.ToString(),ArenaFloor.Z);
                bFloorBlockReported=true;
            }
            return false;
        }
    }
    return true;
}
FVector AThunderStormkeeper::GetThrowVelocity(const AWindseedCharacter* Player) const
{
    return Player->GetActorForwardVector() * 520 + FVector(0, 0, 300);
}
void AThunderStormkeeper::BeginRecovery(bool bOffArena)
{
    if (bRecovering || bDefeated) return;
    if (bOffArena) ++OffArenaReturns;
    bRecovering = true; RecoveryTime = 1;
    bRecoverInPlace=bReferenceBoss&&!bOffArena;
    if(bRecoverInPlace)
    {
        // 有効な投げは実際に着地した場所で起き上がります。元の位置へ瞬間移動させません。
        GetBody()->SetSimulatePhysics(false);GetBody()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        SetActorRotation(FRotator(0,GetActorRotation().Yaw,0));AlignWithFloor();return;
    }
    // 敵だけを戻す前に Carryable の保持・投擲状態を解除します。プレイヤーは移動しません。
    APlayseedCarryable::ResetToSpawn(); GetBody()->SetSimulatePhysics(false); GetBody()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void AThunderStormkeeper::ReturnToArena()
{
    if(!bRecoverInPlace) APlayseedCarryable::ResetToSpawn();
    GetBody()->SetSimulatePhysics(false);GetBody()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    if(bReferenceBoss)AlignWithFloor();
    bRecovering=false;bRecoverInPlace=false;RecoveryTime=0;
}
void AThunderStormkeeper::OnThrownLanding(const FHitResult& Hit)
{
    if (bDefeated || bRecovering) return;
    const bool bOnSummit = bReferenceBoss
        ? Hit.ImpactNormal.Z>.9f&&FMath::Abs(Hit.ImpactPoint.Z-4293)<25&&FVector::Dist2D(Hit.ImpactPoint,FVector(2800,5000,4293))<1800
        : FMath::Abs(Hit.ImpactPoint.Z - ArenaFloor.Z) < 20 && FVector::Dist2D(GetActorLocation(), ArenaFloor) <= 550;
    if (!bOnSummit) { BeginRecovery(true); return; }
    ++ValidThrows;
    UE_LOG(LogTemp, Display, TEXT("STORMKEEPER: physical summit landing %d/3 at %s"), ValidThrows, *Hit.ImpactPoint.ToString());
    if (ValidThrows < 3) { BeginRecovery(false); return; }
    bDefeated = true; GetBody()->SetSimulatePhysics(false); SetActorEnableCollision(false); SetActorHiddenInGame(true);
    // 報酬は World が所有します。ここは弱参照だけを持ち、R で自分の未取得物だけを片付けます。
    Reward = GetWorld()->SpawnActor<APlayseedLantern>(bReferenceBoss?FVector(2000,4500,4500):ArenaFloor + FVector(0, 0, 100), FRotator::ZeroRotator);
    if (Reward.IsValid()) Reward->ConfigureReward(1);
}
void AThunderStormkeeper::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bDefeated || IsHeld()) return;
    if (bRecovering) { RecoveryTime -= DeltaSeconds; if (RecoveryTime <= 0) ReturnToArena(); return; }
    if (IsInThrownFlight()) { if (GetActorLocation().Z < ArenaFloor.Z - 500) BeginRecovery(true); return; }
    // E 投擲以外の落下・ダメージによる手放しには得点を与えません。
    if (GetBody()->IsSimulatingPhysics()) { BeginRecovery(false); return; }
    auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)); if (!Player) return;
    const FVector Offset = Player->GetActorLocation() - GetActorLocation();
    const float Feet = Player->GetActorLocation().Z - 82;
    if (FMath::Abs(Feet - ArenaFloor.Z) > 180 || Offset.Size2D() > 900) return;
    FRotator Rotation = GetActorRotation(); Rotation.Yaw = FMath::FixedTurn(Rotation.Yaw, Offset.Rotation().Yaw, (bReferenceBoss?42:25) * DeltaSeconds); SetActorRotation(Rotation);
    if (Offset.Size2D() > (bReferenceBoss?300:185))
    {
        FVector Destination = GetActorLocation() + Offset.GetSafeNormal2D() * 90 * DeltaSeconds;
        if(bReferenceBoss)
        {
            if(CanStandAt(Destination))
            {
                const FVector Before=GetActorLocation();FHitResult Sweep;
                SetActorLocation(Destination,true,&Sweep);
                if(!bSweepBlockReported&&FVector::DistSquared(Before,GetActorLocation())<.01&&Sweep.bBlockingHit)
                {
                    UE_LOG(LogTemp,Display,TEXT("REFERENCE BOSS: chase sweep blocked at %s by%s, penetrating%d normal%s"),*Before.ToString(),*GetNameSafe(Sweep.GetActor()),Sweep.bStartPenetrating,*Sweep.ImpactNormal.ToString());
                    bSweepBlockReported=true;
                }
            }
        }
        else
        {
            const FVector Radial=(Destination-ArenaFloor).GetClampedToMaxSize2D(400);
            Destination.X=ArenaFloor.X+Radial.X;Destination.Y=ArenaFloor.Y+Radial.Y;SetActorLocation(Destination,true);
        }
    }
    if (Offset.Size2D() < (bReferenceBoss?240:140) && Feet < GetActorLocation().Z + 50) Player->GetVitality()->TryTakeImpact(2, Offset);
}
void AThunderStormkeeper::ResetToSpawn()
{
    if (Reward.IsValid()) Reward->Destroy(); Reward.Reset(); ValidThrows = FrontCounters = OffArenaReturns = 0; bDefeated = false;
    SetActorHiddenInGame(false); SetActorEnableCollision(true); bRecoverInPlace=false; ReturnToArena();
}
