#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PUHGCharacter.h"
#include "JunYeong_Character.generated.h"

UCLASS()
class PUHG_API AJunYeong_Character : public APUHGCharacter
{
	GENERATED_BODY()

private:
	TSubclassOf<class ATimeBombProjectile> AbilityProjectile;

	TArray<class USoundCue*> AbilitySounds;
	TArray<class USoundCue*> TimeStopSounds;
	class USoundCue* AbilityEndSound;

	FTimerHandle AbilityTimerHandle;

public:
	AJunYeong_Character();

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

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void PlayStartSoundOnMulti();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void PlayTimeStopSoundOnMulti();

};
