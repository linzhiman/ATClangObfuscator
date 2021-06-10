#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/Signals.h"

#include "CSUtils.hpp"
#include "CSCache.hpp"
#include "CSHelper.hpp"

using namespace llvm;
using namespace clang;
using namespace clang::tooling;

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
    
    bool TraverseObjCProtocolDecl(ObjCProtocolDecl *decl)
    {
        if (!gCache.isUserSourceCode(gHelper.getFilename(decl), false)) {
            return true;
        }
        
        std::string clsName = decl->getNameAsString();
        
        if (gCache.containClsName(clsName)) {
            return true;
        }
        
        if (gCache.usingLog()) {
            llvm::outs() << "TraverseObjCProtocolDecl " << clsName << "\n";
        }
        
        return RecursiveASTVisitor<ObjCMethodDeclVisitor>::TraverseObjCProtocolDecl(decl);
    }
    
    bool VisitObjCProtocolDecl(const ObjCProtocolDecl *decl)
    {
        if (!decl->hasDefinition() || decl->getDefinition() != decl) {
            return true;
        }
        
        std::string clsName = decl->getNameAsString();
        
        gCache.addClsName(clsName);
        
        if (gCache.usingLog()) {
            llvm::outs() << "\t" << "VisitObjCProtocolDecl " << clsName << "\n";
        }
        
        return true;
    }
    
    bool TraverseObjCInterfaceDecl(ObjCInterfaceDecl *decl)
    {
        if (!gCache.isUserSourceCode(gHelper.getFilename(decl), false)) {
            return true;
        }
        
        std::string clsName = decl->getNameAsString();
        
        if (gCache.containClsName(clsName)) {
            return true;
        }
        
        if (gCache.usingLog()) {
            llvm::outs() << "TraverseObjCInterfaceDecl " << clsName << "\n";
        }
        
        return  RecursiveASTVisitor<ObjCMethodDeclVisitor>::TraverseObjCInterfaceDecl(decl);
    }
    
    bool VisitObjCInterfaceDecl(ObjCInterfaceDecl *decl)
    {
        if (!decl->hasDefinition() || decl->getDefinition() != decl) {
            return true;
        }
        
        std::string clsName = decl->getNameAsString();
        
        gCache.addClsName(clsName);
        
        if (gCache.usingLog()) {
            llvm::outs() << "\t" << "VisitObjCInterfaceDecl " << clsName << "\n";
        }
        
        return true;
    }
    
    bool TraverseObjCImplementationDecl(ObjCImplementationDecl *decl)
    {
        if (gCache.usingLog()) {
            llvm::outs() << "TraverseObjCImplementationDecl " << decl->getNameAsString() << "\n";
        }
        
        _isTraverseImp = true;
        
        bool ret =  RecursiveASTVisitor<ObjCMethodDeclVisitor>::TraverseObjCImplementationDecl(decl);
        
        _isTraverseImp = false;
        
        return ret;
    }
    
    bool VisitObjCImplementationDecl(ObjCImplementationDecl *decl)
    {
        if (gCache.usingLog()) {
            llvm::outs() << "\t" << "VisitObjCImplementationDecl " << decl->getNameAsString() << "\n";
        }
        
        return true;
    }
    
    bool TraverseObjCCategoryDecl(ObjCCategoryDecl *decl)
    {
        if (!gCache.isUserSourceCode(gHelper.getFilename(decl), false)) {
            return true;
        }
        
        std::string clsName = CSHelper::classCategoryName(decl);
        
        if (gCache.containClsName(clsName)) {
            return true;
        }

        if (gCache.usingLog()) {
            llvm::outs() << "TraverseObjCCategoryDecl " << clsName << "\n";
        }
        
        return RecursiveASTVisitor<ObjCMethodDeclVisitor>::TraverseObjCCategoryDecl(decl);
    }
    
    bool VisitObjCCategoryDecl(ObjCCategoryDecl *decl)
    {
        std::string clsName = CSHelper::classCategoryName(decl);
        
        gCache.addClsName(clsName);
        
        if (gCache.usingLog()) {
            llvm::outs() << "\t" << "VisitObjCCategoryDecl " << clsName << "\n";
        }
        
        return true;
    }
    
    bool TraverseObjCCategoryImplDecl(ObjCCategoryImplDecl *decl)
    {
        std::string clsName = CSHelper::classCategoryName(decl);
        
        if (gCache.usingLog()) {
            llvm::outs() << "TraverseObjCCategoryImplDecl " << clsName << "\n";
        }
        
        _isTraverseImp = true;
        
        bool ret = RecursiveASTVisitor<ObjCMethodDeclVisitor>::TraverseObjCCategoryImplDecl(decl);
        
        _isTraverseImp = false;
        
        return ret;
    }
    
    bool VisitObjCCategoryImplDecl(ObjCCategoryImplDecl *decl)
    {
        std::string clsName = CSHelper::classCategoryName(decl);
        
        if (!gCache.containClsName(clsName)) {
            gCache.addClsName(clsName);
        }
        
        if (gCache.usingLog()) {
            llvm::outs() << "\t" << "VisitObjCCategoryImplDecl " << clsName << "\n";
        }
        
        return true;
    }
    
    bool TraverseObjCMethodDecl(ObjCMethodDecl *decl)
    {
        if (gHelper.isNeedObfuscate(decl)) {
            gHelper.obfuscate(decl);
        }
        return true;
    }
};

class ObjCMessageExprVisitor : public RecursiveASTVisitor<ObjCMessageExprVisitor>
{
private:
    ASTContext *_astContext;
    bool _isTraverseImp;
    
public:
    ObjCMessageExprVisitor(SourceManager &sm)
    : _isTraverseImp(false) {}
    
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
        if (!gCache.isUserSourceCode(gHelper.getFilename(decl), false)) {
            return true;
        }
        
        if (gCache.usingLog()) {
            llvm::outs() << "TraverseObjCImplementationDecl " << decl->getNameAsString() << "\n";
        }
        
        _isTraverseImp = true;
        
        bool ret =  RecursiveASTVisitor<ObjCMessageExprVisitor>::TraverseObjCImplementationDecl(decl);
        
        _isTraverseImp = false;
        
        return ret;
    }
    
    bool TraverseObjCCategoryImplDecl(ObjCCategoryImplDecl *decl)
    {
        std::string clsName = CSHelper::classCategoryName(decl);
        
        if (gCache.usingLog()) {
            llvm::outs() << "TraverseObjCCategoryImplDecl " << clsName << "\n";
        }
        
        _isTraverseImp = true;
        
        bool ret =  RecursiveASTVisitor<ObjCMessageExprVisitor>::TraverseObjCCategoryImplDecl(decl);
        
        _isTraverseImp = false;
        
        return ret;
    }
    
    bool VisitObjCMessageExpr(ObjCMessageExpr *expr)
    {
        if (gHelper.isNeedObfuscate(expr)) {
            gHelper.obfuscate(expr, _astContext);
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
        
        if (gCache.usingLog()) {
            llvm::outs() << "\nTranslationUnit finish:" << CSUtils::getCurrentTimestamp() - start << "\n";
        }
    }
};

class CodeStyleSelectorVisitor : public RecursiveASTVisitor<CodeStyleSelectorVisitor>
{
public:
    bool shouldVisitImplicitCode() const { return true; }
    
    bool TraverseObjCPropertyDecl(ObjCPropertyDecl *decl)
    {
//        llvm::outs() << "TraverseObjCPropertyDecl:" << decl->getNameAsString() << "\n";
        
        gHelper.addClassProperty(decl);
        
        return RecursiveASTVisitor<CodeStyleSelectorVisitor>::TraverseObjCPropertyDecl(decl);
    }
    
    bool VisitObjCProtocolDecl(ObjCProtocolDecl *decl)
    {
        if (!decl->hasDefinition() || decl->getDefinition() != decl) {
            return true;
        }
        
        gHelper.addProtocolSelector(decl);
        
        return true;
    }
    
    bool VisitObjCInterfaceDecl(ObjCInterfaceDecl *decl)
    {
        if (!decl->hasDefinition() || decl->getDefinition() != decl) {
            return true;
        }
        
        gHelper.addClassProtocol(decl);
        
        return true;
    }
    
    bool VisitObjCCategoryDecl(ObjCCategoryDecl *decl)
    {
        gHelper.addClassProtocol(decl);
        
        return true;
    }
    
    bool VisitObjCSelectorExpr(ObjCSelectorExpr *expr)
    {
        Selector sel = expr->getSelector();
        
        if (gCache.usingLog()) {
            llvm::outs() << "SelectorExpr" << "\t" << sel.getAsString() << "\n";
        }
        
        gCache.addIgnoreSelector(sel.getAsString());
        
        return true;
    }
};

class CodeStyleSelectorConsumer: public ASTConsumer
{
private:
    CodeStyleSelectorVisitor visitor;
public:
    explicit CodeStyleSelectorConsumer(CompilerInstance &CI) {
        gHelper.setSourceManager(&CI.getSourceManager());
    }
    
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
        if (!gCache.isUserSourceCode(InFile.str(), false)) {
            if (gCache.usingLog()) {
                llvm::outs() << "\n[Ignore] file:" << InFile.str() << "\n";
            }
            return unique_ptr<ASTConsumer>();
        }
        
        if (gCache.usingLog()) {
            llvm::outs() << "\n[Analize] file:" << InFile.str() << "\n";
        }
        
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
    if (analizeFiles.size() != 1) {
        llvm::outs() << "\n[Error] only support one source path";
        return 0;
    }
    
    std::string sourceDir = analizeFiles.front();
    if (sys::fs::is_regular_file(sourceDir)) {
        llvm::outs() << "\n[Error] only support folder";
        return 0;
    }
    
    long long start =  CSUtils::getCurrentTimestamp();
    
    std::string configFilePath = sourceDir + "obfuscatorConfig.txt";
    if (sys::fs::exists(configFilePath)) {
        gCache.loadConfig(configFilePath);
    }

    analizeFiles = gCache.filterNotUserSourceCode(op.getCompilations().getAllFiles());
    
    RefactoringTool tool(op.getCompilations(), analizeFiles);
    
    std::unique_ptr<FrontendActionFactory> factory(new CodeStyleActionFactory(tool));
    
    gHelper.setSelectorPrefix(gCache.getSelectorPrefix(), &gCache.getOriSelectorPrefix());
    gHelper.setReplacementsMap(&tool.getReplacements());
    gHelper.setCache(&gCache);
    
    std::string selectorFilePath = sourceDir + "obfuscatorCache.txt";
    if (gCache.usingCache() && sys::fs::exists(selectorFilePath)) {
        gCache.loadCache(selectorFilePath);
    }
    else {
        runSelectorComsumer = true;
        if (int Result = tool.run(factory.get())) {
            return Result;
        }
        gCache.genIgnoreProtocolSelector();
        gCache.saveCache(selectorFilePath);
    }
    
    gCache.clearClsName();
    gCache.addClsName("_ObjCId_");
    
    runSelectorComsumer = false;
    if (int Result = tool.run(factory.get())) {
        return Result;
    }

    applyChangeToFiles(tool);
    
    llvm::outs() << "\n[Finish] total files:" << analizeFiles.size() <<" cost time:" << CSUtils::getCurrentTimestamp() - start << "(ms)\n";
    return 0;
}
