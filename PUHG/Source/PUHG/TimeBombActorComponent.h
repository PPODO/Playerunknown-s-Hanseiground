#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimeBombActorComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PUHG_API UTimeBombActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTimeBombActorComponent();

private:
	UPROPERTY(VisibleDefaultsOnly)
		class UCurveFloat* Enable_Dilation;
	UPROPERTY(VisibleDefaultsOnly)
		class UCurveFloat* Disable_Dilation;

	UPROPERTY(VisibleDefaultsOnly)
		class UTimelineComponent* EnBlendTimeLine;
	UPROPERTY(VisibleDefaultsOnly)
		class UTimelineComponent* DisBlendTimeLine;
	UPROPERTY(VisibleDefaultsOnly)
		class USoundCue* ReverseSound;
	
	class AActor* DilationActor;

	UFUNCTION()
		void UpdateDilationValue(float Value);

	UFUNCTION()
		void ReverseTimeBlend();

	UFUNCTION()
		void EndReverseTime();
		
public:
	void StartTimeDilationEnable(class AActor* Target);

};
