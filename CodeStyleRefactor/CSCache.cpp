//
//  CSUtils.cpp
//  CodeStyleRefactor
//
//  Created by likaihao on 2019/12/5.
//

#include "CSCache.hpp"

#include <fstream>

#include "clang/AST/ParentMapContext.h"

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
    
    std::string selectorCountTag;
    int selectorCount;
    
    std::string selector;
    
    while (!ifs.eof()) {
        ifs >> selectorCountTag >> selectorCount;
        while (selectorCount > 0) {
            ifs >> selector;
            selectorSet.insert(selector);
            selectorCount--;
        }
    }
}

void CSCache::saveIgnoreSelectors(const std::string &filePath)
{
    std::ofstream ofs = std::ofstream(filePath, std::ofstream::out);
    
    ofs <<"selectorCcount:" << "\t" << selectorSet.size() << "\n";
    
    for (auto it = selectorSet.begin(); it != selectorSet.end(); ++it) {
        ofs << *it <<"\n";
    }
}
