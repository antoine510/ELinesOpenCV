#pragma once

#include <Tickable.h>

#include "OpenCVPlugin.h"
#include "OpenCVUtils.h"

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
		return asyncTask.IsTickable();
	}

	void Tick(float) override {
		asyncTask.Tick();
	}

	void BeginDestroy() override {
		asyncTask.StopTask();
		Super::BeginDestroy();
	}


	UFUNCTION(BlueprintCallable, Category = "OpenCV")
		static void CreateTask(EOpenCVTasks workload,
							   FIntPoint imageSize,
							   UTextureRenderTarget2D* input,
							   UTexture2D*& output,
							   UOpenCVTask*& task);

	UFUNCTION(BlueprintCallable)
		void StartTask(const FProcessDelegate& completed);

	UFUNCTION(BlueprintCallable)
		void SetTaskParameter(EElectricLinesParams param, float value);

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
	OpenCV::AsyncTask asyncTask;

private:
	bool _IsReady() const {
		if(!IsValid()) LogMessageOnScreen(L"Using invalid StreamSource");
		else if(!_workload->CheckParameters()) LogMessageOnScreen(L"OpenCV task has invalid parameters");
		else if(!asyncTask.IsAvailable()) LogMessageOnScreen(L"Async operation in progress on this StreamSource");
		else return true;
		return false;
	}

	FProcessDelegate _delegate;
	std::shared_ptr<Workload> _workload;
};


