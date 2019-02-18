#pragma once

#include <future>

#pragma push_macro("check")
#undef check
#pragma warning( disable : 4946 )
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#pragma warning( default : 4946 )
#pragma pop_macro("check")

#include "OpenCV.h"
#include "Kismet/BlueprintFunctionLibrary.h"

extern OPENCV_API struct FLogCategoryOpenCVLog : public FLogCategory<ELogVerbosity::Log, ELogVerbosity::All> {
	FLogCategoryOpenCVLog() : FLogCategory(TEXT("OpenCV Log")) {}
} OpenCVLog;

namespace OpenCV {

void OPENCV_API LogMessage(const FString& msg);
void OPENCV_API LogMessageOnScreen(const FString& msg);
void OPENCV_API LogMessage(const char* msg);
void OPENCV_API LogMessageOnScreen(const char* msg);

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
	void StageTexture();
	static void ReadPixels(UTextureRenderTarget2D* src, cv::Mat dst, std::promise<void>* promise);

	UTextureRenderTarget2D* InTex;
	//FTexture2DRHIRef Staging;
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

		/*ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			CreateStagingTexture,
			FTexture2DRHIRef&, stagedTex, Staging,
			FIntPoint, size, outSize,
			{
				FRHIResourceCreateInfo CreateInfo;
				stagedTex = RHICreateTexture2D(size.X, size.Y, PF_G8, 1, 1, TexCreate_CPUReadback | TexCreate_Dynamic | TexCreate_SRGB, CreateInfo);
			});*/
		output = OutTex;
	}

	void PreSync() override {
		promise = new std::promise<void>();
		ReadPixels(InTex, _inpMat, promise);
	}

	void Async() override {
		promise->get_future().wait();
		delete promise;
		cv::Mat lines;
		cv::Canny(_inpMat, _edgesMat, thresholdCanny1.load(std::memory_order_relaxed), thresholdCanny2.load(std::memory_order_relaxed));
		cv::HoughLinesP(_edgesMat, lines, rho.load(std::memory_order_relaxed), theta.load(std::memory_order_relaxed),
						thresholdHough.load(std::memory_order_relaxed), minLineLength.load(std::memory_order_relaxed),
						maxLineGap.load(std::memory_order_relaxed));
		memset(_outMat.data, 0, _outMat.dataend - _outMat.datastart);
		for(int i = 0; i < lines.rows; ++i) {
			cv::line(_outMat, cv::Point(lines.at<int>(i, 0), lines.at<int>(i, 1)), cv::Point(lines.at<int>(i, 2), lines.at<int>(i, 3)), cv::Scalar(255), 2);
		}
		if(_pendingDeletion.load(std::memory_order_relaxed)) delete this;
	}

	UTexture2D* PostSync() override {
		UpdateTexture(OutTex, _outMat);
		return OutTex;
	}

	bool CheckParameters() override {
		return rho >= 1.f && theta >= 0.01f;
	}

	std::atomic<double> rho, theta, minLineLength, maxLineGap, thresholdCanny1, thresholdCanny2;
	std::atomic<int> thresholdHough;

protected:
	cv::Mat _inpMat, _outMat, _edgesMat;
	std::promise<void>* promise = nullptr;
};


}
