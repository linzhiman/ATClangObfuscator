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
    
    void setSelectorPrefix(const std::string& prefix, std::set<std::string> *oriPrefixSet);
    void setSourceManager(SourceManager *sm);
    void setReplacementsMap(std::map<std::string, Replacements> *rpsMap);
    void setCache(CSCache *cache);

    std::string getFilename(ObjCMethodDecl *decl);
    std::string getFilename(ObjCContainerDecl *decl);
    std::string getFilename(ObjCMessageExpr *expr);

    bool isNeedObfuscate(ObjCMethodDecl *decl);
    void obfuscate(ObjCMethodDecl *decl);
    
    bool isNeedObfuscate(ObjCMessageExpr *expr);
    void obfuscate(ObjCMessageExpr *expr, ASTContext *context);
    
    void addIgnoreProtocolSelector(ObjCMethodDecl *decl);
    
    void addClassProperty(ObjCPropertyDecl *decl);
    
private:
    std::string mGetterPrefix;
    std::string mSetterPrefix;
    std::set<std::string> *mOriPrefixSet;
    
    CSCache *mCache;
    SourceManager *mSourceManager;
    std::map<std::string, Replacements> *mRpsMap;
    
    std::string newSelectorName(const std::string& selector);
    
    SourceLocation getLoc(DeclContext *declContext);
    SourceLocation getLoc(ObjCMessageExpr *expr);
    
    std::string getClassName(ObjCMethodDecl *decl);
    
    bool isGetterOrSetter(ObjCMethodDecl *decl, ObjCInterfaceDecl *containerDecl);
    bool isPropertyAccessor(ObjCMethodDecl *decl);
    bool isMacroExpansion(ObjCMethodDecl *decl, bool *isWrittenInScratchSpace);
    std::vector<ObjCProtocolDecl *> getAllProtocols(ObjCProtocolDecl *protocol);
    std::vector<ObjCProtocolDecl *> getDefineProtocols(ObjCMethodDecl *decl);
    std::vector<ObjCMethodDecl *> getDefineMethods(ObjCMethodDecl *decl);
    bool isNeedObfuscate(ObjCMethodDecl *decl, bool isMessage);
    
    bool addIgnoreProtocolSelector(ObjCMethodDecl *decl, ObjCProtocolDecl *protocol);
    
    void addReplacement(const std::string &filePath, const Replacement &replace);
    void addReplacement(Selector sel, bool isImplicitProperty, SourceLocation loc);
    void addReplacement(ObjCMethodDecl *decl);
    void addReplacement(ObjCMessageExpr *expr, ASTContext *context);
};

#endif /* CSHelper_hpp */
