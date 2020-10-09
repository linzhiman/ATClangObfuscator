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
#include "CSCache.hpp"
#include "CSHelper.hpp"

using namespace llvm;
using namespace clang;
using namespace clang::tooling;
using namespace std;

bool runSelectorComsumer;

CSCache gCache;
CSHelper gHelper;

class ObjCMethodDeclVisitor : public RecursiveASTVisitor<ObjCMethodDeclVisitor>
{
private:
    SourceManager &_sm;
    bool _isTraverseImp;
    
public:
    ObjCMethodDeclVisitor(SourceManager &sm)
    : _sm(sm) {}
    
    bool TraverseDeclContextHelper(DeclContext *DC)
    {
        if (!DC) {
          return true;
        }
        
        if (DC->isTranslationUnit() || DC->isObjCContainer() || DC->isFunctionOrMethod()) {
            for (auto *Child : DC->decls()) {
                if (!canIgnoreChildDeclWhileTraversingDeclContext(Child)) {
                    TraverseDecl(Child);
                }
            }
        }
        return true;
    }

    bool TraverseImportDecl(ImportDecl * decl)
    {
        llvm::outs() << "TraverseImportDecl " << decl->getImportedModule()->getFullModuleName() << "\n";
        return true;
    }
    
    bool TraverseObjCProtocolDecl(ObjCProtocolDecl * decl)
    {
        StringRef filePath = _sm.getFilename(decl->getSourceRange().getBegin());
        if (!CSUtils::isUserSourceCode(filePath.str(), true)) {
            return true;
        }
        
        if (gCache.containClsName(decl->getNameAsString())) {
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
        
        if (gCache.containClsName(decl->getNameAsString())) {
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
        std::string realName = CSHelper::classCategoryName(className, categoryName);
        
        if (gCache.containClsName(realName)) {
            return true;
        }

        llvm::outs() << "TraverseObjCCategoryDecl " << realName << "\n";
        
        return RecursiveASTVisitor<ObjCMethodDeclVisitor>::TraverseObjCCategoryDecl(decl);
    }
    
    bool TraverseObjCCategoryImplDecl(ObjCCategoryImplDecl *decl)
    {
        std::string categoryName = decl->getNameAsString();
        std::string className = decl->getClassInterface()->getNameAsString();
        std::string realName = CSHelper::classCategoryName(className, categoryName);
        
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
        if (!gHelper.isNeedObfuscate(decl, true, true)) {
            return true;
        }
        
        std::string selector = decl->getSelector().getAsString();
        
        DeclContext *parent = decl->getParent();
        Decl::Kind parentKind = parent->getDeclKind();
        if (parentKind == Decl::ObjCInterface) {
            ObjCInterfaceDecl *interfaceDecl =  decl->getClassInterface();
            if (interfaceDecl) {
                if (gCache.addClsNameSelector(interfaceDecl->getNameAsString(), selector, gHelper.newSelectorName(selector))) {
                    gHelper.addReplacement(decl);
                }
            }
        } else if (parentKind == Decl::ObjCImplementation) {
            ObjCInterfaceDecl *interfaceDecl =  decl->getClassInterface();
            if (interfaceDecl) {
                if (gCache.addClsNameSelector(interfaceDecl->getNameAsString(), selector, gHelper.newSelectorName(selector))) {
                    gHelper.addReplacement(decl);
                }
            }
            
            return RecursiveASTVisitor<ObjCMethodDeclVisitor>::TraverseObjCMethodDecl(decl);
            
        } else if (parentKind == Decl::ObjCCategory || parentKind == Decl::ObjCCategoryImpl) {
            ObjCInterfaceDecl *interfaceDecl =  decl->getClassInterface();
            ObjCCategoryDecl  *category = parentKind == Decl::ObjCCategory ? cast<ObjCCategoryDecl>(parent)
                                                                           : cast<ObjCCategoryImplDecl>(parent)->getCategoryDecl();
            
            std::string catName = category->getNameAsString();
            std::string clsName = interfaceDecl->getNameAsString();
            std::string realName = CSHelper::classCategoryName(clsName, catName);
            
            if (gCache.addClsNameSelector(realName, selector, gHelper.newSelectorName(selector))) {
                gHelper.addReplacement(decl);
            }
            
        } else if (parentKind == Decl::ObjCProtocol) {
            ObjCProtocolDecl *protocolDecl = cast<ObjCProtocolDecl>(parent);
            if (protocolDecl) {
                if (gCache.addClsNameSelector(protocolDecl->getNameAsString(), selector, gHelper.newSelectorName(selector))) {
                    gHelper.addReplacement(decl);
                }
            }
        }
        return true;
    }
    
    bool VisitObjCInterfaceDecl(ObjCInterfaceDecl *decl)
    {
        if (!decl->hasDefinition() || decl->getDefinition() != decl) {
            return true;
        }
        gCache.addClsName(decl->getNameAsString());
        
        return true;
    }
    
    bool VisitObjCCategoryDecl(ObjCCategoryDecl * decl)
    {
        std::string categoryName = decl->getNameAsString();
        std::string className = decl->getClassInterface()->getNameAsString();
        
        std::string name = CSHelper::classCategoryName(className, categoryName);
        gCache.addClsName(name);
        
        return true;
    }
    
    bool VisitObjCImplementationDecl(ObjCImplementationDecl *decl)
    {
        return true;
    }
    
    bool VisitObjCProtocolDecl(const ObjCProtocolDecl *decl)
    {
        if (!decl->hasDefinition() || decl->getDefinition() != decl) {
            return true;
        }
        gCache.addClsName(decl->getNameAsString());
        
        return true;
    }
};

class ObjCMessageExprVisitor : public RecursiveASTVisitor<ObjCMessageExprVisitor>
{
private:
    ASTContext *_astContext;
    SourceManager &_sm;
    bool _isTraverseImp;
    
public:
    ObjCMessageExprVisitor(SourceManager &sm)
    : _sm(sm), _isTraverseImp(false) {}
    
    void setContext(ASTContext *context)
    {
        _astContext = context;
    }
    
    bool TraverseDeclContextHelper(DeclContext *DC)
    {
        if (!DC) {
          return true;
        }
        
        if (_isTraverseImp || DC->isTranslationUnit() || DC->getDeclKind() == Decl::ObjCImplementation
            || DC->getDeclKind() == Decl::ObjCCategoryImpl) {
            for (auto *Child : DC->decls()) {
                if (!canIgnoreChildDeclWhileTraversingDeclContext(Child)) {
                  TraverseDecl(Child);
                }
            }
        }
        
        return true;
    }
    
    bool TraverseObjCImplementationDecl(ObjCImplementationDecl *decl)
    {
        StringRef filePath = gHelper.getFilename(decl);
        if(!CSUtils::isUserSourceCode(filePath.str(), false)) {
            return true;
        }
        
        llvm::outs() << "TraverseObjCImplementationDecl " << decl->getNameAsString() << "\n";
        
        _isTraverseImp = true;
        
        bool ret =  RecursiveASTVisitor<ObjCMessageExprVisitor>::TraverseObjCImplementationDecl(decl);
        
        _isTraverseImp = false;
        
        return ret;
    }
    
    bool TraverseObjCCategoryImplDecl(ObjCCategoryImplDecl *decl)
    {
        _isTraverseImp = true;
        
        bool ret =  RecursiveASTVisitor<ObjCMessageExprVisitor>::TraverseObjCCategoryImplDecl(decl);
        
        _isTraverseImp = false;
        
        return ret;
    }
    
    bool VisitObjCMessageExpr(ObjCMessageExpr *expr)
    {
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
        
        if (!gHelper.isNeedObfuscate(mDecl, false, !isDefineInMacro)) {
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
                    interfaceName = CSHelper::classCategoryName(tmp->getClassInterface()->getNameAsString(), tmp->getNameAsString());
                    fileName = _sm.getFilename(tmp->getSourceRange().getBegin()).str();
                }
            }
            else if (parentKind == Decl::ObjCCategoryImpl) {
                auto *tmp = cast<ObjCCategoryImplDecl>(parent);
                if (tmp) {
                    interfaceName = CSHelper::classCategoryName(tmp->getClassInterface()->getNameAsString(), tmp->getNameAsString());
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
            if (!gCache.containClsName(interfaceName)) {
                return true;
            }
            if (CSUtils::isUserSourceCode(fileName, false)) {
                gHelper.addReplacement(expr, *_astContext);
           }
        }
        
        return true;
    }
};

class CodeStyleASTConsumer: public ASTConsumer
{
private:
    ObjCMethodDeclVisitor methodVisitor;
    ObjCMessageExprVisitor messageExprVisitor;
    
public:
    CodeStyleASTConsumer(std::map<std::string, Replacements> &replacementsMap, CompilerInstance &CI, StringRef file)
    : methodVisitor(CI.getSourceManager())
    , messageExprVisitor(CI.getSourceManager()) {
        gHelper.setSourceManager(&CI.getSourceManager());
    }
    
    ~CodeStyleASTConsumer() {}
    
    void HandleTranslationUnit(ASTContext &context)
    {
        long long start =  CSUtils::getCurrentTimestamp();
        
        methodVisitor.TraverseTranslationUnitDecl(context.getTranslationUnitDecl());
        
        messageExprVisitor.setContext(&context);
        messageExprVisitor.TraverseTranslationUnitDecl(context.getTranslationUnitDecl());
        
        llvm::outs() << "TranslationUnit finish:" << CSUtils::getCurrentTimestamp() - start << "\n";
    }
};

class CodeStyleSelectorVisitor : public RecursiveASTVisitor<CodeStyleSelectorVisitor>
{
public:
    bool shouldVisitImplicitCode() const { return true; }
    
    bool VisitObjCSelectorExpr(ObjCSelectorExpr *expr)
    {
        Selector sel = expr->getSelector();
        
        llvm::outs() << "SelectorExpr" << "\t" << sel.getAsString() << "\n";
        
        gCache.addIgnoreSelector(sel.getAsString());
        
        return true;
    }
};

class CodeStyleSelectorConsumer: public ASTConsumer
{
private:
    CodeStyleSelectorVisitor visitor;
public:
    explicit CodeStyleSelectorConsumer(CompilerInstance &CI) {}
    
    virtual void HandleTranslationUnit(ASTContext &Context)
    {
        visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }
};

class CodeStyleASTAction: public ASTFrontendAction
{
private:
    RefactoringTool &_refactoringTool;
    
public:
    CodeStyleASTAction (RefactoringTool &refactoingTool)
    : ASTFrontendAction(), _refactoringTool(refactoingTool) {}
    
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
        return unique_ptr<CodeStyleASTConsumer> (new CodeStyleASTConsumer(_refactoringTool.getReplacements(), CI, iFile));
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
    : FrontendActionFactory(), _refactoringTool(tool) {}
    
    std::unique_ptr<FrontendAction> create() override
    {
        return unique_ptr<FrontendAction>(new CodeStyleASTAction(_refactoringTool));
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
        I->second.write(llvm::outs());
    }
    
    Rewrite.overwriteChangedFiles();
}

int main(int argc, const char **argv)
{
    llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);
    
    llvm::cl::OptionCategory category("CodeStyleRefactor");
    CommonOptionsParser op(argc, argv, category);
    
    std::vector<std::string> analizeFiles = op.getSourcePathList();
    std::string sourceDir;
    if (analizeFiles.size() == 1) {
        std::string path = analizeFiles.front();
        if (sys::fs::is_regular_file(path)) {
            sourceDir = path.substr(0, path.rfind('\\'));
        }
        else {
            sourceDir = path;
            analizeFiles = CSUtils::filterNotUserSourceCode(op.getCompilations().getAllFiles());
        }
    }
    else {
        llvm::outs() << "[Error] only support one source path";
        return 0;
    }
    
    long long start =  CSUtils::getCurrentTimestamp();
    
    RefactoringTool Tool(op.getCompilations(), analizeFiles);
    
    std::unique_ptr<FrontendActionFactory> factory (new CodeStyleActionFactory(Tool));
    
    llvm::SmallString<256> Path(sourceDir);
    llvm::sys::path::append(Path, "selectors.txt");
    std::string selectorFilePath = std::string(Path.str());
    if (sys::fs::exists(selectorFilePath)) {
        gCache.loadIgnoreSelectors(selectorFilePath);
    }
    else {
        runSelectorComsumer = true;
        if (int Result = Tool.run(factory.get())) {
            return Result;
        }
        gCache.saveIgnoreSelectors(selectorFilePath);
    }
    
    gCache.addClsName("_ObjCId_");
    gHelper.setSelectorPrefix("sealion_");
    gHelper.setReplacementsMap(&Tool.getReplacements());
    gHelper.setCache(&gCache);
    
    runSelectorComsumer = false;
    if (int Result = Tool.run(factory.get())) {
        return Result;
    }
    
    applyChangeToFiles(Tool);
    
    llvm::outs() << "[Finish] total files:" << analizeFiles.size() <<" cost time:" << CSUtils::getCurrentTimestamp() - start << "(ms)\n";
    return 0;
}
