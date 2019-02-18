#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HomingProjectile.generated.h"

UCLASS()
class PUHG_API AHomingProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	AHomingProjectile();

	void SetHomingTarget(class USceneComponent* Target);

	void SetOwnerPlayerName(const FString& OwnerName);

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

private:
	FString OwnerPlayerName;

	FVector StartLocation;

	FHitResult HitResultSave;

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
		class UExplosionActorComponent* ExplosionComponent;

private:
	UFUNCTION()
		void OnHitComponent(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

};
