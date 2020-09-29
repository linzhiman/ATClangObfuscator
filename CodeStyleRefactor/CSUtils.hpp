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
    
    static std::vector<std::string> filterNotUserSourceCode(const std::vector<string> &allFiles);
};

#endif /* CSUtils_hpp */
