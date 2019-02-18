#include "HomingProjectile.h"
#include "PUHGCharacter.h"
#include "ExplosionActorComponent.h"
#include "Rocket.h"
#include "Components/SphereComponent.h"
#include "Components/ChildActorComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ConstructorHelpers.h"
#include "Engine/World.h"

AHomingProjectile::AHomingProjectile() {
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(L"Collision Component");
	CollisionComponent->BodyInstance.SetCollisionProfileName(L"HomingMissle");
	CollisionComponent->OnComponentHit.AddDynamic(this, &AHomingProjectile::OnHitComponent);
	CollisionComponent->SetSphereRadius(5.f);
	RootComponent = CollisionComponent;

	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(L"RadialForce Component");
	RadialForceComponent->bAutoActivate = true;
	RadialForceComponent->Radius = 300.f;
	RadialForceComponent->ImpulseStrength = 1000.f;
	RadialForceComponent->DestructibleDamage = 100.f;
	
	RadialForceComponent->SetupAttachment(RootComponent);

	RocketChildComponent = CreateDefaultSubobject<UChildActorComponent>(L"Child Component");
	RocketChildComponent->SetChildActorClass(ARocket::StaticClass());
	RocketChildComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	RocketChildComponent->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	RocketChildComponent->SetRelativeScale3D(FVector(1.f));
	RocketChildComponent->SetupAttachment(RootComponent);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(L"Projectile Movement Component");
	ProjectileMovementComponent->InitialSpeed = 4000.f;
	ProjectileMovementComponent->MaxSpeed = 4000.f;
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bInitialVelocityInLocalSpace = true;
	ProjectileMovementComponent->Velocity = FVector(4000.f, 0.f, 0.f);

	ExplosionComponent = CreateDefaultSubobject<UExplosionActorComponent>(L"Explosion Component");

	InitialLifeSpan = 20.f;

	SetReplicates(true);
	SetReplicateMovement(true);
	PrimaryActorTick.bCanEverTick = true;
}

void AHomingProjectile::SetHomingTarget(USceneComponent* Target) {
	ProjectileMovementComponent->HomingTargetComponent = Target;
	ProjectileMovementComponent->bIsHomingProjectile = true;
	ProjectileMovementComponent->HomingAccelerationMagnitude = 5000.f;
	this->Target = Target;
}

void AHomingProjectile::SetOwnerPlayerName(const FString & OwnerName) {
	OwnerPlayerName = OwnerName;
}

void AHomingProjectile::BeginPlay() {
	Super::BeginPlay();

	StartLocation = CollisionComponent->GetComponentLocation();
}

void AHomingProjectile::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

}

void AHomingProjectile::OnHitComponent(UPrimitiveComponent * HitComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit) {
	RadialForceComponent->FireImpulse();
	if (OtherActor && OtherActor != this && OtherComp) {
		HitResultSave = Hit;
		if (Role == ROLE_Authority) {
			if (RocketChildComponent->GetChildActor()) {
				RocketChildComponent->GetChildActor()->Destroy();
			}
			UGameplayStatics::ApplyRadialDamage(GetWorld(), 10.f, GetActorLocation(), RadialForceComponent->Radius, UDamageType::StaticClass(), TArray<AActor*>(), this);
			ExplosionComponent->Excute(RadialForceComponent->GetComponentLocation());
			Destroy();
		}
	}
}