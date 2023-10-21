#include <Ice/Utils/FileUtil.h>
#include <fstream>
#include <iostream>
#include <filesystem>



// Initialize static members here.
std::string FileUtil::ProjectRoot = "";
std::string FileUtil::AssetDir = "";

void FileUtil::InitializeStaticMembers() {
    // Get the project root
    std::string projectRoot = std::filesystem::current_path().string() + "/";
    // Change the "\\" to "/"
    for (std::string::iterator it = projectRoot.begin(); it != projectRoot.end(); ++it) {
        if (*it == '\\') {
            *it = '/';
        }
    }

    ProjectRoot = projectRoot;
    AssetDir = ProjectRoot + "Assets/";
}



std::string FileUtil::GetProjectRoot()
{
    // Get the project root
    std::string projectRoot = std::filesystem::current_path().string() + "/";
    // Change the "\\" to "/"
    for (std::string::iterator it = projectRoot.begin(); it != projectRoot.end(); ++it) {
        if (*it == '\\') {
            *it = '/';
        }
    }
    return projectRoot;
}

std::string FileUtil::GetAssetDir()
{
	return GetProjectRoot() + "Assets/";
}




std::string FileUtil::ReadFile(const std::string& filename) 
{

	std::string updatedFilename = SubstituteVariables(filename);
	
    std::ifstream file(updatedFilename);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << updatedFilename << std::endl;
        return ""; // Return an empty string to indicate failure
    }

    // Read the file contents into a string
    std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Close the file
    file.close();

	fileContents = SubstituteVariables(fileContents);

    return fileContents;
}

std::string FileUtil::SubstituteVariables(const std::string& str)
{
	if (ProjectRoot == "")
	{
		InitializeStaticMembers();
	}

	std::string updatedStr = str;

	// PROJECT_ROOT
	size_t found = updatedStr.find("{PROJECT_ROOT}");
	while (found != std::string::npos)
	{
		updatedStr.replace(found, 14, ProjectRoot);
		found = updatedStr.find("{PROJECT_ROOT}");
	}

	// ASSET_DIR
	found = updatedStr.find("{ASSET_DIR}");
	while (found != std::string::npos)
	{
		updatedStr.replace(found, 11, AssetDir);
		found = updatedStr.find("{ASSET_DIR}");
	}

	return updatedStr;
}