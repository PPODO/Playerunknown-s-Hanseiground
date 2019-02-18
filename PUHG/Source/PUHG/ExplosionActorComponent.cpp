#include "ExplosionActorComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "ConstructorHelpers.h"

UExplosionActorComponent::UExplosionActorComponent() {
	ConstructorHelpers::FObjectFinder<UParticleSystem> ExplosionParticleObject(L"ParticleSystem'/Game/ProjectilesPack/Particles/Effects/P_ExplosionWithShrapnel.P_ExplosionWithShrapnel'");
	ConstructorHelpers::FObjectFinder<USoundCue> ExplosionSoundObject(L"SoundCue'/Game/ProjectilesPack/Sounds/Common/Explosion_01_Cue.Explosion_01_Cue'");

	if (ExplosionParticleObject.Succeeded()) {
		ExplosionParticle = ExplosionParticleObject.Object;
	}
	if (ExplosionSoundObject.Succeeded()) {
		ExplosionSound = ExplosionSoundObject.Object;
	}
}

void UExplosionActorComponent::Excute(FVector Location) {
	SpawnParticleAndSoundOnServer(Location);
}

void UExplosionActorComponent::SpawnParticleAndSoundOnServer_Implementation(FVector Location) {
	SpawnParticleAndSoundOnMultiCast(Location);
}

bool UExplosionActorComponent::SpawnParticleAndSoundOnServer_Validate(FVector Location) {
	return true;
}

void UExplosionActorComponent::SpawnParticleAndSoundOnMultiCast_Implementation(FVector Location) {
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, Location);
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticle, Location)->SetRelativeScale3D(ExplosionScale);
}

bool UExplosionActorComponent::SpawnParticleAndSoundOnMultiCast_Validate(FVector Location) {
	return true;
}