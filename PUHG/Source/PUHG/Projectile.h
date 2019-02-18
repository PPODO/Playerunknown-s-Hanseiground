#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class PUHG_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	AProjectile();

public:
	void SetOwnerPlayerName(const FString& OwnerName);

private:
	UPROPERTY(EditAnywhere)
		class USphereComponent* CollisionComponent;
	UPROPERTY(VisibleDefaultsOnly)
		class UStaticMeshComponent* Projectile;
	UPROPERTY(VisibleDefaultsOnly)
		class UProjectileMovementComponent* ProjectileMovement;
	UPROPERTY(VisibleDefaultsOnly)
		class UParticleSystem* ImpactParticle;
	UPROPERTY(VisibleDefaultsOnly)
		class UParticleSystemComponent* ProjectileTrail;
	UPROPERTY(VisibleDefaultsOnly)
		class URadialForceComponent* RadialForceComponent;
	UPROPERTY(VisibleDefaultsOnly)
		class USoundCue* HitSound;

	FString OwnerPlayerName;
	FRotator ImpactParticleDefaultRotation = FRotator(-90.f, 0.f, 0.f);

	UFUNCTION()
		void OnHitComponent(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	UFUNCTION(Server, Reliable, WithValidation)
		void SpawnParticleAndSoundOnServer(const FHitResult& Hit);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void SpawnParticleAndSoundOnMultiCast(const FHitResult& Hit);

	float ProjectileDamage;

};
