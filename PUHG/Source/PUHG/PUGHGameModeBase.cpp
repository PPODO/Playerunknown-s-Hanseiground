#include "PUGHGameModeBase.h"
#include "SpawnPoint.h"
#include "InGameSpawnPoint.h"
#include "TimerManager.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void APUGHGameModeBase::BeginPlay() {
	Super::BeginPlay();

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnPoint::StaticClass(), AllSpawnPoints);
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), BeginGameSpawnPoint);
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AInGameSpawnPoint::StaticClass(), InGameSpawnPoint);

	for (int i = 0; i < BeginGameSpawnPoint.Num(); i++) {
		APlayerStart* StartPoint = Cast<APlayerStart>(BeginGameSpawnPoint[i]);
		if (StartPoint) {
			if (StartPoint->PlayerStartTag == "Lobby") {
				BeginGameSpawnPoint.RemoveAt(i);
			}
		}
	}
	InGame = false;
}

void APUGHGameModeBase::UpdateKillLog_Implementation(const FString& KillPlayerName, const FString& KilledPlayerName, class AController* DeadPlayerController) {

}

FVector APUGHGameModeBase::PickUpRespawnPoint() {
	int32 Index = 0;
	if (!InGame) {
		Index = FMath::RandRange(0, AllSpawnPoints.Num() - 1);
	}
	else {
		Index = FMath::RandRange(0, InGameSpawnPoint.Num() - 1);
	}
	
	FVector SpawnPoint = (InGame ? InGameSpawnPoint[Index]->GetActorLocation() : AllSpawnPoints[Index]->GetActorLocation());
	return SpawnPoint;
}