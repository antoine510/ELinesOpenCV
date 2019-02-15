#include "OpenCVPCH.h"
#include "OpenCVPlugin.h"
#include <Engine.h>

FLogCategoryOpenCVLog OpenCVLog;

void OpenCV::LogMessage(const FString& msg) {
	UE_LOG(OpenCVLog, Log, TEXT("%s"), *msg);
}

void OpenCV::LogMessageOnScreen(const FString& msg) {
	LogMessage(msg);
	if(GEngine != nullptr) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, msg);
}

void OpenCV::LogMessage(const char* msg) {
	UE_LOG(OpenCVLog, Log, TEXT("%s"), UTF8_TO_TCHAR(msg));
}

void OpenCV::LogMessageOnScreen(const char* msg) {
	LogMessage(msg);
	if(GEngine != nullptr) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, UTF8_TO_TCHAR(msg));
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
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		UpdateDynamicTextureCode,
		UTexture2D*, pDst, dst,
		cv::Mat, srcMat, src,
		{
			FUpdateTextureRegion2D region(0, 0, 0, 0, srcMat.rows, srcMat.cols);

			auto resource = (FTexture2DResource*)pDst->Resource;
			RHIUpdateTexture2D(resource->GetTexture2DRHI(), 0, region, srcMat.rows, srcMat.data);
		});
}

void OpenCV::Workload::StageTexture() {
	/*ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		StageTextureCode,
		UTextureRenderTarget2D*, pSrcTex, InTex,
		FTexture2DRHIRef, pDstTex, Staging,
		{
			FTextureRHIParamRef texture = pSrcTex->GetRenderTargetResource()->GetRenderTargetTexture();

			RHICopySubTextureRegion_RenderThread();
			D3DRHI->GetDeviceContext()->CopySubresourceRegion(StagingTexture, 0, 0, 0, 0, GetResource(), Subresource, NULL);
			int32 w = 0;
			int32 h = 0;
			RHICmdList.MapStagingSurface(texture, mappedData, w, h);
			FMemory::Memcpy(dstMat.data, (uchar*)mappedData, w * h);
			barrier->set_value();
			RHICmdList.UnmapStagingSurface(texture);
		});*/
}

void OpenCV::Workload::ReadPixels(UTextureRenderTarget2D* src, cv::Mat dst, std::promise<void>* promise) {
	ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(
		ReadTextureCode,
		UTextureRenderTarget2D*, pRTtex, src,
		cv::Mat, dstMat, dst,
		std::promise<void>*, barrier, promise,
		{
			FTexture2DRHIRef texture = ((FTextureRenderTarget2DResource*)pRTtex->GetRenderTargetResource())->GetTextureRHI();
			uint32 Stride = 0;
			uchar* TexData = (uchar*)RHILockTexture2D(texture, 0, RLM_ReadOnly, Stride, false, false);
			if(Stride != dstMat.rows) {
				for(int i = 0; i < dstMat.cols; ++i) {
					memcpy(dstMat.data + i * dstMat.rows, TexData + i * Stride, dstMat.rows);
				}
			} else {
				memcpy(dstMat.data, TexData, Stride * dstMat.cols);
			}
			RHIUnlockTexture2D(texture, 0, false, false);
			barrier->set_value();
		});
	/*ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(
		ReadTextureCode,
		UTextureRenderTarget2D*, pRTtex, src,
		cv::Mat, dstMat, dst,
		std::promise<void>*, barrier, promise,
		{
			FTextureRHIParamRef texture = pRTtex->GetRenderTargetResource()->GetRenderTargetTexture();
			void* mappedData;
			int32 w = 0;
			int32 h = 0;
			RHICmdList.MapStagingSurface(texture, mappedData, w, h);
			FMemory::Memcpy(dstMat.data, (uchar*)mappedData, w * h);
			barrier->set_value();
			RHICmdList.UnmapStagingSurface(texture);
		});*/
}
