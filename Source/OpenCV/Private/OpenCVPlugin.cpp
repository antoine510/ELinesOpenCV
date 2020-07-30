#include "OpenCVPlugin.h"
#include <Engine/Engine.h>
#include <Engine/TextureRenderTarget2D.h>

FLogCategoryOpenCVLog OpenCVLog;

void OpenCV::LogMessage(const FString& msg) {
	UE_LOG(OpenCVLog, Log, TEXT("%s"), *msg);
}

void OpenCV::LogMessageOnScreen(const FString& msg) {
	LogMessage(msg);
	if(GEngine != nullptr) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, msg);
}


UTexture2D* OpenCV::Workload::CreateTexture(int width, int height, EPixelFormat format) {
	UTexture2D* image = UTexture2D::CreateTransient(width, height, format);
	image->UpdateResource();

	image->AddressX = TextureAddress::TA_Wrap;
	image->AddressY = TextureAddress::TA_Wrap;
	image->Filter = TextureFilter::TF_Default;
	image->SRGB = 1;
	image->RefreshSamplerStates();

	return image;
}

void OpenCV::Workload::UpdateTexture(UTexture2D* dst, cv::Mat src) {
	ENQUEUE_RENDER_COMMAND(UpdateDynamicTextureCode)([dst, src](FRHICommandListImmediate& RHICmdList) {
		FUpdateTextureRegion2D region(0, 0, 0, 0, src.rows, src.cols);

		auto resource = (FTexture2DResource*)dst->Resource;
		RHICmdList.UpdateTexture2D(resource->GetTexture2DRHI(), 0, region, src.rows, src.data);
													 }
	);
}

void OpenCV::Workload::ReadPixels(UTextureRenderTarget2D* src, cv::Mat dst, std::promise<void>* promise) {
	ENQUEUE_RENDER_COMMAND(ReadTextureCode)([src, dst, promise](FRHICommandListImmediate& RHICmdList) {
		FTexture2DRHIRef texture = ((FTextureRenderTarget2DResource*)src->GetRenderTargetResource())->GetTextureRHI();
		uint32 Stride = 0;
		uchar* TexData = (uchar*)RHICmdList.LockTexture2D(texture, 0, RLM_ReadOnly, Stride, false, false);
		if(Stride != dst.rows) {
			for(int i = 0; i < dst.cols; ++i) {
				memcpy(dst.data + i * dst.rows, TexData + i * Stride, dst.rows);
			}
		} else {
			memcpy(dst.data, TexData, Stride * dst.cols);
		}
		RHICmdList.UnlockTexture2D(texture, 0, false, false);
		promise->set_value();
	});
}

#include <chrono>

void OpenCV::ElectricLinesWorkload::Async() {
	using namespace std::chrono;

	static unsigned perf_loops = 0;
	static microseconds perf_timesum, perf_processSum;

	auto start_time = system_clock::now();

	promise->get_future().wait();
	delete promise;
	cv::Mat lines;
	cv::Canny(_inpMat, _edgesMat, thresholdCanny1.load(std::memory_order_relaxed), thresholdCanny2.load(std::memory_order_relaxed));
	cv::HoughLinesP(_edgesMat, lines, rho.load(std::memory_order_relaxed), theta.load(std::memory_order_relaxed),
					thresholdHough.load(std::memory_order_relaxed), minLineLength.load(std::memory_order_relaxed),
					maxLineGap.load(std::memory_order_relaxed));

	perf_processSum += duration_cast<decltype(perf_timesum)>(system_clock::now() - start_time);

	memset(_outMat.data, 0, _outMat.dataend - _outMat.datastart);
	for(int i = 0; i < lines.rows; ++i) {
		cv::line(_outMat, cv::Point(lines.at<int>(i, 0), lines.at<int>(i, 1)), cv::Point(lines.at<int>(i, 2), lines.at<int>(i, 3)), cv::Scalar(255), 2);
	}

	perf_timesum += duration_cast<decltype(perf_timesum)>(system_clock::now() - start_time);
	perf_loops++;

	if(perf_loops % 100 == 0) {
		LogMessage(L"Average OpenCV time (processing/total): " +
					FString::FromInt(perf_processSum.count() / perf_loops) +
					L"/" +
					FString::FromInt(perf_timesum.count() / perf_loops) +
					L" us");
	}

	if(_pendingDeletion.load(std::memory_order_relaxed)) delete this;
}
