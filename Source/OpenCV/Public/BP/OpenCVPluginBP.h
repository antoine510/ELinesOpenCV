#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Tickable.h"

#include "OpenCVPlugin.h"
#include "OpenCVUtils.h"

#include "OpenCVPluginBP.generated.h"


UENUM(BlueprintType)
enum class EOpenCVErrorState : uint8 {
	Ok,
	Fail
};

UENUM(BlueprintType)
enum class EOpenCVTasks : uint8 {
	ElectricLines
};

UENUM(BlueprintType)
enum class EElectricLinesParams : uint8 {
	rho, theta, minLineLength, maxLineGap, thresholdCanny1, thresholdCanny2, thresholdHough
};


UCLASS(BlueprintType)
class OPENCV_API UOpenCVTask : public UObject, public FTickableGameObject {
	GENERATED_UCLASS_BODY()
public:
	DECLARE_DYNAMIC_DELEGATE_OneParam(FProcessDelegate, UTexture2D*, result);

	TStatId GetStatId() const override { return GetStatID(); }
	bool IsTickable() const override {
		return asyncEngine.IsTickable();
	}

	void Tick(float) override {
		asyncEngine.Tick();
	}

	void BeginDestroy() override {
		asyncEngine.StopTask();
		delete _workload;
		Super::BeginDestroy();
	}


	UFUNCTION(BlueprintCallable, Category = "OpenCV")
		static void CreateTask(EOpenCVTasks workload,
							   FIntPoint imageSize,
							   UTextureRenderTarget2D* input,
							   UTexture2D*& output,
							   UOpenCVTask*& task) {
		task = NewObject<UOpenCVTask>();
		switch(workload) {
		case EOpenCVTasks::ElectricLines:
			task->_workload = new OpenCV::ElectricLinesWorkload(imageSize, input, output);
			break;
		default:
			checkNoEntry();
		}
	}

	UFUNCTION(BlueprintCallable)
		void StartTask(const FProcessDelegate& completed) {
		if(!ensure(_IsReady())) return;
		_delegate = completed;
		_workload->PreSync();
		asyncEngine.SetTask(std::bind(&OpenCV::Workload::Async, _workload),
							[this]() { _delegate.ExecuteIfBound(_workload->PostSync()); });
	}

	UFUNCTION(BlueprintCallable)
		void SetTaskParameter(EElectricLinesParams param, float value) {
		if(!ensure(IsValid())) return;
		switch(param) {
		case EElectricLinesParams::rho:
			OpenCV::Workload::SetParameter(((OpenCV::ElectricLinesWorkload*)_workload)->rho, (double)value);
			break;
		case EElectricLinesParams::theta:
			OpenCV::Workload::SetParameter(((OpenCV::ElectricLinesWorkload*)_workload)->theta, (double)value);
			break;
		case EElectricLinesParams::minLineLength:
			OpenCV::Workload::SetParameter(((OpenCV::ElectricLinesWorkload*)_workload)->minLineLength, (double)value);
			break;
		case EElectricLinesParams::maxLineGap:
			OpenCV::Workload::SetParameter(((OpenCV::ElectricLinesWorkload*)_workload)->maxLineGap, (double)value);
			break;
		case EElectricLinesParams::thresholdCanny1:
			OpenCV::Workload::SetParameter(((OpenCV::ElectricLinesWorkload*)_workload)->thresholdCanny1, (double)value);
			break;
		case EElectricLinesParams::thresholdCanny2:
			OpenCV::Workload::SetParameter(((OpenCV::ElectricLinesWorkload*)_workload)->thresholdCanny2, (double)value);
			break;
		case EElectricLinesParams::thresholdHough:
			OpenCV::Workload::SetParameter(((OpenCV::ElectricLinesWorkload*)_workload)->thresholdHough, (int)value);
			break;
		default:
			checkNoEntry();
		}
		if(!_workload->CheckParameters()) {
			OpenCV::LogMessageOnScreen("Invalid task parameters");
		}
	}

	bool IsValid() const { return _workload != nullptr; }

	/**
	* Test validity of OpenCVTask
	*
	* @param	Test			The object to test
	* @return	Return true if the object is usable
	*/
	UFUNCTION(BlueprintPure, Meta = (CompactNodeTitle = "IsValid"))
		static bool IsValid(const UOpenCVTask* Test) { return IsValidT(Test); }


	/** ASYNC calls facilities **/
	OpenCV::AsyncEngine asyncEngine;

private:
	bool _IsReady() const {
		if(!IsValid()) OpenCV::LogMessageOnScreen(L"Using invalid StreamSource");
		else if(!_workload->CheckParameters()) OpenCV::LogMessageOnScreen(L"OpenCV task has invalid parameters");
		else if(!asyncEngine.IsAvailable()) OpenCV::LogMessageOnScreen(L"Async operation in progress on this StreamSource");
		else return true;
		return false;
	}

	FProcessDelegate _delegate;
	OpenCV::Workload* _workload;
};


