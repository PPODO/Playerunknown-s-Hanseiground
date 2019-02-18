#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PUHGCharacter.generated.h"

UCLASS()
class PUHG_API APUHGCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APUHGCharacter();

	UFUNCTION(Server, Reliable, WithValidation)
		void SetUseTickDeltaTime(bool bUse);

	UPROPERTY(BlueprintReadOnly)
		bool bCoolTime = false;

	FTimerHandle AbilityCoolTime;

	void CoolTimeTimer();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Landed(const FHitResult & Hit) override;

	virtual void PossessedBy(class AController* Controller) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

/* Setting */
protected:
	// Axis

	UFUNCTION()
		void MoveForward(float Value);
	UFUNCTION()
		void MoveRight(float Value);
	UFUNCTION()
		void TurnRate(float Value);
	UFUNCTION()
		void LookAtRate(float Value);

	// Action

	UFUNCTION()
		void OnReload();
	UFUNCTION()
		void OnAim();
	UFUNCTION()
		void ReleaseAim();
	UFUNCTION()
		void OnFire();
	UFUNCTION()
		void ReleaseFire();
	UFUNCTION()
		void OnRun();
	UFUNCTION()
		void ReleaseRun();
	UFUNCTION()
		void OnJump();
	UFUNCTION()
		void ReleaseJump();
	UFUNCTION()
		void OnCrouch();

protected:
	class AInGamePC* MyController;

	float BaseTurnRate, BaseLookAtRate;
	float SaveTickTime, DeltaSecond;
	bool bUseTickDeltaTime;

	UPROPERTY(VisibleDefaultsOnly)
		class USpringArmComponent* SpringArmComp;
	UPROPERTY(VisibleDefaultsOnly)
		class UCameraComponent* CameraComp;
	UPROPERTY(VisibleDefaultsOnly)
		class USkeletalMeshComponent* FP_Gun;

protected:
	UPROPERTY(VisibleDefaultsOnly)
		class UMaterialInstanceDynamic* BodyMaterial;

	UPROPERTY(VisibleDefaultsOnly)
		class USceneComponent* FP_MuzzleLocation;

	UPROPERTY(VisibleDefaultsOnly)
		class USoundCue* RifleFireSound;

	UPROPERTY(VisibleDefaultsOnly)
		class USoundCue* RifleEmptySound;

	UPROPERTY(VisibleDefaultsOnly)
		class USoundCue* ReloadingSound;

	UPROPERTY(VisibleDefaultsOnly)
		class USoundCue* ReloadingPlayerSound;
	UPROPERTY(VisibleDefaultsOnly)
		class USoundCue* DeadPlayerSound;
	UPROPERTY(VisibleDefaultsOnly)
		class USoundCue* AbilityPlayerSound;

	TSubclassOf<class AProjectile> ProjectileClass;

	FVector SpawnLocation;
	FRotator SpawnRotation;

	float DefaultTargetLength = 200.f;																		
	float AimTargetLength = 125.f;
	FVector DefaultSocketLocation = FVector(0.f, 0.f, 0.f);
	FVector AimSocketLocation = FVector(30.f, 10.f, -25.f);
	FVector CrouchSocketLocation = FVector(0.f, 0.f, -20.f);

public:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		float Health;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		int32 Ammo;

	void SpringArmCameraMovement(float);

private:
	FHitResult HitLocation;
	FVector LineTraceStart, LineTraceEnd, SelectedVector;
	bool IsHit;

	float OldJumpTime = -1.3f;
	float FireProjectileDelay = -0.15f;

	FTimerHandle CharacterDestroyHandle;

public:
	FString CurrentPlayerName;

public:
	UPROPERTY(Replicated)
		bool bIsMoving;
	UPROPERTY(Replicated)
		bool bIsRunning;
	UPROPERTY(Replicated)
		bool bIsJumping;
	UPROPERTY(Replicated)
		bool bIsAiming;
	UPROPERTY(Replicated)
		bool bIsCrouching;
	UPROPERTY(Replicated)
		bool bIsReloading;
	UPROPERTY(Replicated)
		bool bIsFire;
	UPROPERTY(Replicated)
		FRotator CurrentLookRotation;

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void ReloadOnServer(bool IsReload);

protected:
	// Change State
	UFUNCTION(Server, Reliable, WithValidation)
		void ChangePlayerWalkSpeedOnServer(float Speed);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void ChangePlayerWalkSpeedOnMultiCast(float Speed);

	UFUNCTION(Server, Reliable, WithValidation)
		void ChangePlayerCapsuleSizeOnServer(float Size);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void ChangePlayerCapsuleSizeOnMultiCast(float Size);

	UFUNCTION(Server, Reliable, WithValidation)
		void ChangePlayerCapsuleHeightOnServer(float Size);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void ChangePlayerCapsuleHeightOnMultiCast(float Size);

	UFUNCTION(Server, Reliable, WithValidation)
		void ChangePlayerJumpOnServer(bool StartOrStop);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void ChangePlayerJumpOnMultiCast(bool StartOrStop);

	//

	UFUNCTION(Server, Reliable, WithValidation)
		void MoveOnServer(bool IsMoving);

	UFUNCTION(Server, Reliable, WithValidation)
		void RunOnServer(bool IsRunning);

	UFUNCTION(Server, Reliable, WithValidation)
		void JumpOnServer(bool JumpOrNot);

	UFUNCTION(Server, Reliable, WithValidation)
		void AimOnServer(bool IsAim);

	UFUNCTION(Server, Reliable, WithValidation)
		void CrouchOnServer(bool IsCrouch);

	UFUNCTION(Server, Reliable, WithValidation)
		void FireOnServer(bool IsFire);

	UFUNCTION(Server, Reliable, WithValidation)
		void FireProjectileOnServer(FVector SpawnLocation);

	UFUNCTION(Server, Reliable, WithValidation)
		void DestroyCharacterWithTimerServer();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void DestroyCharacterWithTimerMultiCast();

public:
	UFUNCTION(Server, Reliable, WithValidation)
		void ApplyDamageOnServer(float Damage, const FString& DamageCauserName);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void ApplyDamageOnMultiCast(float Damage, const FString& DamageCauserName);

	UFUNCTION(Client, Reliable, WithValidation)
		void CameraRecoilOnMultiCast(class AController* PlayerController);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void BulletCountDown();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void ResetBullet();

	UFUNCTION(Server, Reliable, WithValidation)
		void PlayerDeadOnServer();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void PlayerDeadOnMultiCast();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void SoundOnMultiCast(class USoundCue* Sound);


	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void SoundAtLocationOnMultiCast(class USoundCue* Sound, FVector Location);
};
