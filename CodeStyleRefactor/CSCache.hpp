//
//  CSUtils.hpp
//  CodeStyleRefactor
//
//  Created by likaihao on 2019/12/5.
//

#ifndef CSCache_hpp
#define CSCache_hpp

#include <string>
#include <map>
#include <set>
#include <vector>

using namespace std;

class CSCache
{
public:
    std::string getSelectorPrefix();
    void loadConfig(const std::string &filePath);
    
    bool containClsName(const std::string& clsName);
    void addClsName(const std::string& clsName);
    bool addClsNameSelector(const std::string& clsName, const std::string& selector, const std::string& newSelector);
    void clearClsName();
    
    void addClsNameIntoWhiteList(const std::string& clsName);
    void addClsNameIntoBlackList(const std::string& clsName);
    bool isInWhiteList(const std::string &clsName) const;
    bool isInBlackList(const std::string &clsName) const;
    
    bool ignoreProtocolSelector(const std::string& protocol, const std::string& selector) const;
    void addIgnoreProtocolSelector(const std::string& protocol, const std::string& selector);
    
    bool isClsGetterOrSetter(const std::string& clsName, const std::string& selector) const;
    void addClsGetterOrSetter(const std::string& clsName, const std::string& selector);
    
    bool ignoreSelector(const std::string& selector) const;
    void addIgnoreSelector(const std::string& selector);
    void loadIgnoreSelectors(const std::string &filePath);
    void saveIgnoreSelectors(const std::string &filePath);
    
    bool isUserSourceCode(const std::string filename, bool checkWeakIgnoreFolder);
    std::vector<std::string> filterNotUserSourceCode(const std::vector<std::string> &allFiles);
    
private:
    //类名:方法列表
    std::map<std::string, std::map<std::string, std::string>> clsMethodMap;
    
    //协议名:方法列表
    std::map<std::string, std::set<std::string>> protocolSelectorMap;
    
    //类名:方法列表
    std::map<std::string, std::set<std::string>> clsGetterSetterMap;
    
    //selector列表
    std::set<std::string> selectorSet;
    
    //白名单
    std::set<std::string> whiteListSet;

    //黑名单
    std::set<std::string> blackListSet;
    
    std::vector<std::string> strongIgnoreFolderVector;
    
    std::vector<std::string> weakIgnoreFolderVector;
    
    std::string selectorPrefix;
};

#endif /* CSCache_hpp */
