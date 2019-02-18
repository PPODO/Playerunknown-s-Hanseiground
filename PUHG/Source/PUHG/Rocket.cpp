#include "Rocket.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "ConstructorHelpers.h"

ARocket::ARocket() {
	ConstructorHelpers::FObjectFinder<UStaticMesh> RocketObject(L"StaticMesh'/Game/ProjectilesPack/Meshes/Projectiles/Rockets/SM_Rocket_01.SM_Rocket_01'");
	ConstructorHelpers::FObjectFinder<UParticleSystem> FlareObject(L"ParticleSystem'/Game/ProjectilesPack/Particles/Effects/P_Flare.P_Flare'");
	ConstructorHelpers::FObjectFinder<UParticleSystem> SmokeObject(L"ParticleSystem'/Game/ProjectilesPack/Particles/Effects/P_Smoke_Trail.P_Smoke_Trail'");

	DefaultScene = CreateDefaultSubobject<USceneComponent>(L"Scene Component");
	RootComponent = DefaultScene;

	Rocket = CreateDefaultSubobject<UStaticMeshComponent>(L"Rocket Mesh");
	Rocket->SetWorldLocation(FVector(-2.5, 0., 0.f));
	Rocket->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));
	Rocket->SetWorldScale3D(FVector(0.15f));
	Rocket->SetCollisionProfileName(L"NoCollision");
	Rocket->SetupAttachment(RootComponent);

	FlareParticle = CreateDefaultSubobject<UParticleSystemComponent>(L"Flare Particle");
	FlareParticle->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	FlareParticle->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	FlareParticle->SetRelativeScale3D(FVector(1.f));
	FlareParticle->SetupAttachment(RootComponent);

	SmokeTrailParticle = CreateDefaultSubobject<UParticleSystemComponent>(L"Smoke Trail Particle");
	SmokeTrailParticle->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	SmokeTrailParticle->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	SmokeTrailParticle->SetRelativeScale3D(FVector(0.5f));
	SmokeTrailParticle->SetupAttachment(RootComponent);

	if (RocketObject.Succeeded()) {
		Rocket->SetStaticMesh(RocketObject.Object);
	}
	if (FlareObject.Succeeded()) {
		FlareParticle->SetTemplate(FlareObject.Object);
	}
	if (SmokeObject.Succeeded()) {
		SmokeTrailParticle->SetTemplate(SmokeObject.Object);
	}
}