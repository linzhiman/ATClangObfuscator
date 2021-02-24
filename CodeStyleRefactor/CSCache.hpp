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
    std::set<std::string> &getOriSelectorPrefix();
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
    
    void loadCache(const std::string &filePath);
    void saveCache(const std::string &filePath);
    
    bool isUserSourceCode(const std::string filename, bool checkWeakIgnoreFolder);
    std::vector<std::string> filterNotUserSourceCode(const std::vector<std::string> &allFiles);
    
private:
    //类名:方法列表
    std::map<std::string, std::map<std::string, std::string>> mReplaceClsMethodMap;
    
    //协议名:方法列表
    std::map<std::string, std::set<std::string>> mIgnoreProtocolSelectorMap;
    
    //类名:方法列表
    std::map<std::string, std::set<std::string>> mClsGetterSetterMap;
    
    //selector列表
    std::set<std::string> mIgnoreSelectorSet;
    
    //白名单
    std::set<std::string> mWhiteListSet;

    //黑名单
    std::set<std::string> mBlackListSet;
    
    std::vector<std::string> mStrongIgnoreFolderVector;
    
    std::vector<std::string> mWeakIgnoreFolderVector;
    
    std::string mSelectorPrefix;
    std::set<std::string> mOriSelectorPrefixSet;
};

#endif /* CSCache_hpp */
