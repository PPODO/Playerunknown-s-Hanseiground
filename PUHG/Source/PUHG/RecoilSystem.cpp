#include "RecoilSystem.h"

URecoilSystem::URecoilSystem() {

	OscillationDuration = 0.2f;

	RotOscillation.Pitch.Amplitude = 2.f;
	RotOscillation.Pitch.Frequency = 2;
	RotOscillation.Yaw.Amplitude = 2.f;
	RotOscillation.Yaw.Frequency = 2;

	LocOscillation.X.Amplitude = 15.f;
	LocOscillation.X.Frequency = 1.f;
	LocOscillation.Z.Amplitude = 5.f;
	LocOscillation.Z.Frequency = 15.f;

}