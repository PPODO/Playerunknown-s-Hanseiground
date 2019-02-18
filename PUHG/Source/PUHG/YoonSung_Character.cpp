#include "YoonSung_Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ConstructorHelpers.h"

AYoonSung_Character::AYoonSung_Character() {
	ConstructorHelpers::FObjectFinder<USkeletalMesh> PlayerMeshObject(L"SkeletalMesh'/Game/AnimStarterPack/UE4_Mannequin/Mesh/SK_Mannequin.SK_Mannequin'");
	ConstructorHelpers::FObjectFinder<USkeletalMesh> PlayerGunObject(L"SkeletalMesh'/Game/Mesh/FPWeapon/Mesh/SK_FPGun.SK_FPGun'");
	ConstructorHelpers::FClassFinder<UAnimInstance> AnimInstObject(L"AnimBlueprint'/Game/BluePrint/InGame/Animations/BP_PUHGAnimClass'");
	ConstructorHelpers::FObjectFinder<UMaterial> BodyMaterialObject(L"Material'/Game/Materials/Character_Color/Yoon/Yoon_Body.Yoon_Body'");

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

	PrimaryActorTick.bCanEverTick = true;
}

void AYoonSung_Character::BeginPlay() {
	APUHGCharacter::BeginPlay();

}

void AYoonSung_Character::Tick(float DeltaTime) {
	APUHGCharacter::Tick(DeltaTime);

}

void AYoonSung_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	APUHGCharacter::SetupPlayerInputComponent(PlayerInputComponent);

}

void AYoonSung_Character::Landed(const FHitResult & Hit) {
	APUHGCharacter::Landed(Hit);

}

void AYoonSung_Character::PossessedBy(AController* PossessController) {
	APUHGCharacter::PossessedBy(PossessController);

}