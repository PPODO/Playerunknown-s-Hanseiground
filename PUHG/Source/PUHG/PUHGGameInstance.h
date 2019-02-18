#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PUHGGameInstance.generated.h"

UCLASS()
class PUHG_API UPUHGGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UPUHGGameInstance();

public:
	UPROPERTY(BlueprintReadWrite)
		int32 CurrentPlayer;

	UPROPERTY(BlueprintReadWrite)
		TMap<FString, TSubclassOf<class ACharacter>> PlayerChooseCharacter;

};
