#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PUHGCharacter.h"
#include "YoonSung_Character.generated.h"

UCLASS()
class PUHG_API AYoonSung_Character : public APUHGCharacter
{
	GENERATED_BODY()

public:
	AYoonSung_Character();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Landed(const FHitResult & Hit) override;

	virtual void PossessedBy(class AController* Controller) override;

};
