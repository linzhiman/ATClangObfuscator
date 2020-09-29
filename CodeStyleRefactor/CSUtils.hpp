//
//  CSUtils.hpp
//  CodeStyleRefactor
//
//  Created by likaihao on 2019/12/5.
//

#ifndef CSUtils_hpp
#define CSUtils_hpp

#include <string>

using namespace std;

class CSUtils
{
public:
    static long long getCurrentTimestamp();
    static bool isUserSourceCode(const std::string filename, bool checkIgnoreFolder);
    static void WriteFile(std::string &Filename, const std::string &Contents);
    static std::vector<std::string> GetProjectFiles(const std::vector<string> &allFiles);
    static std::vector<std::string> GetFilesWithFolder(const std::string &folderName, const std::vector<std::string> &allFiles );
};

#endif /* CSUtils_hpp */
