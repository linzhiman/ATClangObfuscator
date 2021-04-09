//
//  CSUtils.cpp
//  CodeStyleRefactor
//
//  Created by likaihao on 2019/12/5.
//

#include "CSCache.hpp"

#include <fstream>

#include "clang/AST/ParentMapContext.h"

void trim(string &s)
{
    if (!s.empty()) {
        s.erase(0, s.find_first_not_of(" "));
        s.erase(s.find_last_not_of(" ") + 1);
    }
}

bool CSCache::usingCache()
{
    return mConfig.find("cache") != mConfig.end();
}

std::string CSCache::getSelectorPrefix()
{
    if (mSelectorPrefix.length() > 0) {
        return mSelectorPrefix;
    }
    return "at_";
}

std::set<std::string> &CSCache::getOriSelectorPrefix()
{
    return mOriSelectorPrefixSet;
}

void CSCache::loadConfig(const std::string &filePath)
{
    std::ifstream ifs = std::ifstream(filePath, std::ofstream::in);
    
    std::string tmp;
    bool config = false;
    bool prefix = false;
    bool oriPrefix = false;
    bool whiteList = false;
    bool blackList = false;
    bool strongIgnoreFolder = false;
    bool weakIgnoreFolder = false;
    
    while (std::getline(ifs, tmp)) {
        if (tmp.find("#") == 0) {
            continue;
        }
        trim(tmp);
        if (tmp.empty()) {
            continue;
        }
        if (tmp == "config:") {
            config = true;
            prefix = false;
            oriPrefix = false;
            whiteList = false;
            blackList = false;
            strongIgnoreFolder = false;
            weakIgnoreFolder = false;
        }
        else if (tmp == "prefix:") {
            config = false;
            prefix = true;
            oriPrefix = false;
            whiteList = false;
            blackList = false;
            strongIgnoreFolder = false;
            weakIgnoreFolder = false;
        }
        else if (tmp == "oriPrefix:") {
            config = false;
            prefix = false;
            oriPrefix = true;
            whiteList = false;
            blackList = false;
            strongIgnoreFolder = false;
            weakIgnoreFolder = false;
        }
        else if (tmp == "whiteList:") {
            config = false;
            prefix = false;
            oriPrefix = false;
            whiteList = true;
            blackList = false;
            strongIgnoreFolder = false;
            weakIgnoreFolder = false;
        }
        else if (tmp == "blackList:") {
            config = false;
            prefix = false;
            oriPrefix = false;
            whiteList = false;
            blackList = true;
            strongIgnoreFolder = false;
            weakIgnoreFolder = false;
        }
        else if (tmp == "strongIgnoreFolder:") {
            config = false;
            prefix = false;
            oriPrefix = false;
            whiteList = false;
            blackList = false;
            strongIgnoreFolder = true;
            weakIgnoreFolder = false;
        }
        else if (tmp == "weakIgnoreFolder:") {
            config = false;
            prefix = false;
            oriPrefix = false;
            whiteList = false;
            blackList = false;
            strongIgnoreFolder = false;
            weakIgnoreFolder = true;
        }
        else {
            if (config) {
                mConfig.insert(tmp);
            }
            else if (prefix) {
                mSelectorPrefix = tmp;
            }
            else if (oriPrefix) {
                mOriSelectorPrefixSet.insert(tmp);
            }
            else if (whiteList) {
                mWhiteListSet.insert(tmp);
            }
            else if (blackList) {
                mBlackListSet.insert(tmp);
            }
            else if (strongIgnoreFolder) {
                mStrongIgnoreFolderVector.push_back(tmp);
            }
            else if (weakIgnoreFolder) {
                mWeakIgnoreFolderVector.push_back(tmp);
            }
        }
    }
}

bool CSCache::containClsName(const std::string& clsName)
{
    return mReplaceClsSelectorMap.find(clsName) != mReplaceClsSelectorMap.end();
}

void CSCache::addClsName(const std::string& clsName)
{
    mReplaceClsSelectorMap[clsName] = std::map<string, string>();
}

bool CSCache::addClsNameSelector(const std::string& clsName, const std::string& selector, const std::string& newSelector)
{
    auto clsIt = mReplaceClsSelectorMap.find(clsName);
    if (clsIt != mReplaceClsSelectorMap.end()) {
        clsIt->second[selector] = newSelector;
        return true;
    }
    return false;
}

void CSCache::clearClsName()
{
    mReplaceClsSelectorMap.clear();
}

void CSCache::addClsNameIntoWhiteList(const std::string& clsName)
{
    mWhiteListSet.insert(clsName);
}

void CSCache::addClsNameIntoBlackList(const std::string& clsName)
{
    mBlackListSet.insert(clsName);
}

bool CSCache::isInWhiteList(const std::string &clsName) const
{
    std::string clsNameEx = clsName;
    int nPos = clsNameEx.find("(");
    if (nPos != -1) {
        clsNameEx = clsNameEx.substr(0, nPos);
    }
    if (mWhiteListSet.size() > 0) {
        return mWhiteListSet.find(clsNameEx) != mWhiteListSet.end();
    }
    return true;
}

bool CSCache::isInBlackList(const std::string &clsName) const
{
    if (mBlackListSet.find(clsName) != mBlackListSet.end()) {
        return true;
    }
    
    std::string clsNameEx = clsName;
    int nPos = clsNameEx.find("(");
    if (nPos != -1) {
        clsNameEx = clsNameEx.substr(0, nPos);
        return mBlackListSet.find(clsNameEx) != mBlackListSet.end();
    }
    return false;
}

bool CSCache::ignoreProtocolSelector(const std::string& protocol, const std::string& selector) const
{
    auto it = mIgnoreProtocolSelectorMap.find(protocol);
    if (it != mIgnoreProtocolSelectorMap.end()) {
        return it->second.find(selector) != it->second.end();
    }
    return false;
}

void CSCache::genIgnoreProtocolSelector(void)
{
    for (auto it = mClsProtocolMap.begin(); it != mClsProtocolMap.end(); ++it) {
        std::set<std::string> protocols = it->second;
        for (std::string protocol : protocols) {
            auto itSelectors = mProtocolSelectorMap.find(protocol);
            if (itSelectors != mProtocolSelectorMap.end()) {
                std::set<std::string> protocolSelectors = itSelectors->second;
                auto itPropertys = mClsPropertySelectorMap.find(it->first);
                if (itPropertys != mClsPropertySelectorMap.end()) {
                    std::set<std::string> propertySelectors = itPropertys->second;
                    std::set<std::string> intersection;
                    std::set_intersection(protocolSelectors.begin(), protocolSelectors.end(), propertySelectors.begin(), propertySelectors.end(), std::inserter(intersection, intersection.begin()));
                    for (std::string selector : intersection) {
                        addIgnoreProtocolSelector(protocol, selector);
                    }
                }
            }
        }
    }
}

void CSCache::addIgnoreProtocolSelector(const std::string& protocol, const std::string& selector)
{
    auto it = mIgnoreProtocolSelectorMap.find(protocol);
    if (it != mIgnoreProtocolSelectorMap.end()) {
        auto retpair = it->second.insert(selector);
        if (retpair.second) {
            llvm::outs() << "addIgnoreProtocolSelector\t" << protocol << "\t" << selector << "\n";
        }
    }
    else {
        mIgnoreProtocolSelectorMap[protocol] = std::set<std::string>();
        auto retpair = mIgnoreProtocolSelectorMap[protocol].insert(selector);
        if (retpair.second) {
            llvm::outs() << "addIgnoreProtocolSelector\t" << protocol << "\t" << selector << "\n";
        }
    }
}

bool CSCache::isClsGetterOrSetter(const std::string& clsName, const std::string& selector) const
{
    auto it = mClsPropertySelectorMap.find(clsName);
    if (it != mClsPropertySelectorMap.end()) {
        return it->second.find(selector) != it->second.end();
    }
    return false;
}

void CSCache::addClsGetterOrSetter(const std::string& clsName, const std::string& selector)
{
    auto it = mClsPropertySelectorMap.find(clsName);
    if (it != mClsPropertySelectorMap.end()) {
        auto retpair = it->second.insert(selector);
        if (retpair.second) {
            llvm::outs() << "addClsGetterOrSetter\t" << clsName << "\t" << selector << "\n";
        }
    }
    else {
        mClsPropertySelectorMap[clsName] = std::set<std::string>();
        auto retpair = mClsPropertySelectorMap[clsName].insert(selector);
        if (retpair.second) {
            llvm::outs() << "addClsGetterOrSetter\t" << clsName << "\t" << selector << "\n";
        }
    }
}

void CSCache::addClsProtocol(const std::string& clsName, const std::string& protocol)
{
    auto it = mClsProtocolMap.find(clsName);
    if (it != mClsProtocolMap.end()) {
        auto retpair = it->second.insert(protocol);
        if (retpair.second) {
            llvm::outs() << "addClsProtocol\t" << clsName << "\t" << protocol << "\n";
        }
    }
    else {
        mClsProtocolMap[clsName] = std::set<std::string>();
        auto retpair = mClsProtocolMap[clsName].insert(protocol);
        if (retpair.second) {
            llvm::outs() << "addClsProtocol\t" << clsName << "\t" << protocol << "\n";
        }
    }
}

void CSCache::addProtocolSelector(const std::string& protocol, const std::string& selector)
{
    auto it = mProtocolSelectorMap.find(protocol);
    if (it != mProtocolSelectorMap.end()) {
        auto retpair = it->second.insert(selector);
        if (retpair.second) {
            llvm::outs() << "addProtocolSelector\t" << protocol << "\t" << selector << "\n";
        }
    }
    else {
        mProtocolSelectorMap[protocol] = std::set<std::string>();
        auto retpair = mProtocolSelectorMap[protocol].insert(selector);
        if (retpair.second) {
            llvm::outs() << "addProtocolSelector\t" << protocol << "\t" << selector << "\n";
        }
    }
}

bool CSCache::isIgnoreClsSelector(const std::string& clsName, const std::string& selector) const
{
    std::string clsNameEx = clsName;
    int nPos = clsNameEx.find("(");
    if (nPos != -1) {
        clsNameEx = clsNameEx.substr(0, nPos);
    }
    
    auto itProtocols = mClsProtocolMap.find(clsNameEx);
    if (itProtocols != mClsProtocolMap.end()) {
        std::set<std::string> protocols = itProtocols->second;
        for (std::string protocol : protocols) {
            auto itSelectors = mIgnoreProtocolSelectorMap.find(protocol);
            if (itSelectors != mIgnoreProtocolSelectorMap.end()) {
                std::set<std::string> ignoreProtocolSelectors = itSelectors->second;
                if (ignoreProtocolSelectors.find(selector) != ignoreProtocolSelectors.end()) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool CSCache::ignoreSelector(const std::string& selector) const
{
    return mIgnoreSelectorSet.find(selector) != mIgnoreSelectorSet.end();
}

void CSCache::addIgnoreSelector(const std::string& selector)
{
    pair<std::set<std::string>::iterator, bool> retpair = mIgnoreSelectorSet.insert(selector);
    if (retpair.second) {
        llvm::outs() << "addIgnoreSelector\t" << selector << "\n";
    }
}

void CSCache::loadCache(const std::string &filePath)
{
    std::ifstream ifs = std::ifstream(filePath, std::ofstream::in);
    
    std::string ignore;
    int count;
    int count2;
    std::string keyTemp;
    std::string valueTemp;
    
    while (!ifs.eof()) {
        ifs >> ignore >> count;
        while (count > 0) {
            ifs >> valueTemp;
            mIgnoreSelectorSet.insert(valueTemp);
            count--;
        }
        ifs >> ignore >> count;
        while (count > 0) {
            ifs >> keyTemp;
            mIgnoreProtocolSelectorMap[keyTemp] = std::set<std::string>();
            ifs >> ignore >> count2;
            while (count2 > 0) {
                ifs >> valueTemp;
                mIgnoreProtocolSelectorMap[keyTemp].insert(valueTemp);
                count2--;
            }
            count--;
        }
        ifs >> ignore >> count;
        while (count > 0) {
            ifs >> keyTemp;
            mClsPropertySelectorMap[keyTemp] = std::set<std::string>();
            ifs >> ignore >> count2;
            while (count2 > 0) {
                ifs >> valueTemp;
                mClsPropertySelectorMap[keyTemp].insert(valueTemp);
                count2--;
            }
            count--;
        }
        ifs >> ignore >> count;
        while (count > 0) {
            ifs >> keyTemp;
            mClsProtocolMap[keyTemp] = std::set<std::string>();
            ifs >> ignore >> count2;
            while (count2 > 0) {
                ifs >> valueTemp;
                mClsProtocolMap[keyTemp].insert(valueTemp);
                count2--;
            }
            count--;
        }
        ifs >> ignore >> count;
        while (count > 0) {
            ifs >> keyTemp;
            mProtocolSelectorMap[keyTemp] = std::set<std::string>();
            ifs >> ignore >> count2;
            while (count2 > 0) {
                ifs >> valueTemp;
                mProtocolSelectorMap[keyTemp].insert(valueTemp);
                count2--;
            }
            count--;
        }
    }
}

void CSCache::saveCache(const std::string &filePath)
{
    std::ofstream ofs = std::ofstream(filePath, std::ofstream::out);
    
    ofs <<"ignoreSelectorCcount:" << "\t" << mIgnoreSelectorSet.size() << "\n";
    
    for (auto it = mIgnoreSelectorSet.begin(); it != mIgnoreSelectorSet.end(); ++it) {
        ofs << *it << "\n";
    }
    
    ofs << "\n";
    
    ofs <<"ignoreProtocolSelectorMapProtocolCount:" << "\t" << mIgnoreProtocolSelectorMap.size() << "\n";
    
    for (auto it = mIgnoreProtocolSelectorMap.begin(); it != mIgnoreProtocolSelectorMap.end(); ++it) {
        ofs << it->first << "\n";
        ofs <<"ignoreProtocolSelectorMapSelectorCount:" << "\t" << it->second.size() << "\n";
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            ofs << *it2 << "\n";
        }
    }
    
    ofs << "\n";
    
    ofs <<"clsPropertySelectorMapClsCount:" << "\t" << mClsPropertySelectorMap.size() << "\n";
    
    for (auto it = mClsPropertySelectorMap.begin(); it != mClsPropertySelectorMap.end(); ++it) {
        ofs << it->first << "\n";
        ofs <<"clsPropertySelectorMapSelectorCount:" << "\t" << it->second.size() << "\n";
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            ofs << *it2 << "\n";
        }
    }
    
    ofs << "\n";
    
    ofs <<"clsProtocolMapClsCount:" << "\t" << mClsProtocolMap.size() << "\n";
    
    for (auto it = mClsProtocolMap.begin(); it != mClsProtocolMap.end(); ++it) {
        ofs << it->first << "\n";
        ofs <<"clsProtocolMapProtocolCount:" << "\t" << it->second.size() << "\n";
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            ofs << *it2 << "\n";
        }
    }
    
    ofs << "\n";
    
    ofs <<"protocolSelectorMapProtocolCount:" << "\t" << mProtocolSelectorMap.size() << "\n";
    
    for (auto it = mProtocolSelectorMap.begin(); it != mProtocolSelectorMap.end(); ++it) {
        ofs << it->first << "\n";
        ofs <<"protocolSelectorMapSelectorCount:" << "\t" << it->second.size() << "\n";
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            ofs << *it2 << "\n";
        }
    }
}

bool CSCache::isUserSourceCode(const std::string filename, bool checkWeakIgnoreFolder)
{
    if (filename.empty()) {
        return false;
    }
    
    if (filename.find("/Applications/Xcode.app/") == 0) {
        return false;
    }
    
    if (filename.find("/Xcode/DerivedData/") != filename.npos) {
        return false;
    }
    
    if (filename.find(".build/DerivedSources/") != filename.npos) {
        return false;
    }
    
    for (auto it = mStrongIgnoreFolderVector.begin(); it != mStrongIgnoreFolderVector.end(); ++it)
    {
        if (filename.find(*it) != std::string::npos) {
            return false;
        }
    }
    
    if (!checkWeakIgnoreFolder) {
        return true;
    }
    
    for (auto it = mWeakIgnoreFolderVector.begin(); it != mWeakIgnoreFolderVector.end(); ++it)
    {
        if (filename.find(*it) != std::string::npos) {
            return false;
        }
    }
    
    return true;
}

std::vector<std::string> CSCache::filterNotUserSourceCode(const std::vector<std::string> &allFiles)
{
    std::vector<std::string> result;
    result.reserve(allFiles.size());
    
    for (auto it = allFiles.begin();  it != allFiles.end(); ++it) {
        if (isUserSourceCode(*it, false)) {
            result.push_back(it->c_str());
        }
    }
    
    return result;
}
