#include <string>
#include <fstream>
#include <queue>

#include "clang/AST/AST.h"
#include "clang/AST/ASTTypeTraits.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ParentMapContext.h"

#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/Tooling.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

#include "CSUtils.hpp"

using namespace llvm;
using namespace clang;
using namespace clang::tooling;
using namespace std;

const std::string kSelPrefix = "sealion_";
const std::string kSelSetterPrefix = "setSealion_";

bool runSelectorComsumer;

//类名:方法列表
std::map<std::string , /* class name */
         std::map<std::string, std::string> /*selector : new selecotr */ > clsMethodMap;

//selector列表
std::set<std::string> selectorSet;

//白名单
std::set<std::string> whiteListSet;

//黑名单
std::set<std::string> blackListSet;

void SerializaSelectorSet(const std::set<std::string> &selectorSet, const std::string &filePath)
{
    std::ofstream ofs = std::ofstream(filePath, std::ofstream::out);
    
    ofs <<"selectorCcount:" << "\t" << selectorSet.size() << "\n";
    
    for (auto it = selectorSet.begin(); it != selectorSet.end(); ++it) {
        ofs << *it <<"\n";
    }
}

void UnserializaSelectorSet(std::set<std::string> &selectorSet, const std::string &filePath)
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

std::string ReplaceSelectorName(const std::string& selectorName)
{
    if (selectorName.find("set") == 0) {
        std::string tmp = selectorName.substr(3);
        tmp[0] = tolower(tmp[0]);
        return kSelSetterPrefix + tmp;
    }
    
    return kSelPrefix + selectorName;
}

std::string ClassCategoryName(const std::string &className, const std::string &categoryName )
{
    return className + "(" + categoryName + ")";
}

SourceLocation GetLoc(DeclContext *declContext, SourceManager &sm)
{
    SourceLocation loc;
    if (declContext->getDeclKind() == Decl::ObjCMethod) {
        auto *tmp = cast<ObjCMethodDecl>(declContext);
        loc = tmp->getSelectorStartLoc();
    }
    else if (declContext->getDeclKind() == Decl::ObjCImplementation) {
        auto *tmp = cast<ObjCImplementationDecl>(declContext);
        loc = tmp->getSourceRange().getBegin();
    }
    else if (declContext->getDeclKind() == Decl::ObjCProtocol) {
        auto *tmp = cast<ObjCProtocolDecl>(declContext);
        loc = tmp->getSourceRange().getBegin();
    }
    else {
        ;;
    }
    if (sm.isMacroBodyExpansion(loc) || sm.isMacroArgExpansion(loc)) {
        loc = sm.getSpellingLoc(loc);
    }
    return loc;
}

StringRef GetFilename(ObjCMethodDecl *decl, SourceManager &sm)
{
    return sm.getFilename(GetLoc(decl, sm));
}

StringRef GetFilename(ObjCImplementationDecl *decl, SourceManager &sm)
{
    return sm.getFilename(GetLoc(decl, sm));
}

StringRef GetFilename(ObjCProtocolDecl *decl, SourceManager &sm)
{
    return sm.getFilename(GetLoc(decl, sm));
}

bool checkWhiteBlackList(const std::string &name)
{
    if (blackListSet.find(name) != blackListSet.end()) {
        return false;
    }
    if (whiteListSet.size() > 0) {
        return whiteListSet.find(name) != whiteListSet.end();
    }
    return true;
}

bool isNeedObfuscateClassName(ObjCMethodDecl *decl, SourceManager &sm, bool checkIgnoreFolder, bool checkList)
{
    ObjCMethodFamily family = decl->getMethodFamily();
    if (family != OMF_None && family != OMF_performSelector) {
        return false;
    }
    
    Selector selector = decl->getSelector();
    DeclContext *parent = decl->getParent();
    Decl::Kind parentKind = parent->getDeclKind();
    
    if (decl->getCanonicalDecl()->isPropertyAccessor()) {
        return false;
    }
    else {
        if (parentKind == Decl::ObjCCategory || parentKind == Decl::ObjCCategoryImpl) {
            ObjCCategoryDecl *category = parentKind == Decl::ObjCCategory ? cast<ObjCCategoryDecl>(parent) : cast<ObjCCategoryImplDecl>(parent)->getCategoryDecl();
            if (decl->isInstanceMethod()) {
                for (const auto *I : category->instance_properties()) {
                    if (selector == I->getGetterName() || selector == I->getSetterName()) {
                        return false;
                    }
                }
            }
            else {
                for (const auto *I : category->class_properties()) {
                    if (selector == I->getGetterName() || selector == I->getSetterName()) {
                        return false;
                    }
                }
            }
        }
    }
    
    if (parentKind == Decl::ObjCInterface || parentKind == Decl::ObjCImplementation) {
        ObjCInterfaceDecl *interfaceDecl =  decl->getClassInterface();
        if (decl->isOverriding()) {
            const ObjCProtocolList &protocolList = interfaceDecl->getReferencedProtocols();
            for (ObjCProtocolDecl *protocol : protocolList) {
                if (protocol->lookupMethod(decl->getSelector(), decl->isInstanceMethod())) {
                    StringRef filePath = GetFilename(protocol, sm);
                    if (CSUtils::isUserSourceCode(filePath.str(), checkIgnoreFolder)) {
                        return checkList ? checkWhiteBlackList(protocol->getNameAsString()) : true;
                    }
                    break;
                }
            }
        }
        else {
            if (decl->isInstanceMethod()) {
                for (const auto *I : interfaceDecl->instance_properties()) {
                    if (selector == I->getGetterName() || selector == I->getSetterName()) {
                        return false;
                    }
                }
            }
            else {
                for (const auto *I : interfaceDecl->class_properties()) {
                    if (selector == I->getGetterName() || selector == I->getSetterName()) {
                        return false;
                    }
                }
            }
            return checkList ? checkWhiteBlackList(interfaceDecl->getNameAsString()) : true;
        }
    }
    else if (parentKind == Decl::ObjCCategory || parentKind == Decl::ObjCCategoryImpl) {
        ObjCCategoryDecl *category = parentKind == Decl::ObjCCategory ? cast<ObjCCategoryDecl>(parent) : cast<ObjCCategoryImplDecl>(parent)->getCategoryDecl();
        if (decl->isOverriding()) {
            const ObjCProtocolList &protocolList = category->getReferencedProtocols();
            for (ObjCProtocolDecl *protocol : protocolList) {
                if (protocol->lookupMethod(decl->getSelector(), decl->isInstanceMethod())) {
                    StringRef filePath = GetFilename(protocol, sm);
                    if (CSUtils::isUserSourceCode(filePath.str(), checkIgnoreFolder)) {
                        return checkList ? checkWhiteBlackList(protocol->getNameAsString()) : true;
                    }
                    break;
                }
            }
        }
        else {
            ObjCInterfaceDecl *interfaceDecl =  decl->getClassInterface();
            std::string clsName = interfaceDecl->getNameAsString();
            std::string categoryName = category->getNameAsString();
            std::string realName = ClassCategoryName(clsName, categoryName);
            return checkList ? checkWhiteBlackList(realName) : true;
        }
    }
    else if (parentKind == Decl::ObjCProtocol) {
        ObjCProtocolDecl *protocolDecl = cast<ObjCProtocolDecl>(parent);
        return checkList ? checkWhiteBlackList(protocolDecl->getNameAsString()) : true;
    }
    else {
        llvm::outs() << "isNeedObfuscateClassName" << "\t" << selector.getAsString() << "\t" << parentKind << "\n";
    }
    
    return false;
}

void AddReplacement(const std::string &filePath, const Replacement &replace, std::map<std::string, Replacements> &rpsMap)
{
    llvm::outs() << "AddReplacement" << "\t" << replace.toString() << "\n";
    
    if( rpsMap.find(filePath) != rpsMap.end() ) {
        llvm::Error err = rpsMap[filePath].add(replace);
    } else {
        rpsMap.insert(make_pair(filePath, Replacements(replace) ));
    }
}

void AddReplacement(Selector sel, SourceLocation loc, SourceManager &sm, std::map<std::string, Replacements> &rpsMap, bool isImplicitProperty)
{
    std::string selName = sel.getAsString();
    
    if (selName.find(kSelPrefix) == 0 || selName.find(kSelSetterPrefix) == 0) {
        return;
    }
    
    if (sm.isWrittenInScratchSpace(loc)) {
        llvm::outs() << "AddReplacement Ignore ScratchSpace" << "\t" << selName << "\n";
        return;
    }
    
    if (selectorSet.find(selName) != selectorSet.end()) {
        llvm::outs() << "AddReplacement Ignore" << "\t" << selName << "\n";
        return;
    }
    
    // - methodName:(type0)arg0 slot1:(type1)arg1
    //目前只处理 methodName
    std::string selector = sel.getNameForSlot(0).str();
    
    if (isImplicitProperty && selector.find("set") == 0) {
        selector = selector.substr(3);
        selector[0] = toLower(selector[0]);
    }
    
    StringRef filePath = sm.getFilename(loc);
    unsigned begin = sm.getFileOffset(loc);
    unsigned length = selector.length();
    std::string newName = ReplaceSelectorName(selector);
    
    Replacement rep(filePath, begin, length, newName );
    AddReplacement(filePath.str(), rep, rpsMap);
}

void AddReplacement(ObjCMethodDecl *decl, SourceManager &sm, std::map<std::string, Replacements> &rpsMap)
{
    Selector sel = decl->getSelector();
    SourceLocation loc = GetLoc(decl, sm);
    AddReplacement(sel, loc, sm, rpsMap, false);
}

void AddReplacement(ObjCMessageExpr *expr, SourceManager &sm, std::map<std::string, Replacements> &rpsMap, ASTContext &context)
{
    Selector sel = expr->getSelector();
    
    bool isImplicitProperty = false;
    DynTypedNodeList nodeList = context.getParents( *expr );
    for( auto it = nodeList.begin(); it != nodeList.end(); ++it) {
        if (it->getNodeKind().isSame(ASTNodeKind::getFromNodeKind<PseudoObjectExpr>())) {
            const PseudoObjectExpr *pseudo = it->get<PseudoObjectExpr>();
            if (const BinaryOperator *binaryOperator = dyn_cast_or_null<BinaryOperator>(pseudo->getSyntacticForm())) {
                if (ObjCPropertyRefExpr *propertyRefExpr = dyn_cast_or_null<ObjCPropertyRefExpr>(binaryOperator->getLHS())) {
                    if (propertyRefExpr->isImplicitProperty()) {
                        isImplicitProperty = true;
                    }
                }
            }
            break;
        }
    }
    
    SourceLocation loc = expr->getSelectorStartLoc();
    if (sm.isMacroBodyExpansion(loc) || sm.isMacroArgExpansion(loc)) {
        loc = sm.getSpellingLoc(loc);
    }
    AddReplacement(sel, loc, sm, rpsMap, isImplicitProperty);
}

class ObjCMethodDeclVisitor : public RecursiveASTVisitor<ObjCMethodDeclVisitor>
{
    SourceManager &_sm;
    
    std::map<std::string, Replacements> &_replacementMap;
    std::queue<Decl::Kind> _visitQueueKind;
    bool _isTraverseImp;
    
public:
    ObjCMethodDeclVisitor(SourceManager &sm, std::map<std::string, Replacements> &rpsMap)
    : _sm(sm)
    , _replacementMap(rpsMap)
    {
        
    }
    
    bool TraverseDeclContextHelper(DeclContext *DC)
    {
        if (!DC) {
          return true;
        }
        
        if ( DC->isTranslationUnit() || DC->isObjCContainer() || DC->isFunctionOrMethod()) {
            for (auto *Child : DC->decls()) {
                if (!canIgnoreChildDeclWhileTraversingDeclContext(Child)) {
                    TraverseDecl(Child);
                }
            }
        }
        return true;
    }

    bool TraverseImportDecl(ImportDecl * decl) {
        llvm::outs() << "TraverseImportDecl " << decl->getImportedModule()->getFullModuleName() << "\n";
        return true;
    }
    
    bool TraverseObjCProtocolDecl(ObjCProtocolDecl * decl)
    {
        StringRef filePath = _sm.getFilename(decl->getSourceRange().getBegin());
        if (!CSUtils::isUserSourceCode(filePath.str(), true)) {
            return true;
        }
        
        if (clsMethodMap.find(decl->getNameAsString()) != clsMethodMap.end() ) {
            return true;
        }
        
        llvm::outs() << "TraverseObjCProtocolDecl " << decl->getNameAsString() << "\n";
        
        return RecursiveASTVisitor<ObjCMethodDeclVisitor>::TraverseObjCProtocolDecl(decl);
    }
    
    bool TraverseObjCInterfaceDecl(ObjCInterfaceDecl * decl)
    {
        StringRef filePath = _sm.getFilename(decl->getSourceRange().getBegin());
        if (!CSUtils::isUserSourceCode(filePath.str(), true)) {
            return true;
        }
        
        if (clsMethodMap.find(decl->getNameAsString()) != clsMethodMap.end() ) {
            return true;
        }
        
        llvm::outs() << "TraverseObjCInterfaceDecl " << decl->getNameAsString() << "\n";
        
        return  RecursiveASTVisitor<ObjCMethodDeclVisitor>::TraverseObjCInterfaceDecl(decl);
    }
    
    bool TraverseObjCCategoryDecl(ObjCCategoryDecl * decl)
    {
        StringRef filePath = _sm.getFilename(decl->getSourceRange().getBegin());
        if (!CSUtils::isUserSourceCode(filePath.str(), true)) {
            return true;
        }
        
        std::string categoryName = decl->getNameAsString();
        std::string className = decl->getClassInterface()->getNameAsString();
        std::string realName = ClassCategoryName(className, categoryName);
        
        if (clsMethodMap.find(realName) != clsMethodMap.end()) {
            return true;
        }

        llvm::outs() << "TraverseObjCCategoryDecl " << realName << "\n";
        
        return RecursiveASTVisitor<ObjCMethodDeclVisitor>::TraverseObjCCategoryDecl(decl);
    }
    
    bool TraverseObjCCategoryImplDecl(ObjCCategoryImplDecl *decl)
    {
        std::string categoryName = decl->getNameAsString();
        std::string className = decl->getClassInterface()->getNameAsString();
        std::string realName = ClassCategoryName(className, categoryName);
        
        llvm::outs() << "TraverseObjCCategoryImplDecl " << realName << "\n";
        
        _isTraverseImp = true;
        
        bool ret =   RecursiveASTVisitor<ObjCMethodDeclVisitor>::TraverseObjCCategoryImplDecl(decl);
        
        _isTraverseImp = false;
        
        return ret;
    }
    
    bool TraverseObjCImplementationDecl(ObjCImplementationDecl *decl)
    {
        llvm::outs() << "TraverseObjCImplementationDecl " << decl->getNameAsString() << "\n";
        
        _isTraverseImp = true;
        
        bool ret =  RecursiveASTVisitor<ObjCMethodDeclVisitor>::TraverseObjCImplementationDecl(decl);
        
        _isTraverseImp = false;
        
        return ret;
    }
    
    bool TraverseObjCMethodDecl(ObjCMethodDecl *decl)
    {
        if (!isNeedObfuscateClassName(decl, _sm, true, true)) {
            return true;
        }
        
        std::string selector = decl->getSelector().getAsString();
        
        DeclContext *parent = decl->getParent();
        Decl::Kind parentKind = parent->getDeclKind();
        if (parentKind == Decl::ObjCInterface) {
            ObjCInterfaceDecl *interfaceDecl =  decl->getClassInterface();
            if (interfaceDecl) {
                auto clsIt = clsMethodMap.find(interfaceDecl->getNameAsString());
                if (clsIt != clsMethodMap.end()) {
                    clsIt->second[selector] = ReplaceSelectorName(selector);
                    AddReplacement(decl, _sm, _replacementMap);
                }
            }
        } else if (parentKind == Decl::ObjCImplementation) {
            ObjCInterfaceDecl *interfaceDecl =  decl->getClassInterface();
            if (interfaceDecl) {
                auto clsIt = clsMethodMap.find(interfaceDecl->getNameAsString());
                if (clsIt != clsMethodMap.end()) {
                    clsIt->second[selector] = ReplaceSelectorName(selector);
                    AddReplacement(decl, _sm, _replacementMap);
                }
            }
            
            return RecursiveASTVisitor<ObjCMethodDeclVisitor>::TraverseObjCMethodDecl(decl);
            
        } else if (parentKind == Decl::ObjCCategory || parentKind == Decl::ObjCCategoryImpl) {
            ObjCInterfaceDecl *interfaceDecl =  decl->getClassInterface();
            ObjCCategoryDecl  *category = parentKind == Decl::ObjCCategory ? cast<ObjCCategoryDecl>(parent)
                                                                           : cast<ObjCCategoryImplDecl>(parent)->getCategoryDecl();
            
            std::string catName = category->getNameAsString();
            std::string clsName = interfaceDecl->getNameAsString();
            std::string realName = ClassCategoryName(clsName, catName);
            
            auto clsIt = clsMethodMap.find(realName);
            if (clsIt != clsMethodMap.end()) {
                clsIt->second[selector] = ReplaceSelectorName(selector);
                AddReplacement(decl, _sm, _replacementMap);
            }
            
        } else if (parentKind == Decl::ObjCProtocol) {
            ObjCProtocolDecl *protocolDecl = cast<ObjCProtocolDecl>(parent);
            if (protocolDecl) {
                auto clsIt = clsMethodMap.find(protocolDecl->getNameAsString());
                if (clsIt != clsMethodMap.end()) {
                    clsIt->second[selector] = ReplaceSelectorName(selector);
                    AddReplacement(decl, _sm, _replacementMap);
                }
            }
        }
        return true;
    }
    
    bool VisitObjCInterfaceDecl(ObjCInterfaceDecl *decl) {
        if (!decl->hasDefinition() || decl->getDefinition() != decl) {
            return true;
        }
        clsMethodMap[decl->getNameAsString()] = std::map<string, string>();
        
        return true;
    }
    
    bool VisitObjCCategoryDecl(ObjCCategoryDecl * decl) {
        
        std::string categoryName = decl->getNameAsString();
        std::string className = decl->getClassInterface()->getNameAsString();
        
        std::string name = ClassCategoryName(className, categoryName);
        clsMethodMap[name] = std::map<string, string>();
        
        return true;
    }
    
    bool VisitObjCImplementationDecl(ObjCImplementationDecl *decl) {
        
        return true;
    }
    
    bool VisitObjCProtocolDecl(const ObjCProtocolDecl *decl) {
        if (!decl->hasDefinition() || decl->getDefinition() != decl) {
            return true;
        }
        clsMethodMap[decl->getNameAsString()] = std::map<string, string>();
        
        return true;
    }
};

class ObjCMessageExprVisitor : public RecursiveASTVisitor<ObjCMessageExprVisitor>
{
private:
    SourceManager &_sm;
    ASTContext *_astContext;
    
    std::map<std::string, Replacements> &_replacementMap;
    bool _isTraverseImp;
    
public:
    ObjCMessageExprVisitor(SourceManager &sm, std::map<std::string, Replacements> &rpsMap )
    : _sm(sm)
    , _replacementMap(rpsMap)
    , _isTraverseImp(false)
    {
        
    }
    
    void setContext(ASTContext *context) {
        _astContext = context;
    }
    
    bool TraverseDeclContextHelper(DeclContext *DC)
    {
        if (!DC) {
          return true;
        }
        
        if ( _isTraverseImp || DC->isTranslationUnit() || DC->getDeclKind() == Decl::ObjCImplementation
            || DC->getDeclKind() == Decl::ObjCCategoryImpl) {
            for (auto *Child : DC->decls()) {
              if (!canIgnoreChildDeclWhileTraversingDeclContext(Child)) {
                  TraverseDecl(Child);
              }
            }
        }
        
        return true;
    }
    
    bool TraverseObjCImplementationDecl(ObjCImplementationDecl *decl) {
        
        StringRef filePath = GetFilename(decl, _sm);
        if(!CSUtils::isUserSourceCode(filePath.str(), false)) {
            return true;
        }
        
        llvm::outs() << "TraverseObjCImplementationDecl " << decl->getNameAsString() << "\n";
        
        _isTraverseImp = true;
        
        bool ret =  RecursiveASTVisitor<ObjCMessageExprVisitor>::TraverseObjCImplementationDecl(decl);
        
        _isTraverseImp = false;
        
        return ret;
    }
    
    bool TraverseObjCCategoryImplDecl(ObjCCategoryImplDecl *decl) {
        
        _isTraverseImp = true;
        
        bool ret =  RecursiveASTVisitor<ObjCMessageExprVisitor>::TraverseObjCCategoryImplDecl(decl);
        
        _isTraverseImp = false;
        
        return ret;
    }
    
    bool VisitObjCMessageExpr(ObjCMessageExpr *expr) {
     
        ObjCMethodDecl *mDecl = expr->getMethodDecl();
        if (!mDecl) {
            return true;
        }
        
        SourceLocation loc = expr->getSelectorStartLoc();
        if (_sm.isMacroBodyExpansion(loc) || _sm.isMacroArgExpansion(loc)) {
            loc = _sm.getSpellingLoc(loc);
        }

        StringRef filePath = _sm.getFilename(loc);
        if (!CSUtils::isUserSourceCode(filePath.str(), false)) {
           return true;
        }
        
        bool isDefineInMacro = false;
        loc = mDecl->getSelectorStartLoc();
        if (_sm.isMacroBodyExpansion(loc) || _sm.isMacroArgExpansion(loc)) {
            loc = _sm.getSpellingLoc(loc);
            isDefineInMacro = true;
        }
        
        StringRef definePath = _sm.getFilename(loc);
        if (!CSUtils::isUserSourceCode(definePath.str(), true)) {
            return true;
        }
        
        if (!isNeedObfuscateClassName(mDecl, _sm, false, !isDefineInMacro)) {
            return true;
        }
        
        llvm::outs() << "call message: " << mDecl->getNameAsString() << "\n";
        
        DeclContext *parent = mDecl->getParent();
        Decl::Kind parentKind = parent->getDeclKind();
        
        if (parentKind == Decl::ObjCCategory || parentKind == Decl::ObjCCategoryImpl) {
            llvm::outs() << "call ObjCCategory method: " << mDecl->getSelector().getAsString() << "\n";
        }
        
        std::string interfaceName;
        std::string fileName;
        
        if (parentKind == Decl::ObjCInterface || parentKind == Decl::ObjCImplementation) {
            ObjCInterfaceDecl *interface = mDecl->isClassMethod() ? mDecl->getClassInterface() : expr->getReceiverInterface();
            if (interface) {
                interfaceName = interface->getNameAsString();
                fileName = _sm.getFilename(interface->getSourceRange().getBegin()).str();
            }
        }
        else if (parentKind == Decl::ObjCCategory || parentKind == Decl::ObjCCategoryImpl) {
            if (parentKind == Decl::ObjCCategory) {
                auto *tmp = cast<ObjCCategoryDecl>(parent);
                if (tmp) {
                    interfaceName = ClassCategoryName(tmp->getClassInterface()->getNameAsString(), tmp->getNameAsString());
                    fileName = _sm.getFilename(tmp->getSourceRange().getBegin()).str();
                }
            }
            else if (parentKind == Decl::ObjCCategoryImpl) {
                auto *tmp = cast<ObjCCategoryImplDecl>(parent);
                if (tmp) {
                    interfaceName = ClassCategoryName(tmp->getClassInterface()->getNameAsString(), tmp->getNameAsString());
                    fileName = _sm.getFilename(tmp->getSourceRange().getBegin()).str();
                }
            }
        }
        else if (parentKind == Decl::ObjCProtocol) {
            ObjCProtocolDecl *protocolDecl = cast<ObjCProtocolDecl>(parent);
            if (protocolDecl) {
                interfaceName = protocolDecl->getNameAsString();
                fileName = _sm.getFilename(protocolDecl->getSourceRange().getBegin()).str();
            }
        }
        else if (expr->getReceiverType()->isObjCIdType()) {
            interfaceName = "_ObjCId_";
            fileName = _sm.getFilename(expr->getSelectorStartLoc()).str();
        }
        else {
            llvm::outs() <<"[!NotHandle] " << expr->getSelector().getAsString() << "\n";
        }
        
        if (interfaceName.length() > 0) {
            if (clsMethodMap.find(interfaceName) == clsMethodMap.end()) {
                return true;
            }
            if (CSUtils::isUserSourceCode(fileName, false)) {
                AddReplacement(expr, _sm, _replacementMap, *_astContext);
           }
        }
        
        return true;
            
    }
};

class CodeStyleASTConsumer: public ASTConsumer
{
private:
    ObjCMethodDeclVisitor   methodVisitor;
    ObjCMessageExprVisitor  messageExprVisitor;
    
public:
    CodeStyleASTConsumer( std::map<std::string, Replacements> & replacementsMap, CompilerInstance &CI, StringRef file)
    : methodVisitor(CI.getSourceManager(), replacementsMap)
    , messageExprVisitor(CI.getSourceManager(), replacementsMap)
    {
        
    }
    
    ~CodeStyleASTConsumer() {
        
    }
    
    void HandleTranslationUnit(ASTContext &context) {
        
        long long start =  CSUtils::getCurrentTimestamp();
        
        methodVisitor.TraverseTranslationUnitDecl(context.getTranslationUnitDecl());
        
        messageExprVisitor.setContext(&context);
        messageExprVisitor.TraverseTranslationUnitDecl(context.getTranslationUnitDecl());
        
        llvm::outs() << "TranslationUnit finish:" << CSUtils::getCurrentTimestamp() - start << "\n";
    }
};

class CodeStyleSelectorVisitor : public RecursiveASTVisitor<CodeStyleSelectorVisitor>
{
private:
    SourceManager &_sm;
public:
    explicit CodeStyleSelectorVisitor(SourceManager &sm) : _sm(sm)
    {}
    
    bool shouldVisitImplicitCode() const { return true; }
    
    bool VisitObjCSelectorExpr(ObjCSelectorExpr *expr) {
        Selector sel = expr->getSelector();
        
        llvm::outs() << "SelectorExpr" << "\t" << sel.getAsString() << "\n";
        
        selectorSet.insert(sel.getAsString());
        
        return true;
    }
};

class CodeStyleSelectorConsumer: public ASTConsumer
{
private:
    CodeStyleSelectorVisitor visitor;
public:
    explicit CodeStyleSelectorConsumer(CompilerInstance &CI)
    : visitor(CI.getSourceManager()) {}
    
    virtual void HandleTranslationUnit(ASTContext &Context) {
        visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }
};

class CodeStyleASTAction: public ASTFrontendAction
{
private:
    RefactoringTool & _refactoringTool;
    
public:
    CodeStyleASTAction (RefactoringTool &refactoingTool)
    : ASTFrontendAction()
    , _refactoringTool(refactoingTool)
    {
        
    }
    unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef iFile) override
    {
        if (!CSUtils::isUserSourceCode(iFile.str(), false)) {
            llvm::outs() << "[Ignore] file:" << iFile.str() << "\n";
            return unique_ptr<ASTConsumer>();
        }
        
        llvm::outs() << " [Analize] file:" << iFile.str() << "\n";
        
        if (runSelectorComsumer) {
            return unique_ptr<CodeStyleSelectorConsumer> (new CodeStyleSelectorConsumer(CI));
        }
        return unique_ptr<CodeStyleASTConsumer> (new CodeStyleASTConsumer( _refactoringTool.getReplacements(), CI, iFile));
    }
    
    bool ParseArgs(const CompilerInstance &ci, const std::vector<std::string> &args)
    {
        return true;
    }
};

class CodeStyleActionFactory : public FrontendActionFactory
{
private:
    RefactoringTool &_refactoringTool;
    
public:
    CodeStyleActionFactory(RefactoringTool &tool)
    : FrontendActionFactory()
    , _refactoringTool(tool)
    {
        
    }
    
    std::unique_ptr<FrontendAction> create() override
    {
        return unique_ptr<FrontendAction>( new CodeStyleASTAction( _refactoringTool ) );
    }
};

void applyChangeToFiles(RefactoringTool &Tool)
{
    IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
    DiagnosticsEngine Diagnostics(IntrusiveRefCntPtr<DiagnosticIDs>(new DiagnosticIDs()),
                                  &*DiagOpts,
                                  new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts),
                                  true);

    SourceManager Sources(Diagnostics, Tool.getFiles());
    
    Rewriter Rewrite(Sources, LangOptions());
    Tool.applyAllReplacements(Rewrite);
    
    for (Rewriter::buffer_iterator I = Rewrite.buffer_begin(), E = Rewrite.buffer_end(); I != E; ++I) {
        I->second.write( llvm::outs() );
    }
    
    Rewrite.overwriteChangedFiles();
}

int main(int argc, const char **argv)
{
    static llvm::cl::OptionCategory CodeStyleRefactorCategory("CodeStyleRefactor");
    
    llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);

    CommonOptionsParser op(argc, argv, CodeStyleRefactorCategory );
    
    std::vector<std::string> analizeFiles = op.getSourcePathList();
    std::string sourceDir;
    
    if (analizeFiles.size() == 1) {
        std::string path = analizeFiles.front();
        if (sys::fs::is_regular_file(path)) {
            sourceDir = path.substr(0, path.rfind('\\'));
        }
        else {
            sourceDir = path;
            CompilationDatabase &compilation = op.getCompilations();
            analizeFiles = CSUtils::GetProjectFiles(compilation.getAllFiles());
        }
    }
    else {
        llvm::outs() << "[Error] only support one source path";
        return 0;
    }
    
    long long start =  CSUtils::getCurrentTimestamp();
    
    RefactoringTool Tool(op.getCompilations(), analizeFiles );
    
    std::unique_ptr<FrontendActionFactory> factory ( new CodeStyleActionFactory(Tool) );
    
    llvm::SmallString<256> Path(sourceDir);
    llvm::sys::path::append(Path, "selectors.txt");
    std::string selectorFilePath = std::string(Path.str());
    if (sys::fs::exists(selectorFilePath)) {
        UnserializaSelectorSet(selectorSet, selectorFilePath);
    }
    else {
        runSelectorComsumer = true;
        
        if (int Result = Tool.run( factory.get() ) )
        {
            return Result;
        }
        
        SerializaSelectorSet(selectorSet, selectorFilePath);
    }
    
    runSelectorComsumer = false;
    
    clsMethodMap["_ObjCId_"] = std::map<string, string>();
    
    if (int Result = Tool.run( factory.get() ) )
    {
        return Result;
    }
    
    applyChangeToFiles(Tool);
    
    llvm::outs() << "[Finish] total files:" << analizeFiles.size() <<" cost time:" << CSUtils::getCurrentTimestamp() - start << "(ms)\n";
    
    return 0;
}
