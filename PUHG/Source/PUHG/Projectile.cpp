#include "Projectile.h"
#include "PUHGCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Sound/SoundCue.h"
#include "ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

AProjectile::AProjectile() {
	ConstructorHelpers::FObjectFinder<UStaticMesh> ProjectileObject(L"StaticMesh'/Game/ProjectilesPack/Meshes/Projectiles/Bullets/02/SM_Bullet_02_1.SM_Bullet_02_1'");
	ConstructorHelpers::FObjectFinder<UParticleSystem> ImpactParticleObject(L"ParticleSystem'/Game/ProjectilesPack/Particles/Bullet/P_BulletImpact.P_BulletImpact'");
	ConstructorHelpers::FObjectFinder<UParticleSystem> TrailParticleObject(L"ParticleSystem'/Game/ProjectilesPack/Particles/Effects/P_Smoke_Trail.P_Smoke_Trail'");
	ConstructorHelpers::FObjectFinder<USoundCue> ImpactSoundObject(L"SoundCue'/Game/ProjectilesPack/Sounds/Common/BulletImpact_Cue.BulletImpact_Cue'");

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(L"Collision Component");
	CollisionComponent->SetSphereRadius(1.5f);
	CollisionComponent->SetWorldScale3D(FVector(1.f));
	CollisionComponent->BodyInstance.SetCollisionProfileName(L"Projectile");
	CollisionComponent->OnComponentHit.AddDynamic(this, &AProjectile::OnHitComponent);

	CollisionComponent->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComponent->CanCharacterStepUpOn = ECB_No;

	RootComponent = CollisionComponent;

	Projectile = CreateDefaultSubobject<UStaticMeshComponent>(L"Projectile Component");
	if (ProjectileObject.Succeeded()) {
		Projectile->SetStaticMesh(ProjectileObject.Object);
		Projectile->SetRelativeLocation(FVector(-15.f, 0.f, 0.f));
		Projectile->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
		Projectile->SetWorldScale3D(FVector(1.f));
	}
	Projectile->SetupAttachment(RootComponent);

	if (ImpactParticleObject.Succeeded()) {
		ImpactParticle = ImpactParticleObject.Object;
	}

	if (TrailParticleObject.Succeeded()) {
		ProjectileTrail = CreateDefaultSubobject<UParticleSystemComponent>(L"Projectile Trail Particle");
		ProjectileTrail->SetTemplate(TrailParticleObject.Object);
		ProjectileTrail->SetRelativeLocation(FVector(-2.f, 0.f, -0.5f));
		ProjectileTrail->SetRelativeScale3D(FVector(0.02f, 0.02f, 4.f));
		ProjectileTrail->SecondsBeforeInactive = 0.5f;
		
		ProjectileTrail->SetupAttachment(Projectile);
	}

	if (ImpactSoundObject.Succeeded()) {
		HitSound = ImpactSoundObject.Object;
	}

	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(L"Radial Force");
	RadialForceComponent->bAutoActivate = true;
	RadialForceComponent->Radius = 50.f;
	RadialForceComponent->ImpulseStrength = 10.f;
	RadialForceComponent->DestructibleDamage = 10.f;
	RadialForceComponent->SetupAttachment(RootComponent);


	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(L"Projectile Movement");
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	InitialLifeSpan = 5.f;
	SetReplicates(true);
	SetReplicateMovement(true);
}

void AProjectile::SetOwnerPlayerName(const FString& OwnerName) {
	OwnerPlayerName = OwnerName;
}

void AProjectile::OnHitComponent(UPrimitiveComponent * HitComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit) {
	RadialForceComponent->FireImpulse();
	if (OtherActor && OtherActor != this && OtherComp) {
		if (OtherComp->GetCollisionProfileName() != L"Ragdoll") {
			if (OtherComp->IsSimulatingPhysics()) {
				OtherComp->AddImpulseAtLocation(GetVelocity() * 100.f, GetActorLocation());
			}
		}
		if (Role == ROLE_Authority) {
			if (OtherComp->GetCollisionObjectType() == ECC_Pawn) {
				APUHGCharacter* HitCharacter = Cast<APUHGCharacter>(OtherActor);
				if (HitCharacter) {
					HitCharacter->ApplyDamageOnServer(5.f, OwnerPlayerName);
				}
			}
			SpawnParticleAndSoundOnServer(Hit);
			Destroy();
		}
	}
}

void AProjectile::SpawnParticleAndSoundOnServer_Implementation(const FHitResult & Hit) {
	SpawnParticleAndSoundOnMultiCast(Hit);
}

bool AProjectile::SpawnParticleAndSoundOnServer_Validate(const FHitResult & Hit) {
	return true;
}

void AProjectile::SpawnParticleAndSoundOnMultiCast_Implementation(const FHitResult & Hit) {
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, Hit.ImpactPoint, ImpactParticleDefaultRotation + Hit.ImpactNormal.Rotation(), FVector(2.5f));
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, Hit.ImpactPoint);
}

bool AProjectile::SpawnParticleAndSoundOnMultiCast_Validate(const FHitResult & Hit) {
	return true;
}