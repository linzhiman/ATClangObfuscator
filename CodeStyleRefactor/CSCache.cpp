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
    if (selectorPrefix.length() > 0) {
        return selectorPrefix;
    }
    return "at_";
}

void CSCache::loadConfig(const std::string &filePath)
{
    std::ifstream ifs = std::ifstream(filePath, std::ofstream::in);
    
    std::string tmp;
    bool prefix = false;
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
            whiteList = false;
            blackList = false;
            strongIgnoreFolder = false;
            weakIgnoreFolder = false;
        }
        else if (tmp == "whiteList:") {
            prefix = false;
            whiteList = true;
            blackList = false;
            strongIgnoreFolder = false;
            weakIgnoreFolder = false;
        }
        else if (tmp == "blackList:") {
            prefix = false;
            whiteList = false;
            blackList = true;
            strongIgnoreFolder = false;
            weakIgnoreFolder = false;
        }
        else if (tmp == "strongIgnoreFolder:") {
            prefix = false;
            whiteList = false;
            blackList = false;
            strongIgnoreFolder = true;
            weakIgnoreFolder = false;
        }
        else if (tmp == "weakIgnoreFolder:") {
            prefix = false;
            whiteList = false;
            blackList = false;
            strongIgnoreFolder = false;
            weakIgnoreFolder = true;
        }
        else {
            if (prefix) {
                selectorPrefix = tmp;
            }
            else if (whiteList) {
                whiteListSet.insert(tmp);
            }
            else if (blackList) {
                blackListSet.insert(tmp);
            }
            else if (strongIgnoreFolder) {
                strongIgnoreFolderVector.push_back(tmp);
            }
            else if (weakIgnoreFolder) {
                weakIgnoreFolderVector.push_back(tmp);
            }
        }
    }
}

bool CSCache::containClsName(const std::string& clsName)
{
    return clsMethodMap.find(clsName) != clsMethodMap.end();
}

void CSCache::addClsName(const std::string& clsName)
{
    clsMethodMap[clsName] = std::map<string, string>();
}

bool CSCache::addClsNameSelector(const std::string& clsName, const std::string& selector, const std::string& newSelector)
{
    auto clsIt = clsMethodMap.find(clsName);
    if (clsIt != clsMethodMap.end()) {
        clsIt->second[selector] = newSelector;
        return true;
    }
    return false;
}

void CSCache::clearClsName()
{
    clsMethodMap.clear();
}

void CSCache::addClsNameIntoWhiteList(const std::string& clsName)
{
    whiteListSet.insert(clsName);
}

void CSCache::addClsNameIntoBlackList(const std::string& clsName)
{
    blackListSet.insert(clsName);
}

bool CSCache::isInWhiteList(const std::string &clsName) const
{
    std::string clsNameEx = clsName;
    int nPos = clsNameEx.find("(");
    if (nPos != -1) {
        clsNameEx = clsNameEx.substr(0, nPos);
    }
    if (whiteListSet.size() > 0) {
        return whiteListSet.find(clsNameEx) != whiteListSet.end();
    }
    return true;
}

bool CSCache::isInBlackList(const std::string &clsName) const
{
    std::string clsNameEx = clsName;
    int nPos = clsNameEx.find("(");
    if (nPos != -1) {
        clsNameEx = clsNameEx.substr(0, nPos);
    }
    return blackListSet.find(clsNameEx) != blackListSet.end();
}

bool CSCache::ignoreProtocolSelector(const std::string& protocol, const std::string& selector) const
{
    auto it = protocolSelectorMap.find(protocol);
    if (it != protocolSelectorMap.end()) {
        return it->second.find(selector) != it->second.end();
    }
    return false;
}

void CSCache::addIgnoreProtocolSelector(const std::string& protocol, const std::string& selector)
{
    llvm::outs() << "addIgnoreProtocolSelector\t" << protocol << "\t" << selector << "\n";
    
    auto it = protocolSelectorMap.find(protocol);
    if (it != protocolSelectorMap.end()) {
        it->second.insert(selector);
    }
    else {
        protocolSelectorMap[protocol] = std::set<std::string>();
        protocolSelectorMap[protocol].insert(selector);
    }
}

bool CSCache::ignoreSelector(const std::string& selector) const
{
    return selectorSet.find(selector) != selectorSet.end();
}

void CSCache::addIgnoreSelector(const std::string& selector)
{
    pair<std::set<std::string>::iterator, bool> retpair = selectorSet.insert(selector);
    if (retpair.second) {
        llvm::outs() << "addIgnoreSelector\t" << selector << "\n";
    }
}

void CSCache::loadIgnoreSelectors(const std::string &filePath)
{
    std::ifstream ifs = std::ifstream(filePath, std::ofstream::in);
    
    std::string ignore;
    int count;
    int count2;
    std::string protocolTemp;
    std::string selectorTemp;
    
    while (!ifs.eof()) {
        ifs >> ignore >> count;
        while (count > 0) {
            ifs >> selectorTemp;
            selectorSet.insert(selectorTemp);
            count--;
        }
        ifs >> ignore >> count;
        while (count > 0) {
            ifs >> protocolTemp;
            protocolSelectorMap[protocolTemp] = std::set<std::string>();
            ifs >> ignore >> count2;
            while (count2 > 0) {
                ifs >> selectorTemp;
                protocolSelectorMap[protocolTemp].insert(selectorTemp);
                count2--;
            }
            count--;
        }
    }
}

void CSCache::saveIgnoreSelectors(const std::string &filePath)
{
    std::ofstream ofs = std::ofstream(filePath, std::ofstream::out);
    
    ofs <<"ignoreSelectorCcount:" << "\t" << selectorSet.size() << "\n";
    
    for (auto it = selectorSet.begin(); it != selectorSet.end(); ++it) {
        ofs << *it << "\n";
    }
    
    ofs << "\n";
    
    ofs <<"ignoreProtocolSelectorProtocolCount:" << "\t" << protocolSelectorMap.size() << "\n";
    
    for (auto it = protocolSelectorMap.begin(); it != protocolSelectorMap.end(); ++it) {
        ofs << it->first << "\n";
        ofs <<"ignoreProtocolSelectorSelectorCount:" << "\t" << it->second.size() << "\n";
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
    
    for (auto it = strongIgnoreFolderVector.begin(); it != strongIgnoreFolderVector.end(); ++it)
    {
        if (filename.find(*it) != std::string::npos) {
            return false;
        }
    }
    
    if (!checkWeakIgnoreFolder) {
        return true;
    }
    
    for (auto it = weakIgnoreFolderVector.begin(); it != weakIgnoreFolderVector.end(); ++it)
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
