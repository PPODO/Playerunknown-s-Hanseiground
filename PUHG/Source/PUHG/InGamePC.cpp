#include "InGamePC.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

AInGamePC::AInGamePC() {

	SetReplicates(true);
}

void AInGamePC::PlayerTick(float DeltaTime) {
	Super::PlayerTick(DeltaTime);

}

/* Cross Hair */

void AInGamePC::TickSpreadMove(float DeltaTime) {
	if (SpreadCurrent >= SpreadMin) {
		Decrease_Spread(DeltaTime * SpreadDecrease);
	}
}

void AInGamePC::Increase_Spread(float IncreaseAmount) {
	const float Add_Spread = SpreadCurrent + IncreaseAmount;
	if (Add_Spread >= SpreadMax) {
		SpreadCurrent = SpreadMax;
	}
	else {
		SpreadCurrent = Add_Spread;
	}
}

void AInGamePC::CameraRecoil_Implementation(float ScaleShake, float ScaleRecoil) {

}

void AInGamePC::Decrease_Spread(float DecreaseAmount) {
	const float Delta_Spread = SpreadCurrent - DecreaseAmount;
	if (Delta_Spread <= SpreadMin) {
		SpreadCurrent = SpreadMin;
	}
	else {
		SpreadCurrent = Delta_Spread;
	}
}

/* Server */

void AInGamePC::DestroySelectedCharacter_Implementation() {
	if (GetPawn()) {
		GetPawn()->Destroy();
	}
	if (GetCharacter()) {
		GetCharacter()->Destroy();
	}
}

bool AInGamePC::DestroySelectedCharacter_Validate() {
	return true;
}

void AInGamePC::TeleportOnServer_Implementation(FVector NewLocation) {
	MulticastTeleport(NewLocation);
}

bool AInGamePC::TeleportOnServer_Validate(FVector New) {
	return true;
}

void AInGamePC::MulticastTeleport_Implementation(FVector NewLocation) {
	if (GetPawn()) {
		GetPawn()->SetActorLocation(NewLocation);
	}
}

bool AInGamePC::MulticastTeleport_Validate(FVector New) {
	return true;
}