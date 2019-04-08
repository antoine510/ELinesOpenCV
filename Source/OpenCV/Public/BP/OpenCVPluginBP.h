#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Tickable.h"

#include "OpenCVPlugin.h"
#include "OpenCVUtils.h"

#include <Engine/Classes/Components/ActorComponent.h>
#include <Engine/Classes/Components/SceneCaptureComponent2D.h>

#include "OpenCVPluginBP.generated.h"

using namespace OpenCV;

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
			task->_workload = std::make_shared<ElectricLinesWorkload>(imageSize, input, output);
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
		asyncEngine.SetTask(std::bind(&Workload::Async, _workload),
							[this]() { _delegate.ExecuteIfBound(_workload->PostSync()); });
	}

	UFUNCTION(BlueprintCallable)
		void SetTaskParameter(EElectricLinesParams param, float value) {
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
	AsyncEngine asyncEngine;

private:
	bool _IsReady() const {
		if(!IsValid()) LogMessageOnScreen(L"Using invalid StreamSource");
		else if(!_workload->CheckParameters()) LogMessageOnScreen(L"OpenCV task has invalid parameters");
		else if(!asyncEngine.IsAvailable()) LogMessageOnScreen(L"Async operation in progress on this StreamSource");
		else return true;
		return false;
	}

	FProcessDelegate _delegate;
	std::shared_ptr<Workload> _workload;
};

/*UCLASS(BlueprintType)
class OPENCV_API UOpenCVUtility : public UBlueprintFunctionLibrary {
	GENERATED_UCLASS_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "OpenCV", meta = (WorldContext = WorldContextObject))
		static void GetOpenCVManager(UObject* WorldContextObject, UOpenCVManagerBP*& manager) {
		if(GEngine == nullptr || GEngine->GetWorldFromContextObject(WorldContextObject) == nullptr) return;
		auto* world = GEngine->GetWorldFromContextObject(WorldContextObject);
		if(world->GetAuthGameMode() == nullptr) return;
		manager = world->GetAuthGameMode()->FindComponentByClass<UOpenCVManagerBP>();
	}

};*/

