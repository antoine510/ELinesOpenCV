#include "BP/OpenCVPluginBP.h"

UOpenCVTask::UOpenCVTask(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}

void UOpenCVTask::CreateTask(EOpenCVTasks workload,
					   FIntPoint imageSize,
					   UTextureRenderTarget2D* input,
					   UTexture2D*& output,
					   UOpenCVTask*& task) {
	task = NewObject<UOpenCVTask>();
	switch(workload) {
	case EOpenCVTasks::ElectricLines:
		task->_workload = std::make_shared<ElectricLinesWorkload>(imageSize, input, output);
		break;
	default:
		checkNoEntry();
	}
}

void UOpenCVTask::StartTask(const FProcessDelegate& completed) {
	if(!ensure(_IsReady())) return;
	_delegate = completed;
	_workload->PreSync();
	asyncTask.SetTask(std::bind(&Workload::Async, _workload),
					  [this]() { _delegate.ExecuteIfBound(_workload->PostSync()); });
}

void UOpenCVTask::SetTaskParameter(EElectricLinesParams param, float value) {
	if(!ensure(IsValid())) return;
	auto& wl = static_cast<ElectricLinesWorkload&>(*_workload);
	switch(param) {
	case EElectricLinesParams::rho:
		Workload::SetParameter(wl.rho, (double)value);
		break;
	case EElectricLinesParams::theta:
		Workload::SetParameter(wl.theta, (double)value);
		break;
	case EElectricLinesParams::minLineLength:
		Workload::SetParameter(wl.minLineLength, (double)value);
		break;
	case EElectricLinesParams::maxLineGap:
		Workload::SetParameter(wl.maxLineGap, (double)value);
		break;
	case EElectricLinesParams::thresholdCanny1:
		Workload::SetParameter(wl.thresholdCanny1, (double)value);
		break;
	case EElectricLinesParams::thresholdCanny2:
		Workload::SetParameter(wl.thresholdCanny2, (double)value);
		break;
	case EElectricLinesParams::thresholdHough:
		Workload::SetParameter(wl.thresholdHough, (int)value);
		break;
	default:
		checkNoEntry();
	}
	if(!_workload->CheckParameters()) {
		LogMessageOnScreen("Invalid task parameters");
	}
}

