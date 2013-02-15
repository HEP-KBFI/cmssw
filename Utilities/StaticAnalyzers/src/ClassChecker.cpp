// ClassChecker.cpp by Patrick Gartung (gartung@fnal.gov)
//
// Objectives of this checker
//
// For each special function of a class (produce, beginrun, endrun, beginlumi, endlumi)
//
//	1) identify member data being modified
//		built-in types reseting values
//		calling non-const member function object if member data is an object
//	2) for each non-const member functions of self called
//		do 1) above
//	3) for each non member function (external) passed in a member object
//		complain if arguement passed in by non-const ref
//		pass by value OK
//		pass by const ref & pointer OK
//
//


#include "ClassChecker.h"

using namespace clang;
using namespace clang::ento;
using namespace llvm;

namespace clangcms {

class WalkAST : public clang::StmtVisitor<WalkAST> {
  clang::ento::BugReporter &BR;
  clang::AnalysisDeclContext *AC;

  typedef const clang::CXXMemberCallExpr * WorkListUnit;
  typedef clang::SmallVector<WorkListUnit, 50> DFSWorkList;
  typedef const clang::Stmt * StmtListUnit;
  typedef clang::SmallVector<StmtListUnit, 50> StmtList;


  /// A vector representing the worklist which has a chain of CallExprs.
  DFSWorkList WList;
  StmtList SList;
  
  // PreVisited : A CallExpr to this FunctionDecl is in the worklist, but the
  // body has not been visited yet.
  // PostVisited : A CallExpr to this FunctionDecl is in the worklist, and the
  // body has been visited.
  enum Kind { NotVisited,
              PreVisited,  /**< A CallExpr to this FunctionDecl is in the 
                                worklist, but the body has not yet been
                                visited. */
              PostVisited  /**< A CallExpr to this FunctionDecl is in the
                                worklist, and the body has been visited. */
  };

  /// A DenseMap that records visited states of FunctionDecls.
  llvm::DenseMap<const clang::FunctionDecl *, Kind> VisitedFunctions;

  /// The CallExpr whose body is currently being visited.  This is used for
  /// generating bug reports.  This is null while visiting the body of a
  /// constructor or destructor.
  const clang::CXXMemberCallExpr *visitingCallExpr;

public:
  WalkAST(clang::ento::BugReporter &br, clang::AnalysisDeclContext *ac)
    : BR(br),
      AC(ac),
      visitingCallExpr(0) {}

 

  bool hasWork() const { return !WList.empty(); }

  /// This method adds a CallExpr to the worklist 
  void Enqueue(WorkListUnit WLUnit) {
    WList.push_back(WLUnit);
  }

  /// This method returns an item from the worklist without removing it.
  WorkListUnit Dequeue() {
    assert(!WList.empty());
    return WList.back();    
  }
  
  void Execute() {
      WorkListUnit WLUnit = Dequeue();
      if (visitingCallExpr && WLUnit->getMethodDecl() == visitingCallExpr->getMethodDecl()) {
//		llvm::errs()<<"\nRecursive call to ";
//		WLUnit->getDirectCallee()->printName(llvm::errs());
//		llvm::errs()<<" , ";
//		WLUnit->dumpPretty(AC->getASTContext());
//		llvm::errs()<<"\n";
   		WList.pop_back();
		return;
		}
      const clang::CXXMethodDecl *FD = clang::dyn_cast<clang::CXXMethodDecl>(WLUnit->getMethodDecl());
      llvm::SaveAndRestore<const clang::CXXMemberCallExpr *> SaveCall(visitingCallExpr, WLUnit);
      if (FD && FD->hasBody()) Visit(FD->getBody());
      WList.pop_back();
  }

const clang::Stmt * ParentStmt() {
	if (!SList.empty()) {
		if ( visitingCallExpr ){
 	  		for (llvm::SmallVectorImpl<const clang::Stmt *>::iterator 
				I = SList.begin(), E = SList.end(); I != E; I++) {
				if ((*I)==visitingCallExpr ) {
					return (*(I+1));
				}
			}
		}
		else {
			return ( *(SList.begin()));
		}
	}
	return 0;
  }


  void SListDump(llvm::raw_ostream & os) {
  	clang::LangOptions LangOpts;
  	LangOpts.CPlusPlus = true;
  	clang::PrintingPolicy Policy(LangOpts);
  	ParentStmt()->printPretty(os,0,Policy);
  }
  
  void WListDump(llvm::raw_ostream & os) {
  	clang::LangOptions LangOpts;
  	LangOpts.CPlusPlus = true;
  	clang::PrintingPolicy Policy(LangOpts);
	if (!WList.empty()) {
		for (llvm::SmallVectorImpl<const clang::CXXMemberCallExpr *>::iterator 
			I = WList.begin(), E = WList.end(); I != E; I++) {
			(*I)->printPretty(os, 0 , Policy);
			os <<" ";
		}
	}       
  }

  // Stmt visitor methods.
  void VisitChildren(clang::Stmt *S);
  void VisitStmt( clang::Stmt *S) { VisitChildren(S); }
  void VisitMemberExpr(clang::MemberExpr *E);
  void VisitCXXMemberCallExpr(clang::CXXMemberCallExpr *CE);
  void VisitDeclRefExpr(clang::DeclRefExpr * DRE);
  void CXXOperatorCallExpr(const clang::CXXOperatorCallExpr *CE);
  void BinaryOperator(const clang::BinaryOperator * BO);
  void UnaryOperator(const clang::UnaryOperator * UO);
  void ExplicitCastExpr(const clang::ExplicitCastExpr * CE);
  void ReturnStmt(const clang::ReturnStmt * RS);
  void ReportCast(const clang::ExplicitCastExpr *CE);
  void ReportMember(const clang::MemberExpr *ME);
  void ReportCallReturn(const clang::ReturnStmt * RS);
  void ReportCallArg(const clang::CXXMemberCallExpr *CE, const int i);
};

//===----------------------------------------------------------------------===//
// AST walking.
//===----------------------------------------------------------------------===//

void WalkAST::VisitChildren( clang::Stmt *S) {
  for (clang::Stmt::child_iterator I = S->child_begin(), E = S->child_end(); I!=E; ++I)
    if (clang::Stmt *child = *I) {
      SList.push_back(child);
      Visit(child);
      SList.pop_back();
    }
}


void WalkAST::BinaryOperator(const clang::BinaryOperator * BO) {
//	BO->dump(); 
//	llvm::errs()<<"\n";
if (BO->isAssignmentOp()) {
	if (clang::MemberExpr * ME = dyn_cast<clang::MemberExpr>(BO->getLHS()->IgnoreParenImpCasts())){
			ReportMember(ME);
		}
	}
}
void WalkAST::UnaryOperator(const clang::UnaryOperator * UO) {
//	UO->dump(); 
//	llvm::errs()<<"\n";
  if (UO->isIncrementDecrementOp())  {
	if (clang::MemberExpr * ME = dyn_cast<clang::MemberExpr>(UO->getSubExpr()->IgnoreParenImpCasts())) {
		ReportMember(ME);
	}
  }
}

void WalkAST::CXXOperatorCallExpr(const clang::CXXOperatorCallExpr *OCE) {
// OCE->dump(); 
//  llvm::errs()<<"\n";
switch ( OCE->getOperator() ) {

	case OO_Equal:	
	case OO_PlusEqual:
	case OO_MinusEqual:
	case OO_StarEqual:
	case OO_SlashEqual:
	case OO_PercentEqual:
	case OO_AmpEqual:
	case OO_PipeEqual:
	case OO_LessLessEqual:
	case OO_GreaterGreaterEqual:
	if (const clang::MemberExpr * ME = dyn_cast<clang::MemberExpr>(OCE->arg_begin()->IgnoreParenImpCasts())){
			ReportMember(ME);
	}


	case OO_PlusPlus:
	case OO_MinusMinus:
	if (const clang::MemberExpr * ME = dyn_cast<clang::MemberExpr>(OCE->arg_begin()->IgnoreParenImpCasts())) {
			ReportMember(ME);
	}
	
		
}

}



void WalkAST::ExplicitCastExpr(const clang::ExplicitCastExpr * CE){
//	CE->dump();
//	llvm::errs()<<"\n";

	const clang::Expr *E = CE->getSubExpr();
	clang::ASTContext &Ctx = AC->getASTContext();
	clang::QualType OrigTy = Ctx.getCanonicalType(E->getType());
	clang::QualType ToTy = Ctx.getCanonicalType(CE->getType());

//	if (const clang::MemberExpr * ME = dyn_cast<clang::MemberExpr>(E->IgnoreParenImpCasts())) {
	if ( support::isConst( OrigTy ) && ! support::isConst(ToTy) )
		ReportCast(CE);
//	}
}
 

void WalkAST::ReturnStmt(const clang::ReturnStmt * RS){
	if (const clang::Expr * RE = RS->getRetValue()) {
		clang::QualType RQT = RE->getType();
		if ( llvm::isa<clang::CXXNewExpr>(RE) ) return; 
		if ( RQT.getTypePtr()->isPointerType() && !(RQT.getTypePtr()->getPointeeType().isConstQualified()) ) {
//			llvm::errs()<<"\nReturn Expression\n";
//			RS->dump();
//			llvm::errs()<<"\n";
			ReportCallReturn(RS);
			return;
			
		}
	}
}

void WalkAST::VisitDeclRefExpr( clang::DeclRefExpr * DRE) {
	CmsException m_exception;
  	clang::LangOptions LangOpts;
  	LangOpts.CPlusPlus = true;
  	clang::PrintingPolicy Policy(LangOpts);

  	clang::SourceLocation SL = DRE->getLocStart();
  	if (BR.getSourceManager().isInSystemHeader(SL) || BR.getSourceManager().isInExternCSystemHeader(SL)) return;
  if (clang::VarDecl * D = llvm::dyn_cast<clang::VarDecl>(DRE->getDecl()) ) { 

//	llvm::errs()<<"Declaration Ref Expr\t";
//	dyn_cast<Stmt>(DRE)->dumpPretty(AC->getASTContext());
//	llvm::errs()<<"\n";
//	SListDump(llvm::errs());
//	llvm::errs()<<"\n";
	const clang::Stmt * PS = ParentStmt();

	clang::QualType t =  D->getType();  

  	clang::ento::PathDiagnosticLocation CELoc = clang::ento::PathDiagnosticLocation::createBegin(DRE, BR.getSourceManager(),AC);
	if (!m_exception.reportClass( CELoc, BR ) ) return;

	if ( D->isStaticLocal() && ! support::isConst( t ) )
	{
		std::string buf;
	    	llvm::raw_string_ostream os(buf);
	   	os << "Non-const variable '" << D->getNameAsString() << "' is static local and is accessed in statement \n";
//	    	PS->printPretty(os,0,Policy);
	    	BugType * BT = new BugType("ClassChecker : non-const static local variable","ThreadSafety");
		BugReport * R = new BugReport(*BT,os.str(),CELoc);
		BR.emitReport(R);
		return;
	}

	if ( D->isStaticDataMember() && ! support::isConst( t ) )
	{
	    	std::string buf;
	    	llvm::raw_string_ostream os(buf);
	    	os << "Non-const variable '" << D->getNameAsString() << "' is static member data  and might be thread-unsafe\n";
	    	BugType * BT = new BugType("ClassChecker : non-const static member variable","ThreadSafety");
		BugReport * R = new BugReport(*BT,os.str(),CELoc);
		BR.emitReport(R);
	    return;
	}


	if ( (D->getStorageClass() == clang::SC_Static) &&
			  !D->isStaticDataMember() &&
			  !D->isStaticLocal() &&
			  !support::isConst( t ) )
	{

	    	std::string buf;
	    	llvm::raw_string_ostream os(buf);
	    	os << "Non-const variable '" << D->getNameAsString() << "' is global static and might be thread-unsafe\n";
	    	BugType * BT = new BugType("ClassChecker : non-const global static variable","ThreadSafety");
		BugReport * R = new BugReport(*BT,os.str(),CELoc);
		BR.emitReport(R);
	    return;
	
	}

  }
}

void WalkAST::VisitMemberExpr( clang::MemberExpr *ME) {

  clang::SourceLocation SL = ME->getExprLoc();
  if (BR.getSourceManager().isInSystemHeader(SL) || BR.getSourceManager().isInExternCSystemHeader(SL)) return;
  if (SList.empty()) return;
  if (!ME->isImplicitAccess()) return;
  for (llvm::SmallVectorImpl<const clang::Stmt *>::reverse_iterator I = SList.rbegin(), E = SList.rend(); I != E; I++) {
	if (!(*I)) return;
	if (visitingCallExpr && (*I)==visitingCallExpr) return; 
	if (const clang::UnaryOperator * UO = llvm::dyn_cast<clang::UnaryOperator>(*I)) 
		{ WalkAST::UnaryOperator(UO); return;}
	if (const clang::BinaryOperator * BO = llvm::dyn_cast<clang::BinaryOperator>(*I)) 
		{ WalkAST::BinaryOperator(BO); return;}
	if (const clang::CXXOperatorCallExpr *OCE = llvm::dyn_cast<clang::CXXOperatorCallExpr>(*I)) 
		{ WalkAST::CXXOperatorCallExpr(OCE); return;}
	if (const clang::ReturnStmt * RS = llvm::dyn_cast<clang::ReturnStmt>(*I)) 
		{ WalkAST::ReturnStmt(RS); return;}
	if (const clang::ExplicitCastExpr * CE = llvm::dyn_cast<clang::ExplicitCastExpr>(*I))
		{ WalkAST::ExplicitCastExpr(CE); return;} 
	if (const clang::CallExpr *CE = llvm::dyn_cast<clang::CallExpr>(*I)) 
		{ return;}
	if (const clang::IfStmt * IS = llvm::dyn_cast<clang::IfStmt>(*I)) 
		{ return;}
	if (const clang::ForStmt * FS = llvm::dyn_cast<clang::ForStmt>(*I)) 
		{ return;}
	if (const clang::DeclStmt * DS = llvm::dyn_cast<clang::DeclStmt>(*I)) 
		{ return;}
	if (const clang::SwitchStmt * SS = llvm::dyn_cast<clang::SwitchStmt>(*I)) 
		{ return;}
  }

//  clang::Expr * E = ME->getBase();
//  clang::QualType qual_base = E->getType();
//  clang::ValueDecl * VD = ME->getMemberDecl();
//  const clang::Stmt * PS = (*(SList.rbegin()+1));

  {
	ReportMember(ME);
	llvm::errs()<<"Member Expr '";
	dyn_cast<Stmt>(ME)->dumpPretty(AC->getASTContext());
	llvm::errs()<<"' not handled by Class Checker in statement\n\n";
	SListDump(llvm::errs());
	llvm::errs()<<"\n";
	ParentStmt()->dump();
	llvm::errs()<<"\n";
  }

}




void WalkAST::VisitCXXMemberCallExpr( clang::CXXMemberCallExpr *CE) {

  clang::SourceLocation SL = CE->getLocStart();
  if (BR.getSourceManager().isInSystemHeader(SL) || BR.getSourceManager().isInExternCSystemHeader(SL)) return;

  Enqueue(CE);
  Execute();
  clang::CXXMethodDecl * MD = CE->getMethodDecl();
                                                                                                        
  for(int i=0, j=CE->getNumArgs(); i<j; i++) {
	if ( const clang::Expr *E = llvm::dyn_cast<clang::Expr>(CE->getArg(i)))
		{
		clang::QualType qual_arg = E->getType();
		if (const clang::MemberExpr *ME=llvm::dyn_cast<clang::MemberExpr>(E))		
		if (ME->isImplicitAccess())
			{
			clang::ValueDecl * VD = llvm::dyn_cast<clang::ValueDecl>(ME->getMemberDecl());
			clang::QualType qual_decl = llvm::dyn_cast<clang::ValueDecl>(ME->getMemberDecl())->getType();
			clang::ParmVarDecl *PVD=llvm::dyn_cast<clang::ParmVarDecl>(MD->getParamDecl(i));
			clang::QualType QT = PVD->getOriginalType();
			const clang::Type * T = QT.getTypePtr();
			if (!support::isConst(QT))
			if (T->isReferenceType())
				{
				ReportCallArg(CE,i);
				}
			}
		}
  }

//	llvm::errs()<<"\n--------------------------------------------------------------\n";
//	llvm::errs()<<"\n------CXXMemberCallExpression---------------------------------\n";
//	llvm::errs()<<"\n--------------------------------------------------------------\n";
//	CE->dump();
//	llvm::errs()<<"\n--------------------------------------------------------------\n";
//	return;
}

void WalkAST::ReportMember(const clang::MemberExpr *ME) {
  llvm::SmallString<100> buf;
  llvm::raw_svector_ostream os(buf);
  CmsException m_exception;
  clang::ento::PathDiagnosticLocation CELoc;
  clang::SourceRange R;
  clang::LangOptions LangOpts;
  LangOpts.CPlusPlus = true;
  clang::PrintingPolicy Policy(LangOpts);

  CELoc = clang::ento::PathDiagnosticLocation::createBegin(ME, BR.getSourceManager(),AC);
  R = ME->getSourceRange();

  os << " Member data " << ME->getMemberDecl()->getQualifiedNameAsString();
//  ME->printPretty(os,0,Policy);
  os << " is directly or indirectly modified in const function ";
//  llvm::errs()<<os.str();
//  llvm::errs()<< llvm::dyn_cast<clang::CXXMethodDecl>(AC->getDecl())->getQualifiedNameAsString();
//  if (visitingCallExpr) {
//	llvm::errs() << " in call stack ";
//	WListDump(llvm::errs());
//  }



  if (!m_exception.reportClass( CELoc, BR ) ) return;
  BR.EmitBasicReport(AC->getDecl(),"Class Checker : Member data modified in const function","ThreadSafety",os.str(),CELoc,R);
}

void WalkAST::ReportCast(const clang::ExplicitCastExpr *CE) {
  llvm::SmallString<100> buf;
  llvm::raw_svector_ostream os(buf);

  CmsException m_exception;
  clang::LangOptions LangOpts;
  LangOpts.CPlusPlus = true;
  clang::PrintingPolicy Policy(LangOpts);

 
  os << "Const qualifier of member data object was removed via cast expression ";
  os <<  "in const function \n";
  CE->printPretty(os,0,Policy);
//  llvm::errs() << os.str();
//  llvm::errs() << llvm::dyn_cast<clang::CXXMethodDecl>(AC->getDecl())->getQualifiedNameAsString();
//  if (visitingCallExpr) {
//	llvm::errs() << " in call stack ";
//	WListDump(llvm::errs());
//  }
//  llvm::errs()<<"\n";


  clang::ento::PathDiagnosticLocation CELoc =
    clang::ento::PathDiagnosticLocation::createBegin(CE, BR.getSourceManager(),AC);
  

  if (!m_exception.reportClass( CELoc, BR ) ) return;
  BugType * BT = new BugType("Class Checker : Const cast away from member data in const function","ThreadSafety");
  BugReport * R = new BugReport(*BT,os.str(),CELoc);
  BR.emitReport(R);
	 

}

void WalkAST::ReportCallArg(const clang::CXXMemberCallExpr *CE,const int i) {

  llvm::SmallString<100> buf;
  llvm::raw_svector_ostream os(buf);
  CmsException m_exception;
  clang::LangOptions LangOpts;
  LangOpts.CPlusPlus = true;
  clang::PrintingPolicy Policy(LangOpts);

  clang::CXXMethodDecl * MD = llvm::dyn_cast<clang::CXXMemberCallExpr>(CE)->getMethodDecl();
  const clang::MemberExpr *E = llvm::dyn_cast<clang::MemberExpr>(CE->getArg(i));
  clang::ParmVarDecl *PVD=llvm::dyn_cast<clang::ParmVarDecl>(MD->getParamDecl(i));
  clang::ValueDecl * VD = llvm::dyn_cast<clang::ValueDecl>(E->getMemberDecl());
  os << " Member data " << VD->getQualifiedNameAsString();
  os<< " is passed to a non-const reference parameter";
  os <<" of CXX method " << MD->getQualifiedNameAsString()<<" in const function ";
//  llvm::errs() << os.str();
//  llvm::errs() << llvm::dyn_cast<clang::CXXMethodDecl>(AC->getDecl())->getQualifiedNameAsString();
//  if (visitingCallExpr) {
//	llvm::errs() << " in call stack ";
//	WListDump(llvm::errs());
//  }
//  llvm::errs()<<"\n";


  clang::ento::PathDiagnosticLocation ELoc =
   clang::ento::PathDiagnosticLocation::createBegin(CE, BR.getSourceManager(),AC);
  clang::SourceLocation L = E->getExprLoc();

  if (!m_exception.reportClass( ELoc, BR ) ) return;
  BR.EmitBasicReport(CE->getCalleeDecl(),"Class Checker :  Member data passed to non-const reference","ThreadSafety",os.str(),ELoc,L);

}

void WalkAST::ReportCallReturn(const clang::ReturnStmt * RS) {
  llvm::SmallString<100> buf;
  llvm::raw_svector_ostream os(buf);

  CmsException m_exception;
  clang::LangOptions LangOpts;
  LangOpts.CPlusPlus = true;
  clang::PrintingPolicy Policy(LangOpts);

  os << "Returns a pointer to a non-const member data object ";
  os << "in const function.\n ";
  RS->printPretty(os,0,Policy);
//  llvm::errs() << os.str();
//  llvm::errs() << llvm::dyn_cast<clang::CXXMethodDecl>(AC->getDecl())->getQualifiedNameAsString();
//  if (visitingCallExpr) {
//	llvm::errs() << " in call stack ";
//	WListDump(llvm::errs());
//  }
//  llvm::errs()<<"\n";


  clang::ento::PathDiagnosticLocation CELoc =
    clang::ento::PathDiagnosticLocation::createBegin(RS, BR.getSourceManager(),AC);
  

  if (!m_exception.reportClass( CELoc, BR ) ) return;
  BugType * BT = new BugType("Class Checker : Const function returns pointer to non-const member data object","ThreadSafety");
  BugReport * R = new BugReport(*BT,os.str(),CELoc);
  BR.emitReport(R);
	 
}


void ClassChecker::checkASTDecl(const clang::CXXRecordDecl *RD, clang::ento::AnalysisManager& mgr,
                    clang::ento::BugReporter &BR) const {

	const clang::SourceManager &SM = BR.getSourceManager();
	
  	llvm::SmallString<100> buf;
  	llvm::raw_svector_ostream os(buf);
	os <<"class "<<RD->getQualifiedNameAsString()<<"\n";
	if (!SM.getFileManager().getFile("/tmp/classes.txt") ) {
		llvm::errs()<<"\n\nChecker optional.ClassChecker cannot find /tmp/classes.txt. Run 'scram b checker' with USER_LLVM_CHECKERS='-enable-checker optional.ClassDumperCT -enable-checker optional.ClassDumperFT' to create /tmp/classes.txt.\n\n\n";
		exit(1);
		}
	if ( SM.getFileManager().getBufferForFile(SM.getFileManager().getFile("/tmp/classes.txt"))->getBuffer().str().find(os.str(),0) 
		== std::string::npos ) {return;}
	clang::ento::PathDiagnosticLocation DLoc =clang::ento::PathDiagnosticLocation::createBegin( RD, SM );
	if (  !m_exception.reportClass( DLoc, BR ) ) return;
//	llvm::errs()<<os.str();
//	clangcms::WalkAST walker(BR, mgr.getAnalysisDeclContext(RD));
//	clang::LangOptions LangOpts;
//	LangOpts.CPlusPlus = true;
//	clang::PrintingPolicy Policy(LangOpts);
//	RD->print(llvm::errs(),Policy,0,0);
//	llvm::errs() << s.str(); 
//	llvm::errs()<<"\n\n\n\n";
//	RD->dump();
//	llvm::errs()<<"\n\n\n\n";

 
// Check the constructors.
//			for (clang::CXXRecordDecl::ctor_iterator I = RD->ctor_begin(), E = RD->ctor_end();
//         			I != E; ++I) {
//        			if (clang::Stmt *Body = I->getBody()) {
//				llvm::errs()<<"Visited Constructors for\n";
//				llvm::errs()<<RD->getNameAsString();
//          				walker.Visit(Body);
//          				walker.Execute();
//        				}
//    				}
	


// Check the class methods (member methods).
	for (clang::CXXRecordDecl::method_iterator
		I = RD->method_begin(), E = RD->method_end(); I != E; ++I)  
		{
		clang::ento::PathDiagnosticLocation DLoc =clang::ento::PathDiagnosticLocation::createBegin( (*I) , SM );
		if (  !m_exception.reportClass( DLoc, BR ) ) continue;
		if ( !((*I)->hasBody()) ) continue;      
		if ( !llvm::isa<clang::CXXMethodDecl>((*I)) ) continue;
		if (!(*I)->isConst()) continue;
		if ((*I)->isVirtualAsWritten()) continue;
		clang::CXXMethodDecl * MD = llvm::cast<clang::CXXMethodDecl>((*I)->getMostRecentDecl());
//        	llvm::errs() << "\n\nmethod "<<MD->getQualifiedNameAsString()<<"\n\n";
//		for (clang::CXXMethodDecl::method_iterator J = MD->begin_overridden_methods(), F = MD->end_overridden_methods(); J != F; ++J) {
//			llvm::errs()<<"\n\n overwritten method "<<(*J)->getQualifiedNameAsString()<<"\n\n";
//			}


//				llvm::errs()<<"\n*****************************************************\n";
//				llvm::errs()<<"\nVisited CXXMethodDecl\n";
//				llvm::errs()<<RD->getNameAsString();
//				llvm::errs()<<"::";
//				llvm::errs()<<MD->getNameAsString();
//				llvm::errs()<<"\n*****************************************************\n";
				if ( MD->hasBody() ) {
					clang::Stmt *Body = MD->getBody();
//					clang::LangOptions LangOpts;
//					LangOpts.CPlusPlus = true;
//					clang::PrintingPolicy Policy(LangOpts);
//					std::string TypeS;
//	       				llvm::raw_string_ostream s(TypeS);
//	       				llvm::errs() << "\n\n+++++++++++++++++++++++++++++++++++++\n\n";
//	      				llvm::errs() << "\n\nPretty Print\n\n";
//	       				Body->printPretty(s, 0, Policy);
//        				llvm::errs() << s.str();
//					Body->dumpAll();
//	       				llvm::errs() << "\n\n+++++++++++++++++++++++++++++++++++++\n\n";
					clangcms::WalkAST walker(BR, mgr.getAnalysisDeclContext(MD));
	       				walker.Visit(Body);
					clang::QualType RQT = MD->getCallResultType();
					clang::QualType CQT = MD->getResultType();
					if (RQT.getTypePtr()->isPointerType() && !RQT.getTypePtr()->getPointeeType().isConstQualified() 
							&& MD->getName().lower().find("clone")==std::string::npos )  {
						llvm::SmallString<100> buf;
						llvm::raw_svector_ostream os(buf);
						os << MD->getQualifiedNameAsString() << " is a const member function that returns a pointer to a non-const object";
						os << "\n";
						clang::ento::PathDiagnosticLocation ELoc =clang::ento::PathDiagnosticLocation::createBegin( MD , SM );
						clang::SourceRange SR = MD->getSourceRange();
						BR.EmitBasicReport(MD, "Class Checker : Const function returns pointer to non-const object.","ThreadSafety",os.str(),ELoc);
					}
				}
   	}	/* end of methods loop */


} //end of class



void ClassDumper::checkASTDecl(const clang::CXXRecordDecl *RD,clang::ento::AnalysisManager& mgr,
                    clang::ento::BugReporter &BR ) const {

	const clang::SourceManager &SM = BR.getSourceManager();
	clang::ento::PathDiagnosticLocation DLoc =clang::ento::PathDiagnosticLocation::createBegin( RD, SM );
//	if (  !m_exception.reportClass( DLoc, BR ) ) return;
//Dump the template name and args
	if (const ClassTemplateSpecializationDecl *SD = dyn_cast<ClassTemplateSpecializationDecl>(RD))
		{
			for (unsigned J = 0, F = SD->getTemplateArgs().size(); J!=F; ++J)
			{
//			llvm::errs()<<"\nTemplate "<<SD->getSpecializedTemplate()->getQualifiedNameAsString()<<";";
//			llvm::errs()<<"Template Argument ";
//			llvm::errs()<<SD->getTemplateArgs().get(J).getAsType().getAsString();
//			llvm::errs()<<"\n\n\t";
			if (SD->getTemplateArgs().get(J).getKind() == clang::TemplateArgument::Type && SD->getTemplateArgs().get(J).getAsType().getTypePtr()->isRecordType() )
				{
				const clang::CXXRecordDecl * D = SD->getTemplateArgs().get(J).getAsType().getTypePtr()->getAsCXXRecordDecl();
				checkASTDecl( D, mgr, BR );
				}
			}

		}
	
// Dump the class members.
	std::string err;
	std::string fname("/tmp/classes.txt");
	llvm::raw_fd_ostream output(fname.c_str(),err,llvm::raw_fd_ostream::F_Append);
	llvm::errs() <<"class " <<RD->getQualifiedNameAsString()<<"\n";
	output <<"class " <<RD->getQualifiedNameAsString()<<"\n";
	for (clang::RecordDecl::field_iterator I = RD->field_begin(), E = RD->field_end(); I != E; ++I)
	{
		clang::QualType qual;
		if (I->getType().getTypePtr()->isAnyPointerType()) 
			qual = I->getType().getTypePtr()->getPointeeType();
		else 
			qual = I->getType().getNonReferenceType();

		if (!qual.getTypePtr()->isRecordType()) return;
//		llvm::errs() <<"Class Member ";
//		if (I->getType() == qual)
//			{
//			llvm::errs() <<"; "<<I->getType().getCanonicalType().getTypePtr()->getTypeClassName();
//			}
//		else
//			{
//			llvm::errs() <<"; "<<qual.getCanonicalType().getTypePtr()->getTypeClassName()<<" "<<I->getType().getCanonicalType().getTypePtr()->getTypeClassName();
//			}
//		llvm::errs() <<"; "<<I->getType().getCanonicalType().getAsString();
//		llvm::errs() <<"; "<<I->getType().getAsString();
//		llvm::errs() <<"; "<< I->getQualifiedNameAsString();

//		llvm::errs() <<"\n\n";
		if (const CXXRecordDecl * TRD = I->getType().getTypePtr()->getAsCXXRecordDecl()) 
			{
			if (RD->getNameAsString() == TRD->getNameAsString())
				{
				checkASTDecl( TRD, mgr, BR );
				}
			}
	}

} //end class


void ClassDumperCT::checkASTDecl(const clang::ClassTemplateDecl *TD,clang::ento::AnalysisManager& mgr,
                    clang::ento::BugReporter &BR ) const {
	const clang::SourceManager &SM = BR.getSourceManager();
	clang::ento::PathDiagnosticLocation DLoc =clang::ento::PathDiagnosticLocation::createBegin( TD, SM );
	if ( SM.isInSystemHeader(DLoc.asLocation()) || SM.isInExternCSystemHeader(DLoc.asLocation()) ) return;
	if (TD->getTemplatedDecl()->getQualifiedNameAsString() == "edm::Wrapper" ) 
		{
//		llvm::errs()<<"\n";
		for (ClassTemplateDecl::spec_iterator I = const_cast<clang::ClassTemplateDecl *>(TD)->spec_begin(), 
			E = const_cast<clang::ClassTemplateDecl *>(TD)->spec_end(); I != E; ++I) 
			{
			for (unsigned J = 0, F = I->getTemplateArgs().size(); J!=F; ++J)
				{
//				llvm::errs()<<"template class "<< TD->getTemplatedDecl()->getQualifiedNameAsString()<<"<" ;
//				llvm::errs()<<I->getTemplateArgs().get(J).getAsType().getAsString();
//				llvm::errs()<<">\n";
				if (const clang::CXXRecordDecl * D = I->getTemplateArgs().get(J).getAsType().getTypePtr()->getAsCXXRecordDecl())
					{
					ClassDumper dumper;
					dumper.checkASTDecl( D, mgr, BR );
					}
				}
			} 		
		};
} //end class

void ClassDumperFT::checkASTDecl(const clang::FunctionTemplateDecl *TD,clang::ento::AnalysisManager& mgr,
                    clang::ento::BugReporter &BR ) const {
	const clang::SourceManager &SM = BR.getSourceManager();
	clang::ento::PathDiagnosticLocation DLoc =clang::ento::PathDiagnosticLocation::createBegin( TD, SM );
	if ( SM.isInSystemHeader(DLoc.asLocation()) || SM.isInExternCSystemHeader(DLoc.asLocation()) ) return;
	if (TD->getTemplatedDecl()->getQualifiedNameAsString().find("typelookup") != std::string::npos ) 
		{
//		llvm::errs()<<"\n";
		for (FunctionTemplateDecl::spec_iterator I = const_cast<clang::FunctionTemplateDecl *>(TD)->spec_begin(), 
				E = const_cast<clang::FunctionTemplateDecl *>(TD)->spec_end(); I != E; ++I) 
			{
			for (unsigned J = 0, F = (*I)->getTemplateSpecializationArgs()->size(); J!=F;++J)
				{
//				llvm::errs()<<"template function " << TD->getTemplatedDecl()->getQualifiedNameAsString()<<"<";
//				llvm::errs()<<(*I)->getTemplateSpecializationArgs()->get(J).getAsType().getAsString();
//				llvm::errs()<<">\n";
				if (const clang::CXXRecordDecl * D = (*I)->getTemplateSpecializationArgs()->get(J).getAsType().getTypePtr()->getAsCXXRecordDecl()) 
					{
					ClassDumper dumper;
					dumper.checkASTDecl( D, mgr, BR );
					}
				}
	
			} 		
		};
} //end class


}//end namespace


