#include "PUHGAnimInstance.h"
#include "PUHGCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UPUHGAnimInstance::UPUHGAnimInstance() {
	

}

void UPUHGAnimInstance::NativeBeginPlay() {
	Super::NativeBeginPlay();

}

void UPUHGAnimInstance::NativeUpdateAnimation(float DeltaSecond) {
	Super::NativeUpdateAnimation(DeltaSecond);
	if (!Char) {
		if (TryGetPawnOwner()) {
			Char = Cast<APUHGCharacter>(TryGetPawnOwner());
		}
	}

	if (Char) {
		// Cal Pitch
		FRotator NewRotation = FMath::RInterpTo(FRotator(LookPitch, 0.f, 0.f), Char->CurrentLookRotation, DeltaSecond, 15.f);
		LookPitch = FMath::ClampAngle(NewRotation.Pitch, -90.f, 90.f);

		// Cal Direction, Speed
		Direction = CalculateDirection(Char->GetVelocity(), Char->GetActorRotation());
		Speed = Char->GetVelocity().Size();

		bIsCrouching = Char->bIsCrouching;
		bIsMoving = Char->bIsMoving;
		bIsRunning = Char->bIsRunning;
		bIsJumping = Char->bIsJumping;
		bIsAiming = Char->bIsAiming;
		bIsReloading = Char->bIsReloading;
		bIsFire = Char->bIsFire;
	}
}