#pragma once
#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <filesystem>

class FileUtil {
	
public:
    static std::string ProjectRoot;
	static std::string AssetDir;
	static std::string EngineAssetDir;

    static void InitializeStaticMembers();

    static std::string ReadFile(const std::string& filename);
	static bool FileExists(const std::string& filename);
	
	static std::string SubstituteVariables(const std::string& str);

	static std::string GetProjectRoot();
	static std::string GetExecutableDir();
	static std::string GetAssetDir();
	static std::string GetEngineAssetDir();
};


#endif