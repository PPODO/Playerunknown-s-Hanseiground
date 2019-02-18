#include "TimeBombProjectile.h"
#include "SeungChan_Character.h"
#include "TimeBombActorComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Components/SphereComponent.h"
#include "Components/PostProcessComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"
#include "ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATimeBombProjectile::ATimeBombProjectile() {
	ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleObject(L"ParticleSystem'/Game/ProjectilesPack/Particles/TimeBomb/P_TimeBomb_Explosion.P_TimeBomb_Explosion'");

	if (ParticleObject.Succeeded()) {
		ImpactParticle = ParticleObject.Object;
	}

	ForceComp = CreateDefaultSubobject<URadialForceComponent>(L"Force Component");
	ForceComp->Radius = 3000.f;
	ForceComp->ImpulseStrength = 4000.f;
	ForceComp->ForceStrength = 1.f;
	ForceComp->DestructibleDamage = 100.f;
	RootComponent = ForceComp;

	FPostProcessSettings Settings;
	Settings.BloomIntensity = 8.f;
	Settings.BloomThreshold = 5.f;
	Settings.AutoExposureMinBrightness = 2.f;
	Settings.AutoExposureMaxBrightness = 2.f;
	Settings.GrainIntensity = 0.5f;
	Settings.ColorSaturation = FVector4(0.100000f, 0.100000f, 0.100000f, 1.000000f);

	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(L"Post Process");
	
	PostProcess->Settings = Settings;
	PostProcess->bEnabled = true;
	PostProcess->Priority = 1.f;
	PostProcess->BlendWeight = 0.f;
	PostProcess->SetupAttachment(RootComponent);

	TimeBombComponent = CreateDefaultSubobject<UTimeBombActorComponent>(L"Time Bomb Actor Component");

	SetReplicates(true);
	SetReplicateMovement(true);
}

void ATimeBombProjectile::BeginPlay() {
	Super::BeginPlay();

	if (TimeBombComponent) {
		if (Cast<APUHGCharacter>(GetOwner())) {
			Cast<APUHGCharacter>(GetOwner())->SetUseTickDeltaTime(true);
		}
		SpawnParticleOnServer(GetActorLocation());
		TimeBombComponent->StartTimeDilationEnable(GetOwner());
	}
}

/* Particles */

void ATimeBombProjectile::SpawnParticleOnServer_Implementation(FVector Location) {
	SpawnParticleOnMultiCast(Location);
}

bool ATimeBombProjectile::SpawnParticleOnServer_Validate(FVector Location) {
	return true;
}

void ATimeBombProjectile::SpawnParticleOnMultiCast_Implementation(FVector Location) {
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, Location)->SetRelativeScale3D(RelativeScale);
}

bool ATimeBombProjectile::SpawnParticleOnMultiCast_Validate(FVector Location) {
	return true;
}