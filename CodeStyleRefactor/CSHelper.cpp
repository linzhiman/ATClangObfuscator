//
//  CSUtils.cpp
//  CodeStyleRefactor
//
//  Created by likaihao on 2019/12/5.
//

#include "CSHelper.hpp"
#include "CSUtils.hpp"

#include "clang/AST/ParentMapContext.h"

std::string CSHelper::classCategoryName(const std::string &className, const std::string &categoryName)
{
    return className + "(" + categoryName + ")";
}

void CSHelper::setSelectorPrefix(const std::string& prefix)
{
    mGetterPrefix = prefix;
    
    std::string tmp = "set" + prefix;
    tmp[3] = toUpper(tmp[3]);
    mSetterPrefix = tmp;
}

void CSHelper::setSourceManager(SourceManager *sm)
{
    mSourceManager = sm;
}

void CSHelper::setReplacementsMap(std::map<std::string, Replacements> *rpsMap)
{
    mRpsMap = rpsMap;
}

void CSHelper::setCache(CSCache *cache)
{
    mCache = cache;
}

std::string CSHelper::newSelectorName(const std::string& selector)
{
    if (selector.find("set") == 0) {
        std::string tmp = selector.substr(3);
        tmp[0] = tolower(tmp[0]);
        return mSetterPrefix + tmp;
    }
    
    return mGetterPrefix + selector;
}

SourceLocation CSHelper::getLoc(DeclContext *declContext)
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
    if (mSourceManager->isMacroBodyExpansion(loc) || mSourceManager->isMacroArgExpansion(loc)) {
        loc = mSourceManager->getSpellingLoc(loc);
    }
    return loc;
}

StringRef CSHelper::getFilename(ObjCMethodDecl *decl)
{
    return mSourceManager->getFilename(getLoc(decl));
}

StringRef CSHelper::getFilename(ObjCImplementationDecl *decl)
{
    return mSourceManager->getFilename(getLoc(decl));
}

StringRef CSHelper::getFilename(ObjCProtocolDecl *decl)
{
    return mSourceManager->getFilename(getLoc(decl));
}

bool CSHelper::isNeedObfuscate(ObjCMethodDecl *decl, bool checkIgnoreFolder, bool checkList)
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
                    StringRef filePath = getFilename(protocol);
                    if (CSUtils::isUserSourceCode(filePath.str(), checkIgnoreFolder)) {
                        return checkList ? mCache->checkWhiteBlackList(protocol->getNameAsString()) : true;
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
            return checkList ? mCache->checkWhiteBlackList(interfaceDecl->getNameAsString()) : true;
        }
    }
    else if (parentKind == Decl::ObjCCategory || parentKind == Decl::ObjCCategoryImpl) {
        ObjCCategoryDecl *category = parentKind == Decl::ObjCCategory ? cast<ObjCCategoryDecl>(parent) : cast<ObjCCategoryImplDecl>(parent)->getCategoryDecl();
        if (decl->isOverriding()) {
            const ObjCProtocolList &protocolList = category->getReferencedProtocols();
            for (ObjCProtocolDecl *protocol : protocolList) {
                if (protocol->lookupMethod(decl->getSelector(), decl->isInstanceMethod())) {
                    StringRef filePath = getFilename(protocol);
                    if (CSUtils::isUserSourceCode(filePath.str(), checkIgnoreFolder)) {
                        return checkList ? mCache->checkWhiteBlackList(protocol->getNameAsString()) : true;
                    }
                    break;
                }
            }
        }
        else {
            ObjCInterfaceDecl *interfaceDecl =  decl->getClassInterface();
            std::string clsName = interfaceDecl->getNameAsString();
            std::string categoryName = category->getNameAsString();
            std::string realName = CSHelper::classCategoryName(clsName, categoryName);
            return checkList ? mCache->checkWhiteBlackList(realName) : true;
        }
    }
    else if (parentKind == Decl::ObjCProtocol) {
        ObjCProtocolDecl *protocolDecl = cast<ObjCProtocolDecl>(parent);
        return checkList ? mCache->checkWhiteBlackList(protocolDecl->getNameAsString()) : true;
    }
    else {
        llvm::outs() << "isNeedObfuscate" << "\t" << selector.getAsString() << "\t" << parentKind << "\n";
    }
    
    return false;
}

void CSHelper::addReplacement(const std::string &filePath, const Replacement &replace)
{
    llvm::outs() << "addReplacement" << "\t" << replace.toString() << "\n";
    
    if (mRpsMap->find(filePath) != mRpsMap->end()) {
        llvm::Error err = (*mRpsMap)[filePath].add(replace);
    }
    else {
        mRpsMap->insert(make_pair(filePath, Replacements(replace)));
    }
}

void CSHelper::addReplacement(Selector sel, bool isImplicitProperty, SourceLocation loc)
{
    std::string selName = sel.getAsString();
    
    if (selName.find(mGetterPrefix) == 0 || selName.find(mSetterPrefix) == 0) {
        return;
    }
    
    if (mSourceManager->isWrittenInScratchSpace(loc)) {
        llvm::outs() << "AddReplacement Ignore ScratchSpace" << "\t" << selName << "\n";
        return;
    }
    
    if (mCache->ignoreSelector(selName)) {
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
    
    StringRef filePath = mSourceManager->getFilename(loc);
    unsigned begin = mSourceManager->getFileOffset(loc);
    unsigned length = selector.length();
    std::string newName = newSelectorName(selector);
    
    Replacement rep(filePath, begin, length, newName);
    addReplacement(filePath.str(), rep);
}

void CSHelper::addReplacement(ObjCMethodDecl *decl)
{
    Selector sel = decl->getSelector();
    SourceLocation loc = getLoc(decl);
    addReplacement(sel, false, loc);
}

void CSHelper::addReplacement(ObjCMessageExpr *expr, ASTContext &context)
{
    Selector sel = expr->getSelector();
    
    bool isImplicitProperty = false;
    DynTypedNodeList nodeList = context.getParents(*expr);
    for(auto it = nodeList.begin(); it != nodeList.end(); ++it) {
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
    if (mSourceManager->isMacroBodyExpansion(loc) || mSourceManager->isMacroArgExpansion(loc)) {
        loc = mSourceManager->getSpellingLoc(loc);
    }
    addReplacement(sel, isImplicitProperty, loc);
}
