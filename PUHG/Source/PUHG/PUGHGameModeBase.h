#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PUGHGameModeBase.generated.h"

UCLASS()
class PUHG_API APUGHGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintReadWrite)
		bool InGame;

	UPROPERTY(BlueprintReadWrite)
		int32 RespawnTime = 10.f;

	UPROPERTY(BlueprintReadWrite)
		TArray<class AActor*> AllSpawnPoints;

	UPROPERTY(BlueprintReadWrite)
		TArray<class AActor*> BeginGameSpawnPoint;

	UPROPERTY(BlueprintReadWrite)
		TArray<class AActor*> InGameSpawnPoint;

public:
	UFUNCTION(BlueprintNativeEvent)
		void UpdateKillLog(const FString& KillPlayerName, const FString& KilledPlayerName, class AController* DeadPlayerController);

	UFUNCTION(BlueprintCallable)
		FVector PickUpRespawnPoint();

};
