#include "JunYeong_Character.h"
#include "TimeBombProjectile.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "ConstructorHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

AJunYeong_Character::AJunYeong_Character() {
	ConstructorHelpers::FObjectFinder<USkeletalMesh> PlayerMeshObject(L"SkeletalMesh'/Game/AnimStarterPack/UE4_Mannequin/Mesh/SK_Mannequin.SK_Mannequin'");
	ConstructorHelpers::FObjectFinder<USkeletalMesh> PlayerGunObject(L"SkeletalMesh'/Game/Mesh/FPWeapon/Mesh/SK_FPGun.SK_FPGun'");
	ConstructorHelpers::FClassFinder<UAnimInstance> AnimInstObject(L"AnimBlueprint'/Game/BluePrint/InGame/Animations/BP_PUHGAnimClass'");
	ConstructorHelpers::FObjectFinder<UMaterial> BodyMaterialObject(L"Material'/Game/Materials/Character_Color/Yune/Yune_Body.Yune_Body'");
	ConstructorHelpers::FObjectFinder<USoundCue> DeadSoundObject(L"SoundCue'/Game/ProjectilesPack/Sounds/Common/Player/Jun_Dead_Cue.Jun_Dead_Cue'");
	ConstructorHelpers::FObjectFinder<USoundCue> ReloadSoundObject(L"SoundCue'/Game/ProjectilesPack/Sounds/Common/Player/Jun_Reload_Cue.Jun_Reload_Cue'");
	ConstructorHelpers::FObjectFinder<USoundCue> AbilityPlayerObject1(L"SoundCue'/Game/ProjectilesPack/Sounds/Common/Player/Jun_Ability_1_Cue.Jun_Ability_1_Cue'");
	ConstructorHelpers::FObjectFinder<USoundCue> AbilityPlayerObject2(L"SoundCue'/Game/ProjectilesPack/Sounds/Common/Player/Jun_Ability_2_Cue.Jun_Ability_2_Cue'");
	ConstructorHelpers::FObjectFinder<USoundCue> AbilityStartObject1(L"SoundCue'/Game/ProjectilesPack/Sounds/Common/Player/TimeStop_1_Cue.TimeStop_1_Cue'");
	ConstructorHelpers::FObjectFinder<USoundCue> AbilityStartObject2(L"SoundCue'/Game/ProjectilesPack/Sounds/Common/Player/TimeStop_2_Cue.TimeStop_2_Cue'");
	ConstructorHelpers::FObjectFinder<USoundCue> AbilityEndObject(L"SoundCue'/Game/ProjectilesPack/Sounds/Common/Player/Jun_AbilityEnd_Cue.Jun_AbilityEnd_Cue'");

	if (DeadSoundObject.Succeeded()) {
		DeadPlayerSound = DeadSoundObject.Object;
	}
	if (ReloadSoundObject.Succeeded()) {
		ReloadingPlayerSound = ReloadSoundObject.Object;
	}

	if (AbilityPlayerObject1.Succeeded()) {
		AbilitySounds.Add(AbilityPlayerObject1.Object);
	}
	if (AbilityPlayerObject2.Succeeded()) {
		AbilitySounds.Add(AbilityPlayerObject2.Object);
	}
	if (AbilityStartObject1.Succeeded()) {
		TimeStopSounds.Add(AbilityStartObject1.Object);
	}
	if (AbilityStartObject2.Succeeded()) {
		TimeStopSounds.Add(AbilityStartObject2.Object);
	}
	if (AbilityEndObject.Succeeded()) {
		AbilityEndSound = AbilityEndObject.Object;
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

	AbilityProjectile = ATimeBombProjectile::StaticClass();
	PrimaryActorTick.bCanEverTick = true;
}

void AJunYeong_Character::BeginPlay() {
	APUHGCharacter::BeginPlay();

}

void AJunYeong_Character::Tick(float DeltaTime) {
	APUHGCharacter::Tick(DeltaTime);

}

void AJunYeong_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	APUHGCharacter::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(L"Ability", IE_Pressed, this, &AJunYeong_Character::UseAbility);
}

void AJunYeong_Character::Landed(const FHitResult & Hit) {
	APUHGCharacter::Landed(Hit);

}

void AJunYeong_Character::PossessedBy(AController * Controller) {
	APUHGCharacter::PossessedBy(Controller);

}

void AJunYeong_Character::UseAbility() {
	if (!bCoolTime) {
		bCoolTime = true;
		PlayStartSoundOnMulti();
		GetWorld()->GetTimerManager().SetTimer(AbilityTimerHandle, this, &AJunYeong_Character::TimeBombOnServer, 1.f, false);
		GetWorld()->GetTimerManager().SetTimer(AbilityCoolTime, this, &AJunYeong_Character::CoolTimeTimer, 5.f, false);
	}
}

void AJunYeong_Character::TimeBombOnServer_Implementation() {
	GetWorld()->GetTimerManager().ClearTimer(AbilityTimerHandle);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Owner = this;

	ATimeBombProjectile* TimeProjectile = GetWorld()->SpawnActor<ATimeBombProjectile>(AbilityProjectile, GetActorLocation(), GetActorRotation(), Params);
	if (TimeProjectile) {
		PlayTimeStopSoundOnMulti();
	}
}

bool AJunYeong_Character::TimeBombOnServer_Validate() {
	return true;
}

void AJunYeong_Character::PlayStartSoundOnMulti_Implementation() {
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), AbilitySounds[FMath::RandRange(0, AbilitySounds.Num() - 1)], GetActorLocation());
}

bool AJunYeong_Character::PlayStartSoundOnMulti_Validate() {
	return true;
}

void AJunYeong_Character::PlayTimeStopSoundOnMulti_Implementation() {
	UGameplayStatics::PlaySound2D(GetWorld(), TimeStopSounds[FMath::RandRange(0, TimeStopSounds.Num() - 1)]);
}

bool AJunYeong_Character::PlayTimeStopSoundOnMulti_Validate() {
	return true;
}