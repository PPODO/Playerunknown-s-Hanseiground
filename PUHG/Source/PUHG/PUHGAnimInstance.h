#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PUHGAnimInstance.generated.h"

UCLASS()
class PUHG_API UPUHGAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UPUHGAnimInstance();

protected:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSecond) override;

private:
	class APUHGCharacter* Char = nullptr;

public:
	UPROPERTY(BlueprintReadWrite)
		float Speed;
	UPROPERTY(BlueprintReadWrite)
		float Direction;
	UPROPERTY(BlueprintReadWrite)
		bool bIsMoving;
	UPROPERTY(BlueprintReadWrite)
		bool bIsRunning;
	UPROPERTY(BlueprintReadWrite)
		bool bIsJumping;
	UPROPERTY(BlueprintReadWrite)
		bool bIsCrouching;
	UPROPERTY(BlueprintReadWrite)
		bool bIsAiming;
	UPROPERTY(BlueprintReadWrite)
		bool bIsReloading;
	UPROPERTY(BlueprintReadWrite)
		bool bIsFire;
	UPROPERTY(BlueprintReadWrite)
		float LookPitch;

};
