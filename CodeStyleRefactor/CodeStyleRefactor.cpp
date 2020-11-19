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
    bool _isTraverseImp;
    
public:
    ObjCMethodDeclVisitor(SourceManager &sm) {}
    
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

    bool TraverseImportDecl(ImportDecl *decl)
    {
        llvm::outs() << "TraverseImportDecl " << decl->getImportedModule()->getFullModuleName() << "\n";
        
        return true;
    }
    
    bool TraverseObjCProtocolDecl(ObjCProtocolDecl *decl)
    {
        if (!CSUtils::isUserSourceCode(gHelper.getFilename(decl), true)) {
            return true;
        }
        
        if (gCache.containClsName(decl->getNameAsString())) {
            return true;
        }
        
        llvm::outs() << "TraverseObjCProtocolDecl " << decl->getNameAsString() << "\n";
        
        return RecursiveASTVisitor<ObjCMethodDeclVisitor>::TraverseObjCProtocolDecl(decl);
    }
    
    bool VisitObjCProtocolDecl(const ObjCProtocolDecl *decl)
    {
        if (!decl->hasDefinition() || decl->getDefinition() != decl) {
            return true;
        }
        
        gCache.addClsName(decl->getNameAsString());
        
        llvm::outs() << "\t" << "VisitObjCProtocolDecl " << decl->getNameAsString() << "\n";
        
        return true;
    }
    
    bool TraverseObjCInterfaceDecl(ObjCInterfaceDecl *decl)
    {
        if (!CSUtils::isUserSourceCode(gHelper.getFilename(decl), true)) {
            return true;
        }
        
        if (gCache.containClsName(decl->getNameAsString())) {
            return true;
        }
        
        llvm::outs() << "TraverseObjCInterfaceDecl " << decl->getNameAsString() << "\n";
        
        return  RecursiveASTVisitor<ObjCMethodDeclVisitor>::TraverseObjCInterfaceDecl(decl);
    }
    
    bool VisitObjCInterfaceDecl(ObjCInterfaceDecl *decl)
    {
        if (!decl->hasDefinition() || decl->getDefinition() != decl) {
            return true;
        }
        
        gCache.addClsName(decl->getNameAsString());
        
        llvm::outs() << "\t" << "VisitObjCInterfaceDecl " << decl->getNameAsString() << "\n";
        
        return true;
    }
    
    bool TraverseObjCImplementationDecl(ObjCImplementationDecl *decl)
    {
        llvm::outs() << "TraverseObjCImplementationDecl " << decl->getNameAsString() << "\n";
        
        _isTraverseImp = true;
        
        bool ret =  RecursiveASTVisitor<ObjCMethodDeclVisitor>::TraverseObjCImplementationDecl(decl);
        
        _isTraverseImp = false;
        
        return ret;
    }
    
    bool VisitObjCImplementationDecl(ObjCImplementationDecl *decl)
    {
        llvm::outs() << "\t" << "VisitObjCImplementationDecl " << decl->getNameAsString() << "\n";
        
        return true;
    }
    
    bool TraverseObjCCategoryDecl(ObjCCategoryDecl *decl)
    {
        if (!CSUtils::isUserSourceCode(gHelper.getFilename(decl), true)) {
            return true;
        }
        
        std::string realName = CSHelper::classCategoryName(decl);
        
        if (gCache.containClsName(realName)) {
            return true;
        }

        llvm::outs() << "TraverseObjCCategoryDecl " << realName << "\n";
        
        return RecursiveASTVisitor<ObjCMethodDeclVisitor>::TraverseObjCCategoryDecl(decl);
    }
    
    bool VisitObjCCategoryDecl(ObjCCategoryDecl *decl)
    {
        std::string realName = CSHelper::classCategoryName(decl);
        
        gCache.addClsName(realName);
        
        llvm::outs() << "\t" << "VisitObjCCategoryDecl " << realName << "\n";
        
        return true;
    }
    
    bool TraverseObjCCategoryImplDecl(ObjCCategoryImplDecl *decl)
    {
        std::string realName = CSHelper::classCategoryName(decl);
        
        llvm::outs() << "TraverseObjCCategoryImplDecl " << realName << "\n";
        
        _isTraverseImp = true;
        
        bool ret =   RecursiveASTVisitor<ObjCMethodDeclVisitor>::TraverseObjCCategoryImplDecl(decl);
        
        _isTraverseImp = false;
        
        return ret;
    }
    
    bool VisitObjCCategoryImplDecl(ObjCCategoryImplDecl *decl)
    {
        std::string realName = CSHelper::classCategoryName(decl);
        
        llvm::outs() << "\t" << "VisitObjCCategoryImplDecl " << realName << "\n";
        
        return true;
    }
    
    bool TraverseObjCMethodDecl(ObjCMethodDecl *decl)
    {
        if (!gHelper.isNeedObfuscate(decl, true, true)) {
            return true;
        }
        
        std::string selector = decl->getSelector().getAsString();
        std::string newSelector = gHelper.newSelectorName(selector);
        
        DeclContext *parent = decl->getParent();
        Decl::Kind parentKind = parent->getDeclKind();
        
        if (parentKind == Decl::ObjCInterface || parentKind == Decl::ObjCImplementation) {
            ObjCInterfaceDecl *interfaceDecl = decl->getClassInterface();
            if (interfaceDecl) {
                if (gCache.addClsNameSelector(interfaceDecl->getNameAsString(), selector, newSelector)) {
                    gHelper.addReplacement(decl);
                }
            }
            if (parentKind == Decl::ObjCImplementation) {
                return RecursiveASTVisitor<ObjCMethodDeclVisitor>::TraverseObjCMethodDecl(decl);
            }
        }
        else if (parentKind == Decl::ObjCCategory || parentKind == Decl::ObjCCategoryImpl) {
            ObjCCategoryDecl *category = parentKind == Decl::ObjCCategory ? cast<ObjCCategoryDecl>(parent)
                                                                           : cast<ObjCCategoryImplDecl>(parent)->getCategoryDecl();

            std::string realName = CSHelper::classCategoryName(category);
            
            if (gCache.addClsNameSelector(realName, selector, newSelector)) {
                gHelper.addReplacement(decl);
            }
        }
        else if (parentKind == Decl::ObjCProtocol) {
            ObjCProtocolDecl *protocolDecl = cast<ObjCProtocolDecl>(parent);
            if (protocolDecl) {
                if (gCache.addClsNameSelector(protocolDecl->getNameAsString(), selector, newSelector)) {
                    gHelper.addReplacement(decl);
                }
            }
        }
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
        if (!CSUtils::isUserSourceCode(gHelper.getFilename(decl), false)) {
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
        std::string realName = CSHelper::classCategoryName(decl);
        
        llvm::outs() << "TraverseObjCCategoryImplDecl " << realName << "\n";
        
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
        
        DeclContext *parent = mDecl->getParent();
        Decl::Kind parentKind = parent->getDeclKind();
        
        std::string interfaceName;
        std::string fileName;
        
        if (parentKind == Decl::ObjCInterface || parentKind == Decl::ObjCImplementation) {
            ObjCInterfaceDecl *interface = mDecl->isClassMethod() ? mDecl->getClassInterface() : expr->getReceiverInterface();
            if (interface) {
                interfaceName = interface->getNameAsString();
                fileName = gHelper.getFilename(interface);
            }
        }
        else if (parentKind == Decl::ObjCCategory || parentKind == Decl::ObjCCategoryImpl) {
            if (parentKind == Decl::ObjCCategory) {
                auto *tmp = cast<ObjCCategoryDecl>(parent);
                if (tmp) {
                    interfaceName = CSHelper::classCategoryName(tmp);
                    fileName = gHelper.getFilename(tmp);
                }
            }
            else if (parentKind == Decl::ObjCCategoryImpl) {
                auto *tmp = cast<ObjCCategoryImplDecl>(parent);
                if (tmp) {
                    interfaceName = CSHelper::classCategoryName(tmp);
                    fileName = gHelper.getFilename(tmp);
                }
            }
        }
        else if (parentKind == Decl::ObjCProtocol) {
            ObjCProtocolDecl *protocolDecl = cast<ObjCProtocolDecl>(parent);
            if (protocolDecl) {
                interfaceName = protocolDecl->getNameAsString();
                fileName = gHelper.getFilename(protocolDecl);
            }
        }
        else if (expr->getReceiverType()->isObjCIdType()) {
            interfaceName = "_ObjCId_";
            fileName = gHelper.getFilename(expr);
        }
        else {
            llvm::outs() << "\t" << "[!NotHandle] " << expr->getSelector().getAsString() << "\n";
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
    
    unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef InFile) override
    {
        if (!CSUtils::isUserSourceCode(InFile.str(), false)) {
            llvm::outs() << "[Ignore] file:" << InFile.str() << "\n";
            return unique_ptr<ASTConsumer>();
        }
        
        llvm::outs() << " [Analize] file:" << InFile.str() << "\n";
        
        if (runSelectorComsumer) {
            return unique_ptr<CodeStyleSelectorConsumer> (new CodeStyleSelectorConsumer(CI));
        }
        return unique_ptr<CodeStyleASTConsumer> (new CodeStyleASTConsumer(_refactoringTool.getReplacements(), CI, InFile));
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

void applyChangeToFiles(RefactoringTool &tool)
{
    IntrusiveRefCntPtr<DiagnosticOptions> diagOpts = new DiagnosticOptions();
    DiagnosticsEngine diagnostics(IntrusiveRefCntPtr<DiagnosticIDs>(new DiagnosticIDs()),
                                  &*diagOpts,
                                  new TextDiagnosticPrinter(llvm::errs(), &*diagOpts),
                                  true);

    SourceManager sources(diagnostics, tool.getFiles());
    
    Rewriter rewriter(sources, LangOptions());
    tool.applyAllReplacements(rewriter);
    
    for (Rewriter::buffer_iterator I = rewriter.buffer_begin(), E = rewriter.buffer_end(); I != E; ++I) {
//        I->second.write(llvm::outs());
    }
    
    rewriter.overwriteChangedFiles();
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
    
    RefactoringTool tool(op.getCompilations(), analizeFiles);
    
    std::unique_ptr<FrontendActionFactory> factory(new CodeStyleActionFactory(tool));
    
    llvm::SmallString<256> path(sourceDir);
    llvm::sys::path::append(path, "selectors.txt");
    std::string selectorFilePath = std::string(path.str());
    if (sys::fs::exists(selectorFilePath)) {
        gCache.loadIgnoreSelectors(selectorFilePath);
    }
    else {
        runSelectorComsumer = true;
        if (int Result = tool.run(factory.get())) {
            return Result;
        }
        gCache.saveIgnoreSelectors(selectorFilePath);
    }
    
    gCache.addClsName("_ObjCId_");
    gHelper.setSelectorPrefix("sealion_");
    gHelper.setReplacementsMap(&tool.getReplacements());
    gHelper.setCache(&gCache);
    
    runSelectorComsumer = false;
    if (int Result = tool.run(factory.get())) {
        return Result;
    }
    
    applyChangeToFiles(tool);
    
    llvm::outs() << "[Finish] total files:" << analizeFiles.size() <<" cost time:" << CSUtils::getCurrentTimestamp() - start << "(ms)\n";
    return 0;
}
