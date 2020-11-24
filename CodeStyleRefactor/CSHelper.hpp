//
//  CSUtils.hpp
//  CodeStyleRefactor
//
//  Created by likaihao on 2019/12/5.
//

#ifndef CSHelper_hpp
#define CSHelper_hpp

#include <string>

#include "clang/AST/AST.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Tooling/Refactoring.h"

#include "CSCache.hpp"

using namespace std;
using namespace llvm;
using namespace clang;
using namespace clang::tooling;

class CSHelper
{
public:
    static std::string classCategoryName(const std::string &className, const std::string &categoryName);
    static std::string classCategoryName(ObjCCategoryDecl *decl);
    static std::string classCategoryName(ObjCCategoryImplDecl *decl);
    
    void setSelectorPrefix(const std::string& prefix);
    void setSourceManager(SourceManager *sm);
    void setReplacementsMap(std::map<std::string, Replacements> *rpsMap);
    void setCache(CSCache *cache);
    
    std::string newSelectorName(const std::string& selector);

    SourceLocation getLoc(DeclContext *declContext);
    std::string getFilename(ObjCMethodDecl *decl);
    std::string getFilename(ObjCContainerDecl *decl);
    std::string getFilename(ObjCMessageExpr *expr);

    bool isNeedObfuscate(ObjCMethodDecl *decl);
    void obfuscate(ObjCMethodDecl *decl);
    
    bool isNeedObfuscate(ObjCMessageExpr *expr);
    void obfuscate(ObjCMessageExpr *expr, ASTContext *context);
    
    void addIgnoreProtocolSelector(ObjCMethodDecl *decl);

    void addReplacement(ObjCMethodDecl *decl);
    void addReplacement(ObjCMessageExpr *expr, ASTContext *context);
    
private:
    std::string mGetterPrefix;
    std::string mSetterPrefix;
    
    CSCache *mCache;
    SourceManager *mSourceManager;
    std::map<std::string, Replacements> *mRpsMap;
    
    bool isNeedObfuscate(ObjCMethodDecl *decl, bool checkIgnoreFolder);
    
    void addReplacement(const std::string &filePath, const Replacement &replace);
    void addReplacement(Selector sel, bool isImplicitProperty, SourceLocation loc);
};

#endif /* CSHelper_hpp */
