#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Rocket.generated.h"

UCLASS()
class PUHG_API ARocket : public AActor
{
	GENERATED_BODY()
	
public:
	ARocket();

private:
	UPROPERTY(VisibleDefaultsOnly)
		class UStaticMeshComponent* Rocket;

	UPROPERTY(VisibleDefaultsOnly)
		class UParticleSystemComponent* FlareParticle;

	UPROPERTY(VisibleDefaultsOnly)
		class UParticleSystemComponent* SmokeTrailParticle;

	UPROPERTY(VisibleDefaultsOnly)
		class USceneComponent* DefaultScene;

};
