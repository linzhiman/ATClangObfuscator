//
//  CSUtils.cpp
//  CodeStyleRefactor
//
//  Created by likaihao on 2019/12/5.
//

#include "CSCache.hpp"

#include <fstream>

#include "clang/AST/ParentMapContext.h"

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
    
    while (!ifs.eof()) {
        ifs >> tmp;
        if (tmp == "prefix:") {
            prefix = true;
            whiteList = false;
            blackList = false;
        }
        else if (tmp == "whiteList:") {
            prefix = false;
            whiteList = true;
            blackList = false;
        }
        else if (tmp == "blackList:") {
            prefix = false;
            whiteList = false;
            blackList = true;
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

bool CSCache::checkWhiteBlackList(const std::string &clsName) const
{
    if (blackListSet.find(clsName) != blackListSet.end()) {
        return false;
    }
    if (whiteListSet.size() > 0) {
        return whiteListSet.find(clsName) != whiteListSet.end();
    }
    return true;
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
