#include "SungHun_Character.h"
#include "MultiHomingProjectile.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "ConstructorHelpers.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

ASungHun_Character::ASungHun_Character() {
	ConstructorHelpers::FObjectFinder<USkeletalMesh> PlayerMeshObject(L"SkeletalMesh'/Game/AnimStarterPack/UE4_Mannequin/Mesh/SK_Mannequin.SK_Mannequin'");
	ConstructorHelpers::FObjectFinder<USkeletalMesh> PlayerGunObject(L"SkeletalMesh'/Game/Mesh/FPWeapon/Mesh/SK_FPGun.SK_FPGun'");
	ConstructorHelpers::FObjectFinder<UMaterial> BodyMaterialObject(L"Material'/Game/Materials/Character_Color/Han/Han_BOdy.Han_Body'");
	ConstructorHelpers::FClassFinder<UAnimInstance> AnimInstObject(L"AnimBlueprint'/Game/BluePrint/InGame/Animations/BP_PUHGAnimClass'");
	ConstructorHelpers::FObjectFinder<USoundCue> DeadSoundObject(L"SoundCue'/Game/ProjectilesPack/Sounds/Common/Player/Han_Dead_Cue.Han_Dead_Cue'");
	ConstructorHelpers::FObjectFinder<USoundCue> ReloadSoundObject(L"SoundCue'/Game/ProjectilesPack/Sounds/Common/Player/Han_Reload_Cue.Han_Reload_Cue'");
	ConstructorHelpers::FObjectFinder<USoundCue> AbilitySoundObject(L"SoundCue'/Game/ProjectilesPack/Sounds/Common/Player/Han_Ability_Cue.Han_Ability_Cue'");

	if (DeadSoundObject.Succeeded()) {
		DeadPlayerSound = DeadSoundObject.Object;
	}
	if (ReloadSoundObject.Succeeded()) {
		ReloadingPlayerSound = ReloadSoundObject.Object;
	}
	if (AbilitySoundObject.Succeeded()) {
		AbilityPlayerSound = AbilitySoundObject.Object;
	}

	if (BodyMaterialObject.Succeeded()) {
		BodyMaterial = UMaterialInstanceDynamic::Create(BodyMaterialObject.Object, nullptr);
		if (GetMesh()) {
			GetMesh()->SetMaterial(0, BodyMaterial);
		}
	}

	if (PlayerMeshObject.Succeeded()) {
		USkeletalMeshComponent* TempMesh = GetMesh();
		TempMesh->SetSkeletalMesh(PlayerMeshObject.Object);
		TempMesh->SetRelativeLocation(FVector(0.f, -5.f, -90.f));
		TempMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
		TempMesh->SetNotifyRigidBodyCollision(true);
		if (AnimInstObject.Succeeded()) {
			GetMesh()->SetAnimInstanceClass(AnimInstObject.Class);
		}
	}

	if (PlayerGunObject.Succeeded()) {
		FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(L"FP Gun Mesh");
		FP_Gun->SetSkeletalMesh(PlayerGunObject.Object);
		FP_Gun->SetRelativeLocation(FVector(-10.f, 5.f, -2.5f));
		FP_Gun->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
		FP_Gun->SetupAttachment(GetMesh(), L"hand_r");

		FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(L"MuzzleLocation");
		FP_MuzzleLocation->SetupAttachment(FP_Gun);
		FP_MuzzleLocation->SetRelativeLocation(FVector(0.f, 60.f, 11.f));
	}

	SetReplicates(true);
	SetReplicateMovement(true);
	PrimaryActorTick.bCanEverTick = true;
}

void ASungHun_Character::BeginPlay() {
	APUHGCharacter::BeginPlay();

}

void ASungHun_Character::Tick(float DeltaTime) {
	APUHGCharacter::Tick(DeltaTime);

}

void ASungHun_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	APUHGCharacter::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(L"Ability", IE_Pressed, this, &ASungHun_Character::UseAbility);
	PlayerInputComponent->BindAction(L"ShootTrace", IE_Pressed, this, &ASungHun_Character::SetTarget);
}

void ASungHun_Character::Landed(const FHitResult & Hit) {
	APUHGCharacter::Landed(Hit);

}

void ASungHun_Character::PossessedBy(AController * Controller) {
	APUHGCharacter::PossessedBy(Controller);

}

void ASungHun_Character::BeginDestroy() {
	Super::BeginDestroy();

	if (HomingTargetActor) {
		if (Cast<AStaticMeshActor>(HomingTargetActor)) {
			Cast<AStaticMeshActor>(HomingTargetActor)->GetStaticMeshComponent()->SetRenderCustomDepth(false);
		}
		if (Cast<APUHGCharacter>(HomingTargetActor)) {
			Cast<APUHGCharacter>(HomingTargetActor)->GetMesh()->SetRenderCustomDepth(false);
		}
	}
}

void ASungHun_Character::UseAbility() {
	if (!bCoolTime) {
		AbilityOnServer(HomingTargetActor);
		bCoolTime = true;
		GetWorld()->GetTimerManager().SetTimer(AbilityCoolTime, this, &ASungHun_Character::CoolTimeTimer, 5.f);
	}
}

void ASungHun_Character::SetTarget() {
	FHitResult AbilityHitResult;
	FVector TraceStart = CameraComp->GetComponentLocation();
	FVector TraceEnd = CameraComp->GetComponentLocation() + CameraComp->GetForwardVector() * 10000.f;

	if (GetWorld()->LineTraceSingleByChannel(AbilityHitResult, TraceStart, TraceEnd, ECC_Visibility)) {
		HomingTargetActor = Cast<AActor>(AbilityHitResult.Actor);
		if (Cast<AStaticMeshActor>(HomingTargetActor)) {
			if (BeforeHomingTargetActor) {
				if (Cast<AStaticMeshActor>(BeforeHomingTargetActor)) {
					Cast<AStaticMeshActor>(BeforeHomingTargetActor)->GetStaticMeshComponent()->SetRenderCustomDepth(false);
				}
				if(Cast<APUHGCharacter>(BeforeHomingTargetActor)) {
					Cast<APUHGCharacter>(BeforeHomingTargetActor)->GetMesh()->SetRenderCustomDepth(false);
				}
			}
			Cast<AStaticMeshActor>(HomingTargetActor)->GetStaticMeshComponent()->SetRenderCustomDepth(true);
			BeforeHomingTargetActor = HomingTargetActor;
		}
		if(Cast<APUHGCharacter>(HomingTargetActor)) {
			if (BeforeHomingTargetActor) {
				if (Cast<APUHGCharacter>(BeforeHomingTargetActor)) {
					Cast<APUHGCharacter>(BeforeHomingTargetActor)->GetMesh()->SetRenderCustomDepth(false);
				}
				if (Cast<AStaticMeshActor>(BeforeHomingTargetActor)) {
					Cast<AStaticMeshActor>(BeforeHomingTargetActor)->GetStaticMeshComponent()->SetRenderCustomDepth(false);
				}
			}
			Cast<APUHGCharacter>(HomingTargetActor)->GetMesh()->SetRenderCustomDepth(true);
			BeforeHomingTargetActor = HomingTargetActor;
		}
	}
}

void ASungHun_Character::ShootTraceAndSetTargetOnMulti_Implementation(FHitResult Hit) {
	HomingTargetActor = Cast<AActor>(Hit.Actor);
}

bool ASungHun_Character::ShootTraceAndSetTargetOnMulti_Validate(FHitResult Hit) {
	return true;
}

void ASungHun_Character::ShootTraceAndSetTargetOnServer_Implementation(FHitResult HitResult) {
	ShootTraceAndSetTargetOnMulti(HitResult);
}

bool ASungHun_Character::ShootTraceAndSetTargetOnServer_Validate(FHitResult HitResult) {
	return true;
}

void ASungHun_Character::AbilityOnServer_Implementation(class AActor* Target) {
	FActorSpawnParameters Params;
	FVector SpawnLocation = FP_MuzzleLocation->GetComponentLocation();
	FRotator SpawnRotation = GetControlRotation();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMultiHomingProjectile* HomingProjectile = GetWorld()->SpawnActor<AMultiHomingProjectile>(AMultiHomingProjectile::StaticClass(), SpawnLocation, SpawnRotation, Params);
	if (HomingProjectile) {
		SoundAtLocationOnMultiCast(AbilityPlayerSound, GetActorLocation());
		if (Target) {
			HomingProjectile->SetHomingTarget(Target->FindComponentByClass<USceneComponent>());
			HomingProjectile->SetOwnerPlayerName(CurrentPlayerName);
		}
	}
}

bool ASungHun_Character::AbilityOnServer_Validate(class AActor* Target) {
	return true;
}