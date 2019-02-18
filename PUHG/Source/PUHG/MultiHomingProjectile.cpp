#include "MultiHomingProjectile.h"
#include "HomingProjectile.h"
#include "PUHGCharacter.h"
#include "Rocket.h"
#include "Components/SceneComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/SphereComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "ConstructorHelpers.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"

AMultiHomingProjectile::AMultiHomingProjectile() {
	ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleObject(L"ParticleSystem'/Game/ProjectilesPack/Particles/Effects/P_Smoke.P_Smoke'");
	ConstructorHelpers::FObjectFinder<USoundCue> SoundObject(L"SoundCue'/Game/ProjectilesPack/Sounds/Common/Explosion_02_Cue.Explosion_02_Cue'");

	if (ParticleObject.Succeeded()) {
		ImpactParticle = ParticleObject.Object;
	}
	if (SoundObject.Succeeded()) {
		ImpactSound = SoundObject.Object;
	}

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(L"Collision Component");
	CollisionComponent->BodyInstance.SetCollisionProfileName(L"HomingMissle");
	CollisionComponent->OnComponentHit.AddDynamic(this, &AMultiHomingProjectile::OnHitComponent);
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
	ProjectileMovementComponent->InitialSpeed = 2000.f;
	ProjectileMovementComponent->MaxSpeed = 3000.f;
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bInitialVelocityInLocalSpace = true;
	ProjectileMovementComponent->Velocity = FVector(2000.f, 0.f, 0.f);

	MaxRocket = 10;
	InitialLifeSpan = 5.f;

	SetReplicates(true);
	SetReplicateMovement(true);
}

void AMultiHomingProjectile::SetHomingTarget(USceneComponent* Target) {
	ProjectileMovementComponent->HomingTargetComponent = Target;
	ProjectileMovementComponent->bIsHomingProjectile = true;
	ProjectileMovementComponent->HomingAccelerationMagnitude = 10000.f;
	this->Target = Target;
}

void AMultiHomingProjectile::SetOwnerPlayerName(const FString & OwnerName) {
	OwnerPlayerName = OwnerName;
}

void AMultiHomingProjectile::BeginPlay() {
	Super::BeginPlay();

	if (Role == ROLE_Authority) {
		GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &AMultiHomingProjectile::DestroyAndSpawnMissile, 0.5f);
	}
}

void AMultiHomingProjectile::OnHitComponent(UPrimitiveComponent * HitComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit) {
	RadialForceComponent->FireImpulse();
	if (OtherActor && OtherActor != this && OtherComp) {
		if (Role == ROLE_Authority && RocketChildComponent->GetChildActor() && OtherActor->GetName() != OwnerPlayerName) {
			SpawnParticleAndSoundOnServer();
			DestroyAndSpawnMissile();
		}
	}
}

void AMultiHomingProjectile::DestroyAndSpawnMissile() {
	if (RocketChildComponent->GetChildActor()) {
		MissileTransform = UKismetMathLibrary::MakeTransform(GetActorLocation(), CollisionComponent->GetComponentRotation(), FVector(1.f));
		for (int32 i = 0; i < MaxRocket; i++) {
			FTransform NewTransform = SetHomingDirection(i + 1);
			SpawnMultiHomingMissileOnServer(NewTransform);
		}
		if (RocketChildComponent->GetChildActor()) {
			RocketChildComponent->GetChildActor()->Destroy();
		}
		GetWorld()->GetTimerManager().ClearTimer(DestroyTimerHandle);
	}
}

FTransform AMultiHomingProjectile::SetHomingDirection(int32 Index) {
	float RotateShift = Index * 360 / MaxRocket;
	FRotator MissileRotation = MissileTransform.GetRotation().Rotator();

	FVector NewLocation(UKismetMathLibrary::RandomIntegerInRange(0, MaxRocket * 10), UKismetMathLibrary::RandomIntegerInRange(0, MaxRocket * 10), UKismetMathLibrary::RandomIntegerInRange(0, MaxRocket * 10));
	FRotator NewRotation(MissileRotation.Pitch + RotateShift, MissileRotation.Yaw + RotateShift, MissileRotation.Roll + RotateShift);

	return FTransform(NewRotation, MissileTransform.GetLocation() + NewLocation, MissileTransform.GetScale3D());
}

void AMultiHomingProjectile::SpawnParticleAndSoundOnServer_Implementation() {
	SpawnParticleAndSoundOnMultiCast();
}

bool AMultiHomingProjectile::SpawnParticleAndSoundOnServer_Validate() {
	return true;
}

void AMultiHomingProjectile::SpawnParticleAndSoundOnMultiCast_Implementation() {
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, RadialForceComponent->GetComponentLocation());
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, RadialForceComponent->GetComponentLocation());
}

bool AMultiHomingProjectile::SpawnParticleAndSoundOnMultiCast_Validate() {
	return true;
}

void AMultiHomingProjectile::SpawnMultiHomingMissileOnServer_Implementation(FTransform NewTransform) {
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AHomingProjectile* Projectile = GetWorld()->SpawnActor<AHomingProjectile>(AHomingProjectile::StaticClass(), NewTransform, Params);
	if (Target && Projectile) {
		Projectile->SetHomingTarget(Target);
		Projectile->SetOwnerPlayerName(OwnerPlayerName);
	}
}

bool AMultiHomingProjectile::SpawnMultiHomingMissileOnServer_Validate(FTransform NewTransform) {
	return true;
}