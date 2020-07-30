#pragma once

#include <CoreTypes.h>
#include <Engine/Texture2D.h>

#include <future>

#pragma push_macro("check")
#undef check
#pragma warning( disable : 4946 )
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#pragma warning( default : 4946 )
#pragma pop_macro("check")

#include "OpenCV.h"

extern OPENCV_API struct FLogCategoryOpenCVLog : public FLogCategory<ELogVerbosity::Log, ELogVerbosity::All> {
	FLogCategoryOpenCVLog() : FLogCategory(TEXT("OpenCV Log")) {}
} OpenCVLog;

namespace OpenCV {

void OPENCV_API LogMessage(const FString& msg);
void OPENCV_API LogMessageOnScreen(const FString& msg);

class OPENCV_API Workload {
public:
	Workload(UTextureRenderTarget2D* input, UTexture2D* output) : InTex(input), OutTex(output), _pendingDeletion(false) {}
	virtual ~Workload() = default;

	virtual void PreSync() = 0;
	virtual void Async() = 0;
	virtual UTexture2D* PostSync() = 0;

	virtual bool CheckParameters() = 0;

	template<typename T>
	static void SetParameter(std::atomic<T>& param, T value) {
		param.store(value, std::memory_order_relaxed);
	}

	void SuicideOnFinished() { _pendingDeletion.store(true, std::memory_order_relaxed); }

protected:
	static UTexture2D* CreateTexture(int width, int height, EPixelFormat format);
	static void UpdateTexture(UTexture2D* dst, cv::Mat src);
	static void ReadPixels(UTextureRenderTarget2D* src, cv::Mat dst, std::promise<void>* promise);

	UTextureRenderTarget2D* InTex;
	UTexture2D* OutTex;

	std::atomic<bool> _pendingDeletion;
};

class OPENCV_API ElectricLinesWorkload : public Workload {
public:
	ElectricLinesWorkload(FIntPoint outSize, UTextureRenderTarget2D* input, UTexture2D*& output) :
		Workload(input, CreateTexture(outSize.X, outSize.Y, PF_G8)),
		_inpMat(outSize.X, outSize.Y, CV_8UC1),
		_outMat(outSize.X, outSize.Y, CV_8UC1),
		_edgesMat(outSize.X, outSize.Y, CV_8UC1) {

		output = OutTex;
	}

	void PreSync() override {
		promise = new std::promise<void>();
		ReadPixels(InTex, _inpMat, promise);
	}

	void Async() override;

	UTexture2D* PostSync() override {
		UpdateTexture(OutTex, _outMat);
		return OutTex;
	}

	bool CheckParameters() override {
		return rho >= 1.f && theta >= 0.01f;
	}

	std::atomic<double> rho{1}, theta{0.017}, minLineLength{100}, maxLineGap{10}, thresholdCanny1{100}, thresholdCanny2{200};
	std::atomic<int> thresholdHough{5};

protected:
	cv::Mat _inpMat, _outMat, _edgesMat;
	std::promise<void>* promise = nullptr;
};


}
