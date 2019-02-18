#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MultiHomingProjectile.generated.h"

UCLASS()
class PUHG_API AMultiHomingProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	AMultiHomingProjectile();

	void SetHomingTarget(class USceneComponent* Target);

	void SetOwnerPlayerName(const FString& OwnerName);

protected:
	virtual void BeginPlay() override;

private:
	FString OwnerPlayerName;
	int32 MaxRocket;
	FTransform MissileTransform;
	FTimerHandle DestroyTimerHandle;

	class USceneComponent* Target;

private:
	UPROPERTY(VisibleDefaultsOnly)
		class USphereComponent* CollisionComponent;

	UPROPERTY(VisibleDefaultsOnly)
		class URadialForceComponent* RadialForceComponent;

	UPROPERTY(VisibleDefaultsOnly)
		class UChildActorComponent* RocketChildComponent;

	UPROPERTY(VisibleDefaultsOnly)
		class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(VisibleDefaultsOnly)
		class UParticleSystem* ImpactParticle;

	UPROPERTY(VisibleDefaultsOnly)
		class USoundCue* ImpactSound;

private:
	UFUNCTION()
		void OnHitComponent(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void DestroyAndSpawnMissile();

	FTransform SetHomingDirection(int32);

private:
	UFUNCTION(Server, Reliable, WithValidation)
		void SpawnParticleAndSoundOnServer();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void SpawnParticleAndSoundOnMultiCast();

	UFUNCTION(Server, Reliable, WithValidation)
		void SpawnMultiHomingMissileOnServer(FTransform NewTransform);

};
