#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimeBombProjectile.generated.h"

UCLASS()
class PUHG_API ATimeBombProjectile : public AActor
{
	GENERATED_BODY()

public:
	ATimeBombProjectile();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleDefaultsOnly)
		class URadialForceComponent* ForceComp;
	UPROPERTY(VisibleDefaultsOnly)
		class UPostProcessComponent* PostProcess;
	UPROPERTY(VisibleDefaultsOnly)
		class USphereComponent* CollisionComponent;
	UPROPERTY(VisibleDefaultsOnly)
		class UTimeBombActorComponent* TimeBombComponent;

	TArray<class AActor*> OverlapActors;

	FTimerHandle TimerHandle;

private:
	UPROPERTY(VisibleDefaultsOnly)
		class UParticleSystem* ImpactParticle;

	FVector RelativeScale = FVector(3.f, 3.f, 3.f);

	UFUNCTION(Server, Reliable, WithValidation)
		void SpawnParticleOnServer(FVector Location);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void SpawnParticleOnMultiCast(FVector Location);

};
