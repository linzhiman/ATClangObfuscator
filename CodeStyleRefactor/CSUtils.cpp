//
//  CSUtils.cpp
//  CodeStyleRefactor
//
//  Created by likaihao on 2019/12/5.
//

#include "CSUtils.hpp"

#include <fstream>
#include <vector>
#include <chrono>

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

long long CSUtils::getCurrentTimestamp()
{
    std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch()
    );
    
    return ms.count();
}

bool CSUtils::isUserSourceCode(const std::string filename, bool checkIgnoreFolder)
{
    if (filename.empty()) {
        return false;
    }
    
    if (filename.find("/Applications/Xcode.app/") == 0) {
        return false;
    }
    
    if (filename.find(".build/DerivedSources/") != filename.npos) {
        return false;
    }
    
    static std::vector<std::string> ignoreFolder;
    if (ignoreFolder.size() == 0) {
        ignoreFolder.push_back("Pods");
        ignoreFolder.push_back("protos");
    }
    
    for (auto it = ignoreFolder.begin(); it != ignoreFolder.end(); ++it )
    {
        if (filename.find( *it ) != std::string::npos) {
            return false;
        }
    }
    
    if (!checkIgnoreFolder) {
        return true;
    }
    
    static std::vector<std::string> ignoreFolder2;
    if (ignoreFolder2.size() == 0) {
        ignoreFolder2.push_back("ThirdParty");
    }
    
    for (auto it = ignoreFolder2.begin(); it != ignoreFolder2.end(); ++it )
    {
        if (filename.find( *it ) != std::string::npos) {
            return false;
        }
    }
    
    return true;
}

std::vector<std::string> CSUtils::GetProjectFiles(const std::vector<string> &allFiles)
{
    std::vector<std::string> result;
    result.reserve( allFiles.size() );
    
    for (auto it = allFiles.begin();  it != allFiles.end(); ++it) {
        if (isUserSourceCode( *it, false)) {
            result.push_back(it->c_str());
        }
    }
    
    return result;
}

std::vector<std::string> CSUtils::GetFilesWithFolder(const std::string &folderName, const std::vector<string> &allFiles )
{
    std::vector<std::string> result;
    result.reserve( allFiles.size() );
    
    for ( auto it = allFiles.begin(); it != allFiles.end(); ++it ) {
        if (!isUserSourceCode(*it, false)) {
            continue;
        }
        if ( it->find(folderName) != std::string::npos) {
            result.push_back( it->c_str() );
        }
    }
    
    return result;
}

void CSUtils::WriteFile(std::string &Filename, const std::string &Contents)
{
  if (!llvm::sys::path::is_absolute(Filename)) {
    llvm::SmallString<256> Path("~/clangOut");
    llvm::sys::path::append(Path, Filename);
    Filename = std::string(Path.str());
//    Files.insert(Filename);
  }
    
  llvm::sys::fs::create_directories(llvm::sys::path::parent_path(Filename));
  std::ofstream OS(Filename);
  OS << Contents;
  assert(OS.good());
}
