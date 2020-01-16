//FTexture2DRHIRef Staging;

ElectricLinesWorkload(FIntPoint outSize, UTextureRenderTarget2D* input, UTexture2D*& output) {
	/*ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		CreateStagingTexture,
		FTexture2DRHIRef&, stagedTex, Staging,
		FIntPoint, size, outSize,
		{
			FRHIResourceCreateInfo CreateInfo;
			stagedTex = RHICreateTexture2D(size.X, size.Y, PF_G8, 1, 1, TexCreate_CPUReadback | TexCreate_Dynamic | TexCreate_SRGB, CreateInfo);
		}
	);*/
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
		}
	);*/
}

void OpenCV::Workload::ReadPixels(UTextureRenderTarget2D* src, cv::Mat dst, std::promise<void>* promise) {
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
		}
	);*/
}