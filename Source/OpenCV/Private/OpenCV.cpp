#include "OpenCVPCH.h"
#include "OpenCV.h"
#include "Core.h"
#include "ModuleManager.h"

bool OpenCVModule::initialized = false;
void* OpenCVModule::dllHandle = nullptr;

namespace OpenCV {
	extern void LogMessage(const char* msg);
}

void OpenCVModule::StartupModule() {
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Add on the relative location of the third party dll and load it
	FString libraryPath = FPaths::Combine(FPaths::ProjectPluginsDir(), L"OpenCV/ThirdParty/opencv/lib/");
	FString libraryName;

#if PLATFORM_WINDOWS
#if PLATFORM_64BITS
	libraryPath = FPaths::Combine(libraryPath, L"Win64");
#elif PLATFORM_32BITS
	libraryPath = FPaths::Combine(libraryPath, L"Win32");
#endif
	libraryName = L"opencv_world401.dll";
#elif PLATFORM_LINUX
	libraryPath = FPaths::Combine(libraryPath, L"Linux");
	libraryName = L"libopencv_world401.so";
#elif PLATFORM_MAC
	libraryPath = FPaths::Combine(libraryPath, L"MacOS");
	libraryName = L"libopencv_world401.dylib";
#endif
	if(libraryName.Len() == 0) {
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(L"Unsuported platform!"));
		return;
	}

	if(!FPaths::FileExists(FPaths::Combine(libraryPath, libraryName))) {
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(L"Couldn't find OpenCV at: " + libraryPath));
		return;
	}

	FPlatformProcess::PushDllDirectory(*libraryPath);

	dllHandle = FPlatformProcess::GetDllHandle(*libraryName);

	FPlatformProcess::PopDllDirectory(*libraryPath);

	if(dllHandle == nullptr) {
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(L"Couldn't load OpenCV library"));
		ShutdownModule();
		return;
	}

	initialized = true;
}

void OpenCVModule::ShutdownModule() {
	initialized = false;

	FPlatformProcess::FreeDllHandle(dllHandle);
}

IMPLEMENT_MODULE(OpenCVModule, OpenCV);
