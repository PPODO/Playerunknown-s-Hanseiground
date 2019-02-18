#include "TimeBombActorComponent.h"
#include "PUHGCharacter.h"
#include "GameFramework/GameNetworkManager.h"
#include "Components/TimelineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Curves/CurveFloat.h"
#include "ConstructorHelpers.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UTimeBombActorComponent::UTimeBombActorComponent() {
	ConstructorHelpers::FObjectFinder<UCurveFloat> DilationEn(L"CurveFloat'/Game/BluePrint/Projectile_BP/Enable_Dilation.Enable_Dilation'");
	ConstructorHelpers::FObjectFinder<UCurveFloat> DilationDis(L"CurveFloat'/Game/BluePrint/Projectile_BP/Disable_Dilation.Disable_Dilation'");
	ConstructorHelpers::FObjectFinder<USoundCue> ReverseTimeObject(L"SoundCue'/Game/ProjectilesPack/Sounds/Common/Player/TimeReverse_Cue.TimeReverse_Cue'");

	if (DilationEn.Succeeded()) {
		Enable_Dilation = DilationEn.Object;
	}
	if (DilationDis.Succeeded()) {
		Disable_Dilation = DilationDis.Object;
	}
	if (ReverseTimeObject.Succeeded()) {
		ReverseSound = ReverseTimeObject.Object;
	}

	EnBlendTimeLine = CreateDefaultSubobject<UTimelineComponent>(L"Enable Timeline");
	DisBlendTimeLine = CreateDefaultSubobject<UTimelineComponent>(L"Disable Timeline");

	FOnTimelineFloat DilationCurveEn;
	FOnTimelineEvent EndEnBlend;
	DilationCurveEn.BindUFunction(this, FName("UpdateDilationValue"));
	EndEnBlend.BindUFunction(this, FName("ReverseTimeBlend"));
	EnBlendTimeLine->AddInterpFloat(Enable_Dilation, DilationCurveEn);
	EnBlendTimeLine->SetTimelineFinishedFunc(EndEnBlend);

	FOnTimelineFloat DilationCurveDis;
	FOnTimelineEvent EndDisBlend;
	DilationCurveDis.BindUFunction(this, FName("UpdateDilationValue"));
	EndDisBlend.BindUFunction(this, FName("EndReverseTime"));
	DisBlendTimeLine->AddInterpFloat(Disable_Dilation, DilationCurveDis);
	DisBlendTimeLine->SetTimelineFinishedFunc(EndDisBlend);
}

void UTimeBombActorComponent::UpdateDilationValue(float Value) {
	if (DilationActor) {
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), Value);
		DilationActor->CustomTimeDilation = 1 / Value;
	}
}

void UTimeBombActorComponent::StartTimeDilationEnable(AActor* Target) {
	if (Target) {
		DilationActor = Target;
		EnBlendTimeLine->Play();
	}
}

void UTimeBombActorComponent::ReverseTimeBlend() {
	DisBlendTimeLine->Play();
	UGameplayStatics::PlaySound2D(GetWorld(), ReverseSound);
}

void UTimeBombActorComponent::EndReverseTime() {
	if (Cast<APUHGCharacter>(DilationActor)) {
		Cast<APUHGCharacter>(DilationActor)->SetUseTickDeltaTime(false);
	}
	if (GetOwner()) {
		GetOwner()->Destroy();
	}
	DestroyComponent();
}