#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PUHGCharacter.h"
#include "SeungChan_Character.generated.h"

UCLASS()
class PUHG_API ASeungChan_Character : public APUHGCharacter
{
	GENERATED_BODY()

private:
	TSubclassOf<class ATimeBombProjectile> AbilityProjectile;

public:
	ASeungChan_Character();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Landed(const FHitResult & Hit) override;

	virtual void PossessedBy(class AController* Controller) override;

private:
	UFUNCTION()
		void UseAbility();

	UFUNCTION(Server, Reliable, WithValidation)
		void TimeBombOnServer();

};