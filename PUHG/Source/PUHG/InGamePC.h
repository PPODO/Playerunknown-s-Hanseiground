#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InGamePC.generated.h"

UCLASS()
class PUHG_API AInGamePC : public APlayerController
{
	GENERATED_BODY()

public:
	AInGamePC();

protected:
	virtual void PlayerTick(float Delta) override;

private:
	float SpreadDecrease = 1.f;

private:
	void Decrease_Spread(float DecreaseAmount);

public:
	UPROPERTY(BlueprintReadWrite)
		float SpreadCurrent = 0.2f;
	UPROPERTY(BlueprintReadWrite)
		float SpreadMax = 0.3f;
	UPROPERTY(BlueprintReadWrite)
		float SpreadMin = 0.15f;

public:
	void TickSpreadMove(float);
	void Increase_Spread(float IncreaseAmount);

/* Server!! */
public:
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void DestroySelectedCharacter();

public:
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void TeleportOnServer(FVector New);

	UFUNCTION(NetMulticast, Reliable, WithValidation, BlueprintCallable)
		void MulticastTeleport(FVector New);

public:
	UFUNCTION(BlueprintNativeEvent)
		void CameraRecoil(float ScaleShake, float ScaleRecoil);

};
