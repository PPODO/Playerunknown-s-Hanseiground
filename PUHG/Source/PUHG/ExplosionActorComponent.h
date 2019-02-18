#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ExplosionActorComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PUHG_API UExplosionActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UExplosionActorComponent();

	void Excute(FVector Location);

private:
	UFUNCTION(Server, Reliable, WithValidation)
		void SpawnParticleAndSoundOnServer(FVector Location);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void SpawnParticleAndSoundOnMultiCast(FVector Location);

private:
	UPROPERTY(VisibleDefaultsOnly)
		class UParticleSystem* ExplosionParticle;

	UPROPERTY(VisibleDefaultsOnly)
		class USoundCue* ExplosionSound;

	FVector ExplosionScale = FVector(3.f);

};
