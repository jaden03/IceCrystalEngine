#pragma once
#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <filesystem>

class FileUtil {
public:
    static std::string ProjectRoot;
    static std::string AssetDir;

    static void InitializeStaticMembers();

    static std::string ReadFile(const std::string& filename);
	
	static std::string SubstituteVariables(const std::string& str);
};


#endif