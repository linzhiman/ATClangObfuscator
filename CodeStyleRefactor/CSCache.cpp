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
        if (tmp == "prefix:") {
            prefix = true;
            oriPrefix = false;
            whiteList = false;
            blackList = false;
            strongIgnoreFolder = false;
            weakIgnoreFolder = false;
        }
        else if (tmp == "oriPrefix:") {
            prefix = false;
            oriPrefix = true;
            whiteList = false;
            blackList = false;
            strongIgnoreFolder = false;
            weakIgnoreFolder = false;
        }
        else if (tmp == "whiteList:") {
            prefix = false;
            oriPrefix = false;
            whiteList = true;
            blackList = false;
            strongIgnoreFolder = false;
            weakIgnoreFolder = false;
        }
        else if (tmp == "blackList:") {
            prefix = false;
            oriPrefix = false;
            whiteList = false;
            blackList = true;
            strongIgnoreFolder = false;
            weakIgnoreFolder = false;
        }
        else if (tmp == "strongIgnoreFolder:") {
            prefix = false;
            oriPrefix = false;
            whiteList = false;
            blackList = false;
            strongIgnoreFolder = true;
            weakIgnoreFolder = false;
        }
        else if (tmp == "weakIgnoreFolder:") {
            prefix = false;
            oriPrefix = false;
            whiteList = false;
            blackList = false;
            strongIgnoreFolder = false;
            weakIgnoreFolder = true;
        }
        else {
            if (prefix) {
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
    return mReplaceClsMethodMap.find(clsName) != mReplaceClsMethodMap.end();
}

void CSCache::addClsName(const std::string& clsName)
{
    mReplaceClsMethodMap[clsName] = std::map<string, string>();
}

bool CSCache::addClsNameSelector(const std::string& clsName, const std::string& selector, const std::string& newSelector)
{
    auto clsIt = mReplaceClsMethodMap.find(clsName);
    if (clsIt != mReplaceClsMethodMap.end()) {
        clsIt->second[selector] = newSelector;
        return true;
    }
    return false;
}

void CSCache::clearClsName()
{
    mReplaceClsMethodMap.clear();
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

void CSCache::addIgnoreProtocolSelector(const std::string& protocol, const std::string& selector)
{
    llvm::outs() << "addIgnoreProtocolSelector\t" << protocol << "\t" << selector << "\n";
    
    auto it = mIgnoreProtocolSelectorMap.find(protocol);
    if (it != mIgnoreProtocolSelectorMap.end()) {
        it->second.insert(selector);
    }
    else {
        mIgnoreProtocolSelectorMap[protocol] = std::set<std::string>();
        mIgnoreProtocolSelectorMap[protocol].insert(selector);
    }
}

bool CSCache::isClsGetterOrSetter(const std::string& clsName, const std::string& selector) const
{
    auto it = mClsGetterSetterMap.find(clsName);
    if (it != mClsGetterSetterMap.end()) {
        return it->second.find(selector) != it->second.end();
    }
    return false;
}

void CSCache::addClsGetterOrSetter(const std::string& clsName, const std::string& selector)
{
    llvm::outs() << "addClsGetterOrSetter\t" << clsName << "\t" << selector << "\n";
    
    auto it = mClsGetterSetterMap.find(clsName);
    if (it != mClsGetterSetterMap.end()) {
        it->second.insert(selector);
    }
    else {
        mClsGetterSetterMap[clsName] = std::set<std::string>();
        mClsGetterSetterMap[clsName].insert(selector);
    }
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
            mClsGetterSetterMap[keyTemp] = std::set<std::string>();
            ifs >> ignore >> count2;
            while (count2 > 0) {
                ifs >> valueTemp;
                mClsGetterSetterMap[keyTemp].insert(valueTemp);
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
    
    ofs <<"ignoreProtocolSelectorProtocolCount:" << "\t" << mIgnoreProtocolSelectorMap.size() << "\n";
    
    for (auto it = mIgnoreProtocolSelectorMap.begin(); it != mIgnoreProtocolSelectorMap.end(); ++it) {
        ofs << it->first << "\n";
        ofs <<"ignoreProtocolSelectorSelectorCount:" << "\t" << it->second.size() << "\n";
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            ofs << *it2 << "\n";
        }
    }
    
    ofs << "\n";
    
    ofs <<"clsGetterSetterMapClsCount:" << "\t" << mClsGetterSetterMap.size() << "\n";
    
    for (auto it = mClsGetterSetterMap.begin(); it != mClsGetterSetterMap.end(); ++it) {
        ofs << it->first << "\n";
        ofs <<"clsGetterSetterMapGetterSetterCount:" << "\t" << it->second.size() << "\n";
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
