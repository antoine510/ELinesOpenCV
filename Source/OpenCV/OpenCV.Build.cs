using UnrealBuildTool;
using System.Collections.Generic;
using System.IO;

public class OpenCV : ModuleRules {

	private string opencvPath {
		get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/opencv/")); }
	}

	public OpenCV(ReadOnlyTargetRules Target) : base(Target) {
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "RHI", "RenderCore"});
		PrivatePCHHeaderFile = "Private/OpenCVPCH.h";

		if (Target.Platform != UnrealTargetPlatform.Win64) {
			throw new System.Exception("This plugin is only available for Win64 right now.");
		}

		PrivateIncludePaths.Add(Path.Combine(opencvPath, "include/"));
		
		PublicLibraryPaths.Add(Path.Combine(opencvPath, "lib/Win64/"));
		PublicAdditionalLibraries.Add("opencv_world401.lib");

		PublicDelayLoadDLLs.Add("opencv_world401.dll");

		addDependency("Win64", "opencv_world401.dll");
		addDependency("Win64", "opencv_ffmpeg401_64.dll");
		
		bEnableExceptions = true;
	}
	
	private void addDependency(string arch, string lib) {
		RuntimeDependencies.Add(Path.Combine(opencvPath, "lib/", arch, lib));
	}
}
