#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PUHGCharacter.h"
#include "SungHun_Character.generated.h"

UCLASS()
class PUHG_API ASungHun_Character : public APUHGCharacter
{
	GENERATED_BODY()

public:
	ASungHun_Character();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Landed(const FHitResult & Hit) override;

	virtual void PossessedBy(class AController* Controller) override;

	virtual void BeginDestroy() override;

private:
	class AActor* HomingTargetActor;
	class AActor* BeforeHomingTargetActor;

private:
	UFUNCTION()
		void UseAbility();

	UFUNCTION()
		void SetTarget();

	UFUNCTION(Server, Reliable, WithValidation)
		void ShootTraceAndSetTargetOnServer(FHitResult Hit);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void ShootTraceAndSetTargetOnMulti(FHitResult Hit);

	UFUNCTION(Server, Reliable, WithValidation)
		void AbilityOnServer(class AActor* Target);

};
