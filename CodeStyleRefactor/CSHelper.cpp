//
//  CSUtils.cpp
//  CodeStyleRefactor
//
//  Created by likaihao on 2019/12/5.
//

#include "CSHelper.hpp"
#include "CSUtils.hpp"

#include "clang/AST/ParentMapContext.h"

bool CheckIsString(const char * p, const std::string &key)
{
    size_t i = 0, len = key.size();
    while ( *p != '\0' && i < len && (*p == key[i]) ) {
        p++;
        i++;
    }
    return i == len;
}

std::string CSHelper::classCategoryName(const std::string &className, const std::string &categoryName)
{
    return className + "(" + categoryName + ")";
}

std::string CSHelper::classCategoryName(ObjCCategoryDecl *decl)
{
    std::string categoryName = decl->getNameAsString();
    std::string className = decl->getClassInterface()->getNameAsString();
    return CSHelper::classCategoryName(className, categoryName);
}

std::string CSHelper::classCategoryName(ObjCCategoryImplDecl *decl)
{
    std::string categoryName = decl->getNameAsString();
    std::string className = decl->getClassInterface()->getNameAsString();
    return CSHelper::classCategoryName(className, categoryName);
}

void CSHelper::setSelectorPrefix(const std::string& prefix, std::set<std::string> *oriPrefixSet)
{
    mGetterPrefix = prefix;
    
    std::string tmp = "set" + prefix;
    tmp[3] = toUpper(tmp[3]);
    mSetterPrefix = tmp;
    
    mOriPrefixSet = oriPrefixSet;
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
    std::string oldName = selector;
    std::string newName = mGetterPrefix;
    if (selector.find("set") == 0) {
        std::string tmp = selector.substr(3);
        tmp[0] = tolower(tmp[0]);
        oldName = tmp;
        newName = mSetterPrefix;
    }
    
    newName += "_";
    
    for (std::set<std::string>::iterator i = mOriPrefixSet->begin(); i != mOriPrefixSet->end(); ++i) {
        std::string prefix = *i + "_";
        if (oldName.find(prefix) == 0) {
            oldName = oldName.substr(prefix.length());
            newName += "_";
            break;
        }
    }
    
    for (size_t i = 0;  i < oldName.size(); i++) {
        if (oldName[i] == ':') {
            newName += oldName.substr(i);
            break;
        }
        if (isUppercase(oldName[i])) {
            if (CheckIsString(&oldName[i], "URL")) {
                newName += "_url";
                i += 2;
                continue;
            } else if (CheckIsString(&oldName[i], "GET") ) {
                newName += "_get";
                i += 2 ;
                continue;
                
            } else if (CheckIsString(&oldName[i], "JSON") ) {
                newName += "_json";
                i += 3;
                continue;
            }
            newName += "_";
        }
        newName += tolower(oldName[i]);
    }
    return newName;
}

SourceLocation CSHelper::getLoc(DeclContext *declContext)
{
    SourceLocation loc;
    if (declContext->getDeclKind() == Decl::ObjCMethod) {
        auto *tmp = cast<ObjCMethodDecl>(declContext);
        loc = tmp->getSelectorStartLoc();
    }
    else if (auto *tmp = cast<ObjCContainerDecl>(declContext)) {
        loc = tmp->getSourceRange().getBegin();
    }
    else {
        llvm::outs() << "[!NotHandle]" << "\t" << declContext->getDeclKindName() << "\n";
    }
    if (mSourceManager->isMacroBodyExpansion(loc) || mSourceManager->isMacroArgExpansion(loc)) {
        loc = mSourceManager->getSpellingLoc(loc);
    }
    return loc;
}

SourceLocation CSHelper::getLoc(ObjCMessageExpr *expr)
{
    SourceLocation loc = expr->getSelectorStartLoc();
    if (mSourceManager->isMacroBodyExpansion(loc) || mSourceManager->isMacroArgExpansion(loc)) {
        loc = mSourceManager->getSpellingLoc(loc);
    }
    return loc;
}

std::string CSHelper::getFilename(ObjCMethodDecl *decl)
{
    return mSourceManager->getFilename(getLoc(decl)).str();
}

std::string CSHelper::getFilename(ObjCContainerDecl *decl)
{
    return mSourceManager->getFilename(getLoc(decl)).str();
}

std::string CSHelper::getFilename(ObjCMessageExpr *expr)
{
    return mSourceManager->getFilename(getLoc(expr)).str();
}

bool CSHelper::isGetterOrSetter(ObjCMethodDecl *decl, ObjCInterfaceDecl *interfaceDecl)
{
    Selector selector = decl->getSelector();
    
    if (decl->isInstanceMethod()) {
        for (const auto *I : interfaceDecl->instance_properties()) {
            if (selector == I->getGetterName() || selector == I->getSetterName()) {
                return true;
            }
        }
    }
    else {
        for (const auto *I : interfaceDecl->class_properties()) {
            if (selector == I->getGetterName() || selector == I->getSetterName()) {
                return true;
            }
        }
    }
    
    if (mCache->isClsGetterOrSetter(interfaceDecl->getNameAsString(), selector.getAsString())) {
        return true;
    }
    
    return false;
}

bool CSHelper::isPropertyAccessor(ObjCMethodDecl *decl)
{
    DeclContext *parent = decl->getParent();
    Decl::Kind parentKind = parent->getDeclKind();
    
    if (decl->getCanonicalDecl()->isPropertyAccessor()) {
        return true;
    }
    else if (parentKind == Decl::ObjCInterface || parentKind == Decl::ObjCImplementation) {
        ObjCInterfaceDecl *interfaceDecl =  decl->getClassInterface();
        return isGetterOrSetter(decl, interfaceDecl);
    }
    else if (parentKind == Decl::ObjCCategory || parentKind == Decl::ObjCCategoryImpl) {
        ObjCCategoryDecl *categoryDecl = parentKind == Decl::ObjCCategory ? cast<ObjCCategoryDecl>(parent) : cast<ObjCCategoryImplDecl>(parent)->getCategoryDecl();
        return isGetterOrSetter(decl, categoryDecl->getClassInterface());
    }
    
    return false;
}

bool CSHelper::isMacroExpansion(ObjCMethodDecl *decl, bool *isWrittenInScratchSpace)
{
    bool isMacro = false;
    bool isInScratchSpace = false;
    SourceLocation loc = decl->getSelectorStartLoc();
    if (mSourceManager->isMacroBodyExpansion(loc) || mSourceManager->isMacroArgExpansion(loc)) {
        isMacro = true;
        loc = mSourceManager->getSpellingLoc(loc);
        if (mSourceManager->isWrittenInScratchSpace(loc)) {
            isInScratchSpace = true;
        }
    }
    if (isWrittenInScratchSpace) {
        *isWrittenInScratchSpace = isInScratchSpace;
    }
    return isMacro;
}

std::vector<ObjCProtocolDecl *> CSHelper::getAllProtocols(ObjCProtocolDecl *protocol)
{
    std::vector<ObjCProtocolDecl *> all;
    all.push_back(protocol);
    for (ObjCProtocolDecl *protocol : protocol->getReferencedProtocols()) {
        std::vector<ObjCProtocolDecl *> subAll = getAllProtocols(protocol);
        all.insert(all.end(), subAll.begin(), subAll.end());
    }
    return all;
}

std::vector<ObjCProtocolDecl *> CSHelper::getDefineProtocols(ObjCMethodDecl *decl)
{
    std::vector<ObjCProtocolDecl *> defineProtocols;
    
    DeclContext *parent = decl->getParent();
    Decl::Kind parentKind = parent->getDeclKind();
    
    if (parentKind == Decl::ObjCInterface || parentKind == Decl::ObjCImplementation) {
        ObjCInterfaceDecl *interfaceDecl =  decl->getClassInterface();
        for (ObjCProtocolDecl *protocol : interfaceDecl->all_referenced_protocols()) {
            for (ObjCProtocolDecl *protocol : getAllProtocols(protocol)) {
                if (protocol->lookupMethod(decl->getSelector(), decl->isInstanceMethod())) {
                    defineProtocols.push_back(protocol);
                }
            }
        }
    }
    else if (parentKind == Decl::ObjCCategory || parentKind == Decl::ObjCCategoryImpl) {
        ObjCCategoryDecl *category = parentKind == Decl::ObjCCategory ? cast<ObjCCategoryDecl>(parent) : cast<ObjCCategoryImplDecl>(parent)->getCategoryDecl();
        for (ObjCProtocolDecl *protocol : category->getReferencedProtocols()) {
            for (ObjCProtocolDecl *protocol : getAllProtocols(protocol)) {
                if (protocol->lookupMethod(decl->getSelector(), decl->isInstanceMethod())) {
                    defineProtocols.push_back(protocol);
                }
            }
        }
    }
    else if (parentKind == Decl::ObjCProtocol) {
        ObjCProtocolDecl *protocolDecl = cast<ObjCProtocolDecl>(parent);
        for (ObjCProtocolDecl *protocol : getAllProtocols(protocolDecl)) {
            if (protocol->lookupMethod(decl->getSelector(), decl->isInstanceMethod())) {
                defineProtocols.push_back(protocol);
            }
        }
    }
    
    return defineProtocols;
}

std::vector<ObjCMethodDecl *> CSHelper::getDefineMethods(ObjCMethodDecl *decl)
{
    std::vector<ObjCMethodDecl *> defineMethods;
    defineMethods.push_back(decl);
    
    Selector selector = decl->getSelector();
    DeclContext *parent = decl->getParent();
    Decl::Kind parentKind = parent->getDeclKind();
    
    ObjCInterfaceDecl *interfaceDecl = nullptr;
    if (parentKind == Decl::ObjCInterface || parentKind == Decl::ObjCImplementation) {
        interfaceDecl = decl->getClassInterface();
    }
    else if (parentKind == Decl::ObjCCategory || parentKind == Decl::ObjCCategoryImpl) {
        ObjCCategoryDecl *category = parentKind == Decl::ObjCCategory ? cast<ObjCCategoryDecl>(parent) : cast<ObjCCategoryImplDecl>(parent)->getCategoryDecl();
        interfaceDecl = category->getClassInterface();
    }
    
    if (interfaceDecl) {
        ObjCMethodDecl *superMethod = nullptr;
        while ((interfaceDecl = interfaceDecl->getSuperClass())) {
            if ((superMethod = interfaceDecl->getMethod(selector, decl->isInstanceMethod()))) {
                defineMethods.push_back(superMethod);
            }
            if ((superMethod = interfaceDecl->lookupPropertyAccessor(selector, nullptr, !decl->isInstanceMethod()))) {
                defineMethods.push_back(superMethod);
            }
            for (const auto *Cat : interfaceDecl->known_categories()) {
                if ((superMethod = Cat->getMethod(selector, decl->isInstanceMethod()))) {
                    defineMethods.push_back(superMethod);
                }
            }
        }
    }
    else if (parentKind == Decl::ObjCProtocol) {
        ObjCProtocolDecl *protocolDecl = cast<ObjCProtocolDecl>(parent);
        ObjCMethodDecl *superMethod = nullptr;
        for (ObjCProtocolDecl *protocol : getAllProtocols(protocolDecl)) {
            if ((superMethod = protocol->lookupMethod(selector, decl->isInstanceMethod()))) {
                defineMethods.push_back(superMethod);
            }
        }
    }
    else {
        llvm::outs() << "\t\t"  << "[!NotHandle]" << "\t" << selector.getAsString() << "\t" << parentKind << "\n";
    }
    
    return defineMethods;
}

bool CSHelper::isNeedObfuscate(ObjCMethodDecl *decl, bool isMessage)
{
    ObjCMethodFamily family = decl->getMethodFamily();
    if (family != OMF_None && family != OMF_performSelector) {
        return false;
    }
    
    std::string selector = decl->getSelector().getAsString();
    
    bool isWrittenInScratchSpace = false;
    if (isMacroExpansion(decl, &isWrittenInScratchSpace)) {
        if (isWrittenInScratchSpace) {
            return false;
        }
        else {
            std::string filePath = getFilename(decl);
            if (!mCache->isUserSourceCode(filePath, true)) {
                return false;
            }
        }
    }
    
    if (isPropertyAccessor(decl)) {
        return false;
    }
    
    if (mCache->isIgnoreClsSelector(getClassName(decl), selector)) {
        return false;
    }
    
    bool inWhiteList = false;
    std::vector<ObjCProtocolDecl *> protocols = getDefineProtocols(decl);
    for (ObjCProtocolDecl *protocol : protocols) {
        if (mCache->ignoreProtocolSelector(protocol->getNameAsString(), decl->getSelector().getAsString())) {
            return false;
        }
        std::string filePath = getFilename(protocol);
        if (!mCache->isUserSourceCode(filePath, true)) {
            return false;
        }
        else {
            if (mCache->isInBlackList(protocol->getNameAsString())) {
                return false;
            }
            if (mCache->isInWhiteList(protocol->getNameAsString())) {
                inWhiteList = true;
            }
        }
    }
    if (inWhiteList) {
        return true;
    }
    bool inBlackList = false;
    for (ObjCMethodDecl *method : getDefineMethods(decl)) {
        if (isPropertyAccessor(method)) {
            return false;
        }
        std::string filePath = getFilename(method);
        if (!mCache->isUserSourceCode(filePath, true)) {
            return false;
        }
        else {
            if (mCache->isInBlackList(getClassName(method))) {
                inBlackList = true;
                continue;
            }
            if (mCache->isInWhiteList(getClassName(method))) {
                inBlackList = false;
                continue;
            }
        }
    }
    return !inBlackList;
}

std::string CSHelper::getClassName(ObjCMethodDecl *decl)
{
    std::string clsName = "";
    
    DeclContext *parent = decl->getParent();
    Decl::Kind parentKind = parent->getDeclKind();
    
    if (parentKind == Decl::ObjCInterface || parentKind == Decl::ObjCImplementation) {
        ObjCInterfaceDecl *interfaceDecl = decl->getClassInterface();
        if (interfaceDecl) {
            clsName = interfaceDecl->getNameAsString();
        }
    }
    else if (parentKind == Decl::ObjCCategory || parentKind == Decl::ObjCCategoryImpl) {
        ObjCCategoryDecl *category = parentKind == Decl::ObjCCategory ? cast<ObjCCategoryDecl>(parent)
                                                                       : cast<ObjCCategoryImplDecl>(parent)->getCategoryDecl();
        clsName = CSHelper::classCategoryName(category);
    }
    else if (parentKind == Decl::ObjCProtocol) {
        ObjCProtocolDecl *protocolDecl = cast<ObjCProtocolDecl>(parent);
        if (protocolDecl) {
            clsName = protocolDecl->getNameAsString();
        }
    }
    
    return clsName;
}

bool CSHelper::isNeedObfuscate(ObjCMethodDecl *decl)
{
    return isNeedObfuscate(decl, false);
}

void CSHelper::obfuscate(ObjCMethodDecl *decl)
{
    std::string clsName = getClassName(decl);
    if (clsName.length() == 0) {
        return;
    }
    
    std::string selector = decl->getSelector().getAsString();
    std::string newSelector = newSelectorName(selector);
    
    if (mCache->addClsNameSelector(clsName, selector, newSelector)) {
        addReplacement(decl);
    }
}

bool CSHelper::isNeedObfuscate(ObjCMessageExpr *expr)
{
    ObjCMethodDecl *decl = expr->getMethodDecl();
    if (!decl) {
        return false;
    }
    
    return isNeedObfuscate(decl, true);
}

void CSHelper::obfuscate(ObjCMessageExpr *expr, ASTContext *context)
{
    ObjCMethodDecl *decl = expr->getMethodDecl();
    if (!decl) {
        return;
    }
    
    std::string clsName = getClassName(decl);
    if (clsName.length() == 0) {
        if (expr->getReceiverType()->isObjCIdType()) {
            clsName = "_ObjCId_";
        }
        else {
            llvm::outs() << "\t" << "[!NotHandle] " << expr->getSelector().getAsString() << "\n";
        }
    }
    
    if (clsName.length() == 0) {
        return;
    }
    
    if (mCache->containClsName(clsName)) {
        addReplacement(expr, context);
    }
}

void CSHelper::addClassProperty(ObjCPropertyDecl *decl)
{
    ObjCMethodDecl *getterDecl = decl->getGetterMethodDecl();
    ObjCMethodDecl *setterDecl = decl->getSetterMethodDecl();
    
    std::vector<ObjCMethodDecl *> decls;
    if (getterDecl) {
        decls.push_back(getterDecl);
    }
    if (setterDecl) {
        decls.push_back(setterDecl);
    }
    
    for (ObjCMethodDecl *methodDecl : decls) {
        if (!mCache->isUserSourceCode(getFilename(methodDecl), false)) {
            continue;
        }
        
        ObjCInterfaceDecl *interfaceDecl = methodDecl->getClassInterface();
        if (interfaceDecl) {
            mCache->addClsGetterOrSetter(interfaceDecl->getNameAsString(), methodDecl->getSelector().getAsString());
        }
        else {
            DeclContext *parent = methodDecl->getParent();
            Decl::Kind parentKind = parent->getDeclKind();
            if (parentKind == Decl::ObjCProtocol) {
                ObjCProtocolDecl *protocolDecl = cast<ObjCProtocolDecl>(parent);
                if (protocolDecl) {
                    mCache->addIgnoreProtocolSelector(protocolDecl->getNameAsString(), methodDecl->getSelector().getAsString());
                }
            }
        }
    }
}

void CSHelper::addClassProtocol(ObjCContainerDecl *decl)
{
    if (!mCache->isUserSourceCode(getFilename(decl), false)) {
        return;
    }

    Decl::Kind kind = decl->getDeclKind();
    if (kind == Decl::ObjCInterface) {
        auto *tmp = cast<ObjCInterfaceDecl>(decl);
        for (ObjCProtocolDecl *protocol : tmp->all_referenced_protocols()) {
            for (ObjCProtocolDecl *protocol : getAllProtocols(protocol)) {
                if (mCache->isUserSourceCode(getFilename(protocol), false)) {
                    mCache->addClsProtocol(tmp->getNameAsString(), protocol->getNameAsString());
                }
            }
        }
    }
    else if (kind == Decl::ObjCCategory) {
        auto *tmp = cast<ObjCCategoryDecl>(decl);
        for (ObjCProtocolDecl *protocol : tmp->getReferencedProtocols()) {
            for (ObjCProtocolDecl *protocol : getAllProtocols(protocol)) {
                if (mCache->isUserSourceCode(getFilename(protocol), false)) {
                    mCache->addClsProtocol(tmp->getClassInterface()->getNameAsString(), protocol->getNameAsString());
                }
            }
        }
    }
}

void CSHelper::addProtocolSelector(ObjCProtocolDecl *decl)
{
    if (!mCache->isUserSourceCode(getFilename(decl), false)) {
        return;
    }
    
    for (ObjCMethodDecl *method : decl->methods()) {
        mCache->addProtocolSelector(decl->getNameAsString(), method->getSelector().getAsString());
    }
}

void CSHelper::addReplacement(const std::string &filePath, const Replacement &replace)
{
    if (mCache->usingLog()) {
        llvm::outs() << "\t\t" << "addReplacement" << "\t" << replace.toString() << "\n";
    }
    
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
        if (mCache->usingLog()) {
            llvm::outs() << "\t\t" << "AddReplacement\tIgnore ScratchSpace" << "\t" << selName << "\n";
        }
        return;
    }
    
    if (mCache->ignoreSelector(selName)) {
        if (mCache->usingLog()) {
            llvm::outs() << "\t\t" << "AddReplacement\tIgnore" << "\t" << selName << "\n";
        }
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

void CSHelper::addReplacement(ObjCMessageExpr *expr, ASTContext *context)
{
    Selector sel = expr->getSelector();
    
    bool isImplicitProperty = false;
    DynTypedNodeList nodeList = context->getParents(*expr);
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
    
    SourceLocation loc = getLoc(expr);
    addReplacement(sel, isImplicitProperty, loc);
}
