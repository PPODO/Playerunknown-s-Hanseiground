#include "PUHGCharacter.h"
#include "Projectile.h"
#include "InGamePC.h"
#include "PUHGAnimInstance.h"
#include "PUGHGameModeBase.h"
#include "Animation/AnimBlueprint.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpectatorPawn.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "ConstructorHelpers.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"

APUHGCharacter::APUHGCharacter() {
	ConstructorHelpers::FObjectFinder<USoundCue> FireSoundObject(L"SoundCue'/Game/ProjectilesPack/Sounds/Common/Rifle_Cue.Rifle_Cue'");
	ConstructorHelpers::FObjectFinder<USoundCue> EmptySoundObject(L"SoundCue'/Game/ProjectilesPack/Sounds/Common/Empty_Gun_Cue.Empty_Gun_Cue'");
	ConstructorHelpers::FObjectFinder<USoundCue> ReloadSoundObject(L"SoundCue'/Game/ProjectilesPack/Sounds/Common/Reload_Sound_Cue.Reload_Sound_Cue'");

	if (FireSoundObject.Succeeded()) {
		RifleFireSound = FireSoundObject.Object;
	}
	if (EmptySoundObject.Succeeded()) {
		RifleEmptySound = EmptySoundObject.Object;
	}
	if (ReloadSoundObject.Succeeded()) {
		ReloadingSound = ReloadSoundObject.Object;
	}

	bUseControllerRotationPitch = bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->CrouchedHalfHeight = 60.f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 300.f;

	GetCharacterMovement()->JumpZVelocity = 475.f;
	GetCapsuleComponent()->SetCapsuleRadius(25.f);
	GetCapsuleComponent()->SetNotifyRigidBodyCollision(true);

	SpawnLocation = GetActorLocation();
	ProjectileClass = AProjectile::StaticClass();

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(L"Spring Arm Component");
	SpringArmComp->bDoCollisionTest = false;
	SpringArmComp->TargetArmLength = DefaultTargetLength;
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetRelativeLocation(FVector(0.f, 50.f, 90.f));
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(L"Camera Component");
	CameraComp->bUsePawnControlRotation = true;
	CameraComp->AttachTo(SpringArmComp, USpringArmComponent::SocketName);

	BaseTurnRate = 30.f;
	BaseLookAtRate = 40.f;
	Ammo = 32;
	Health = 100.f;
	bUseTickDeltaTime = false;

	SetReplicates(true);
	SetReplicateMovement(true);
	bUseTickDeltaTime = false;
	PrimaryActorTick.bCanEverTick = true;
}

void APUHGCharacter::SetUseTickDeltaTime_Implementation(bool bUse) {
	if (!bUse) {
		FireProjectileDelay = GetWorld()->GetTimeSeconds();
	}
	bUseTickDeltaTime = bUse;
}

bool APUHGCharacter::SetUseTickDeltaTime_Validate(bool bUse) {
	return true;
}

void APUHGCharacter::CoolTimeTimer() {
	bCoolTime = false;
	GetWorld()->GetTimerManager().ClearTimer(AbilityCoolTime);
}

void APUHGCharacter::BeginPlay() {
	Super::BeginPlay();

	if (Controller) {
		MyController = Cast<AInGamePC>(Controller);
		if (Controller->GetPlayerState<APlayerState>()) {
			CurrentPlayerName = Controller->GetPlayerState<APlayerState>()->GetPlayerName();
		}
	}
}

void APUHGCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	SaveTickTime += DeltaTime;
	DeltaSecond = DeltaTime;

	if (MyController) {
		MyController->TickSpreadMove(DeltaTime);
	}
	if (bIsMoving && GetVelocity().Size() < 10.f) {
		if (bIsRunning) {
			ReleaseRun();
		}
		MoveOnServer(false);
	}
	CurrentLookRotation = GetControlRotation() - GetActorRotation();
	if (bIsFire && IsLocallyControlled()) {
		if (Ammo > 0) {
			FireProjectileOnServer(FP_MuzzleLocation->GetComponentLocation());
		}
		else {
			ReleaseFire();
		}
	}
	SpringArmCameraMovement(DeltaTime);
}

void APUHGCharacter::SetupPlayerInputComponent(UInputComponent * PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(L"Crouch", IE_Pressed, this, &APUHGCharacter::OnCrouch);
	PlayerInputComponent->BindAction(L"Aim", IE_Pressed, this, &APUHGCharacter::OnAim);
	PlayerInputComponent->BindAction(L"Aim", IE_Released, this, &APUHGCharacter::ReleaseAim);
	PlayerInputComponent->BindAction(L"Run", IE_Pressed, this, &APUHGCharacter::OnRun);
	PlayerInputComponent->BindAction(L"Run", IE_Released, this, &APUHGCharacter::ReleaseRun);
	PlayerInputComponent->BindAction(L"Shoot", IE_Pressed, this, &APUHGCharacter::OnFire);
	PlayerInputComponent->BindAction(L"Shoot", IE_Released, this, &APUHGCharacter::ReleaseFire);
	PlayerInputComponent->BindAction(L"Jump", IE_Pressed, this, &APUHGCharacter::OnJump);
	PlayerInputComponent->BindAction(L"Jump", IE_Released, this, &APUHGCharacter::ReleaseJump);
	PlayerInputComponent->BindAction(L"ReloadAmmo", IE_Released, this, &APUHGCharacter::OnReload);

	PlayerInputComponent->BindAxis(L"ForwardMove", this, &APUHGCharacter::MoveForward);
	PlayerInputComponent->BindAxis(L"RightMove", this, &APUHGCharacter::MoveRight);
	PlayerInputComponent->BindAxis(L"TurnRate", this, &APUHGCharacter::TurnRate);
	PlayerInputComponent->BindAxis(L"LookAtRate", this, &APUHGCharacter::LookAtRate);
}

void APUHGCharacter::Landed(const FHitResult & Hit) {
	Super::Landed(Hit);

	if (Role == ROLE_Authority) {
		JumpOnServer(false);
		ChangePlayerCapsuleHeightOnServer(88.f);
		ChangePlayerCapsuleSizeOnServer(25.f);
	}
}

void APUHGCharacter::PossessedBy(AController * Controller) {
	Super::PossessedBy(Controller);

	SaveTickTime = GetWorld()->GetTimeSeconds();
	MyController = Cast<AInGamePC>(Controller);
	if (Controller->GetPlayerState<APlayerState>()) {
		CurrentPlayerName = Controller->GetPlayerState<APlayerState>()->GetPlayerName();
	}
}

void APUHGCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APUHGCharacter, bIsMoving);
	DOREPLIFETIME(APUHGCharacter, bIsRunning);
	DOREPLIFETIME(APUHGCharacter, bIsJumping);
	DOREPLIFETIME(APUHGCharacter, bIsAiming);
	DOREPLIFETIME(APUHGCharacter, bIsCrouching);
	DOREPLIFETIME(APUHGCharacter, bIsReloading);
	DOREPLIFETIME(APUHGCharacter, bIsFire);
	DOREPLIFETIME(APUHGCharacter, CurrentLookRotation);
}

float APUHGCharacter::TakeDamage(float Damage, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser) {
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	ApplyDamageOnServer(Damage, DamageCauser->GetName());
	return 0.0f;
}

/* Input! */

void APUHGCharacter::OnReload() {
	if (!bIsFire && Ammo < 32 && !bIsReloading) {
		ReloadOnServer(true);
		SoundAtLocationOnMultiCast(ReloadingSound, GetActorLocation());
		SoundAtLocationOnMultiCast(ReloadingPlayerSound, GetActorLocation());
	}
}

void APUHGCharacter::OnAim() {
	if (!bIsJumping) {
		if (MyController) {
			MyController->SpreadMax = 0.03f;
			MyController->SpreadCurrent = 0.005f;
			MyController->SpreadMin = 0.005f;
		}
		if (bIsRunning) {
			RunOnServer(false);
		}
		ChangePlayerWalkSpeedOnServer(300.f);
		AimOnServer(true);
	}
}

void APUHGCharacter::ReleaseAim() {
	if (MyController) {
		MyController->SpreadMax = 0.3f;
		MyController->SpreadCurrent = 0.15f;
		MyController->SpreadMin = 0.15f;
	}
	ChangePlayerWalkSpeedOnServer(600.f);
	AimOnServer(false);
}

void APUHGCharacter::OnFire() {
	if (Ammo > 0 && !bIsReloading) {
		if (bIsRunning) {
			ReleaseRun();
		}
		FireOnServer(true);
	}
	else if(Ammo <= 0){
		SoundOnMultiCast(RifleEmptySound);
	}
	UE_LOG(LogTemp, Warning, L"ReleaseFIRE");
}

void APUHGCharacter::ReleaseFire() {
	FireOnServer(false);
	UE_LOG(LogTemp, Warning, L"ReleaseFIRE");
}

void APUHGCharacter::OnRun() {
	if (bIsMoving && !bIsAiming && !bIsJumping) {
		if (!bIsCrouching) {
			ChangePlayerCapsuleSizeOnServer(35.f);
		}
		else {
			OnCrouch();
		}
		if (bIsFire) {
			ReleaseFire();
		}
		ChangePlayerWalkSpeedOnServer(800.f);
		RunOnServer(true);

		if (MyController) {
			MyController->SpreadMax = 0.4f;
			MyController->SpreadCurrent = 0.4f;
			MyController->SpreadMin = 0.25f;
		}
	}
}

void APUHGCharacter::ReleaseRun() {
	if (bIsMoving && !bIsAiming) {
		ChangePlayerWalkSpeedOnServer(600.f);
		ChangePlayerCapsuleSizeOnServer(25.f);
		RunOnServer(false);

		if (MyController) {
			MyController->SpreadMax = 0.3f;
			MyController->SpreadCurrent = 0.15f;
			MyController->SpreadMin = 0.15f;
		}
	}
}

void APUHGCharacter::OnJump() {
	if(!bIsCrouching && GetWorld()->GetTimeSeconds() - OldJumpTime > 1.3f) {
		OldJumpTime = GetWorld()->GetTimeSeconds();
		ChangePlayerJumpOnServer(true);
		ChangePlayerCapsuleSizeOnServer(35.f);
		ChangePlayerCapsuleHeightOnServer(80.f);
		JumpOnServer(true);
	}
}

void APUHGCharacter::ReleaseJump() {
	ChangePlayerJumpOnServer(false);
}

void APUHGCharacter::OnCrouch() {
	if (!bIsJumping && !bIsCrouching) {
		if (!bIsAiming) {
			ChangePlayerWalkSpeedOnServer(400.f);
			if (MyController) {
				MyController->SpreadMax = 0.2f;
				MyController->SpreadCurrent = 0.1f;
				MyController->SpreadMin = 0.1f;
			}
		}
		if (bIsRunning) {
			ChangePlayerWalkSpeedOnServer(800.f);
			ChangePlayerCapsuleSizeOnServer(25.f);
			if (MyController) {
				MyController->SpreadMax = 0.4f;
				MyController->SpreadCurrent = 0.4f;
				MyController->SpreadMin = 0.25f;
			}
		}
		CrouchOnServer(true);
		GetCharacterMovement()->bWantsToCrouch = true;
	}
	else if(!bIsJumping && bIsCrouching) {
		if (!bIsAiming) {
			ChangePlayerWalkSpeedOnServer(600.f);
			if (MyController) {
				MyController->SpreadMax = 0.3f;
				MyController->SpreadCurrent = 0.15f;
				MyController->SpreadMin = 0.15f;
			}
		}
		if (bIsRunning) {
			ChangePlayerWalkSpeedOnServer(800.f);
			ChangePlayerCapsuleSizeOnServer(35.f);
			if (MyController) {
				MyController->SpreadMax = 0.4f;
				MyController->SpreadCurrent = 0.4f;
				MyController->SpreadMin = 0.25f;
			}
		}
		CrouchOnServer(false);
		GetCharacterMovement()->bWantsToCrouch = false;
	}
}

void APUHGCharacter::MoveForward(float Value) {
	if (Value != 0.f) {
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
		MoveOnServer(true);

		if (MyController) {
			MyController->Increase_Spread(0.1f);
		}
	}
}

void APUHGCharacter::MoveRight(float Value) {
	if (Value != 0.f) {
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
		MoveOnServer(true);

		if (MyController) {
			MyController->Increase_Spread(0.1f);
		}
	}
}

void APUHGCharacter::TurnRate(float Value) {
	if (Controller && Value != 0.f) {
		AddControllerYawInput(Value * BaseTurnRate *  (!bUseTickDeltaTime ? GetWorld()->GetDeltaSeconds() : DeltaSecond));
	}
}

void APUHGCharacter::LookAtRate(float Value) {
	if (Controller && Value != 0.f) {
		AddControllerPitchInput(Value * BaseLookAtRate * (!bUseTickDeltaTime ? GetWorld()->GetDeltaSeconds() : DeltaSecond));
	}
}

/* Server!!! */

void APUHGCharacter::ChangePlayerWalkSpeedOnServer_Implementation(float Speed) {
	ChangePlayerWalkSpeedOnMultiCast(Speed);
}

bool APUHGCharacter::ChangePlayerWalkSpeedOnServer_Validate(float Speed) {
	return true;
}

void APUHGCharacter::ChangePlayerWalkSpeedOnMultiCast_Implementation(float Speed) {
	if (GetCharacterMovement()) {
		GetCharacterMovement()->MaxWalkSpeed = Speed;
	}
}

bool APUHGCharacter::ChangePlayerWalkSpeedOnMultiCast_Validate(float Speed) {
	return true;
}

void APUHGCharacter::ChangePlayerJumpOnServer_Implementation(bool StartOrStop) {
	ChangePlayerJumpOnMultiCast(StartOrStop);
}

bool APUHGCharacter::ChangePlayerJumpOnServer_Validate(bool StartOrStop) {
	return true;
}

void APUHGCharacter::ChangePlayerJumpOnMultiCast_Implementation(bool StartOrStop) {
	if (StartOrStop) {
		Jump();
	}
	else {
		StopJumping();
	}
}

bool APUHGCharacter::ChangePlayerJumpOnMultiCast_Validate(bool StartOrStop) {
	return true;
}

void APUHGCharacter::ChangePlayerCapsuleSizeOnServer_Implementation(float Size) {
	ChangePlayerCapsuleSizeOnMultiCast(Size);
}

bool APUHGCharacter::ChangePlayerCapsuleSizeOnServer_Validate(float Size) {
	return true;
}

void APUHGCharacter::ChangePlayerCapsuleSizeOnMultiCast_Implementation(float Size) {
	GetCapsuleComponent()->SetCapsuleRadius(Size);
}

bool APUHGCharacter::ChangePlayerCapsuleSizeOnMultiCast_Validate(float Size) {
	return true;
}

void APUHGCharacter::ChangePlayerCapsuleHeightOnServer_Implementation(float Size) {
	ChangePlayerCapsuleHeightOnMultiCast(Size);
}

bool APUHGCharacter::ChangePlayerCapsuleHeightOnServer_Validate(float Size) {
	return true;
}

void APUHGCharacter::ChangePlayerCapsuleHeightOnMultiCast_Implementation(float Size) {
	GetCapsuleComponent()->SetCapsuleHalfHeight(Size);
}

bool APUHGCharacter::ChangePlayerCapsuleHeightOnMultiCast_Validate(float Size) {
	return true;
}

/*  */

void APUHGCharacter::FireProjectileOnServer_Implementation(FVector SpawnLocation) {
	if (ProjectileClass && (!bUseTickDeltaTime ? GetWorld()->GetTimeSeconds() : SaveTickTime) - FireProjectileDelay > 0.15f) {
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		LineTraceStart = CameraComp->GetComponentLocation();
		LineTraceEnd = CameraComp->GetComponentLocation() + CameraComp->GetForwardVector() * 5000.f;
		IsHit = GetWorld()->LineTraceSingleByChannel(HitLocation, LineTraceStart, LineTraceEnd, ECC_Visibility);
		SelectedVector = UKismetMathLibrary::SelectVector(HitLocation.ImpactPoint, LineTraceEnd, IsHit);

		SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, SelectedVector);

		AProjectile* ProjectileActor = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (ProjectileActor) {
			ProjectileActor->SetOwnerPlayerName(CurrentPlayerName);
		}
		SoundOnMultiCast(RifleFireSound);
		BulletCountDown();
		FireProjectileDelay = (!bUseTickDeltaTime ? GetWorld()->GetTimeSeconds() : SaveTickTime);

		CameraRecoilOnMultiCast(Controller);
	}
}

bool APUHGCharacter::FireProjectileOnServer_Validate(FVector SpawnLocation) {
	return true;
}

void APUHGCharacter::CameraRecoilOnMultiCast_Implementation(AController * PlayerController) {
	AInGamePC* TempCon = Cast<AInGamePC>(PlayerController);
	if (TempCon) {
		TempCon->CameraRecoil(0.3f, 1.5f);
	}
}

bool APUHGCharacter::CameraRecoilOnMultiCast_Validate(AController * PlayerController) {
	return true;
}

void APUHGCharacter::BulletCountDown_Implementation() {
	--Ammo;
}

bool APUHGCharacter::BulletCountDown_Validate() {
	return true;
}

void APUHGCharacter::ResetBullet_Implementation() {
	Ammo = 32;
}

bool APUHGCharacter::ResetBullet_Validate() {
	return true;
}

void APUHGCharacter::SoundOnMultiCast_Implementation(USoundCue* Sound) {
	UGameplayStatics::PlaySound2D(GetWorld(), Sound);
}

bool APUHGCharacter::SoundOnMultiCast_Validate(USoundCue* Sound) {
	return true;
}

void APUHGCharacter::SoundAtLocationOnMultiCast_Implementation(USoundCue* Sound, FVector Location) {
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location);
}

bool APUHGCharacter::SoundAtLocationOnMultiCast_Validate(USoundCue* Sound, FVector Location) {
	return true;
}

void APUHGCharacter::ReloadOnServer_Implementation(bool IsReload) {
	bIsReloading = IsReload;
	if (!IsReload) {
		ResetBullet();
	}
}

bool APUHGCharacter::ReloadOnServer_Validate(bool IsReload) {
	return true;
}

void APUHGCharacter::MoveOnServer_Implementation(bool IsMoving) {
	bIsMoving = IsMoving;
}

bool APUHGCharacter::MoveOnServer_Validate(bool IsMoving) {
	return true;
}

void APUHGCharacter::RunOnServer_Implementation(bool IsRun) {
	bIsRunning = IsRun;
}

bool APUHGCharacter::RunOnServer_Validate(bool IsRun) {
	return true;
}

void APUHGCharacter::JumpOnServer_Implementation(bool JumpOrNot) {
	bIsJumping = JumpOrNot;
}

bool APUHGCharacter::JumpOnServer_Validate(bool JumpOrNot) {
	return true;
}

void APUHGCharacter::AimOnServer_Implementation(bool IsAim) {
	bIsAiming = IsAim;
}

bool APUHGCharacter::AimOnServer_Validate(bool IsAim) {
	return true;
}

void APUHGCharacter::FireOnServer_Implementation(bool bIsFire) {
	this->bIsFire = bIsFire;
}

bool APUHGCharacter::FireOnServer_Validate(bool bIsFire) {
	return true;
}

void APUHGCharacter::CrouchOnServer_Implementation(bool bIsCrouch) {
	bIsCrouching = bIsCrouch;
}

bool APUHGCharacter::CrouchOnServer_Validate(bool bIsCrouch) {
	return true;
}

/* System */

void APUHGCharacter::ApplyDamageOnServer_Implementation(float Damage, const FString& DamageCauserName) {
	ApplyDamageOnMultiCast(Damage, DamageCauserName);
}

bool APUHGCharacter::ApplyDamageOnServer_Validate(float Damage, const FString& DamageCauserName) {
	return true;
}

void APUHGCharacter::ApplyDamageOnMultiCast_Implementation(float Damage, const FString& DamageCauserName) {
	if (DamageCauserName != CurrentPlayerName && Controller) {
		if (FMath::IsNearlyZero(Health) || Health - Damage <= 0.f) {
			APUGHGameModeBase* GameMode = Cast<APUGHGameModeBase>(GetWorld()->GetAuthGameMode());
			if (GameMode) {
				GameMode->UpdateKillLog(DamageCauserName, CurrentPlayerName, Controller);
			}
			if (Role == ROLE_Authority) {
				GetWorld()->GetTimerManager().SetTimer(CharacterDestroyHandle, this, &APUHGCharacter::DestroyCharacterWithTimerServer, 5.f, false);
				PlayerDeadOnServer();
			}
		}
		else {
			Health -= Damage;
		}
	}
}

bool APUHGCharacter::ApplyDamageOnMultiCast_Validate(float Damage, const FString& DamageCauserName) {
	return true;
}

void APUHGCharacter::PlayerDeadOnServer_Implementation() {
	PlayerDeadOnMultiCast();
}

bool APUHGCharacter::PlayerDeadOnServer_Validate() {
	return true;
}

void APUHGCharacter::PlayerDeadOnMultiCast_Implementation() {
	if (GetMesh() && GetCapsuleComponent()) {
		GetCapsuleComponent()->SetCollisionProfileName(L"Ragdoll");
		GetMesh()->SetCollisionProfileName(L"Ragdoll");
		GetMesh()->SetSimulatePhysics(true);

		if (Role == ROLE_Authority && Controller) {
			ASpectatorPawn* SpectatorPawn = GetWorld()->SpawnActor<ASpectatorPawn>(ASpectatorPawn::StaticClass(), GetActorLocation(), GetActorRotation());
			if (SpectatorPawn) {
				Controller->Possess(SpectatorPawn);
			}
			else {
				Controller->UnPossess();
			}
		}
		SoundAtLocationOnMultiCast(DeadPlayerSound, GetActorLocation());
	}
}

bool APUHGCharacter::PlayerDeadOnMultiCast_Validate() {
	return true;
}

void APUHGCharacter::DestroyCharacterWithTimerServer_Implementation() {
	DestroyCharacterWithTimerMultiCast();
}

bool APUHGCharacter::DestroyCharacterWithTimerServer_Validate() {
	return true;
}

void APUHGCharacter::DestroyCharacterWithTimerMultiCast_Implementation() {
	if (IsValid(this)) {
		Destroy();
	}
	if (Role == ROLE_Authority) {
		GetWorld()->GetTimerManager().ClearTimer(CharacterDestroyHandle);
	}
}

bool APUHGCharacter::DestroyCharacterWithTimerMultiCast_Validate() {
	return true;
}

void APUHGCharacter::SpringArmCameraMovement(float DeltaTime) {
	FVector InterpOffset;
	float InterpLength;
	if (bIsAiming) {
		if (!bIsCrouching) {
			InterpOffset = UKismetMathLibrary::VInterpTo(SpringArmComp->SocketOffset, AimSocketLocation, DeltaTime, 10.f);
			InterpLength = UKismetMathLibrary::FInterpTo(SpringArmComp->TargetArmLength, AimTargetLength, DeltaTime, 10.f);
		}
		else {
			InterpOffset = UKismetMathLibrary::VInterpTo(SpringArmComp->SocketOffset, AimSocketLocation + CrouchSocketLocation, DeltaTime, 10.f);
			InterpLength = UKismetMathLibrary::FInterpTo(SpringArmComp->TargetArmLength, AimTargetLength, DeltaTime, 10.f);
		}
		SpringArmComp->TargetArmLength = InterpLength;
		SpringArmComp->SocketOffset = InterpOffset;
	}
	else {
		if (!bIsCrouching) {
			InterpOffset = UKismetMathLibrary::VInterpTo(SpringArmComp->SocketOffset, DefaultSocketLocation, DeltaTime, 10.f);
			InterpLength = UKismetMathLibrary::FInterpTo(SpringArmComp->TargetArmLength, DefaultTargetLength, DeltaTime, 10.f);
		}
		else {
			InterpOffset = UKismetMathLibrary::VInterpTo(SpringArmComp->SocketOffset, CrouchSocketLocation, DeltaTime, 10.f);
			InterpLength = UKismetMathLibrary::FInterpTo(SpringArmComp->TargetArmLength, DefaultTargetLength, DeltaTime, 10.f);
		}
		SpringArmComp->TargetArmLength = InterpLength;
		SpringArmComp->SocketOffset = InterpOffset;
	}
}
