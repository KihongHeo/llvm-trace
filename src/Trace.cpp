#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <vector>

using namespace llvm;

namespace {
struct TracePass : public ModulePass {
  static char ID;
  TracePass() : ModulePass(ID) {}
  StructType *IO_FILE_ty;
  Type *IO_FILE_PTR_ty;
  Value *FPrintf;
  Value *pFile;
  virtual Function *getfprint(Module &M, std::vector<Value *> args) {
    std::vector<Type *> argsTypes;
    for (unsigned i = 0, e = args.size(); i != e; i++) {
      args[i]->getType()->print(errs());
      argsTypes.push_back(args[i]->getType());
      // argsTypes.push_back(Type::getInt8PtrTy(M.getContext()));
    }

    // create fprintf function
    FPrintf = M.getOrInsertFunction(
        "myprintf",
        FunctionType::get(Type::getVoidTy(M.getContext()), argsTypes, false));

    if (Function *func_fprintf = dyn_cast<Function>(FPrintf)) {
      return func_fprintf;
    } else {
      llvm::errs() << "WARN: invalid fprintf\n";
      return NULL;
    }

    return NULL;
  }

  std::string Str = "hello world";
  virtual bool runOnBasicBlock(BasicBlock &BB, Module &M) {
    //    BB.print(llvm::errs(), true);
    for (auto &I : BB) {
      I.print(errs());
      errs() << " << \n";
      std::vector<Value *> Args;
      IRBuilder<> builder(&BB);
      std::string Code;
      llvm::raw_string_ostream SS(Code);
      I.print(SS);
      errs() << SS.str() << "\n";
      Value *v = builder.CreateGlobalStringPtr(Code);
      Args.push_back(v);
      Function *funcFprintf = getfprint(M, Args);
      if (!funcFprintf)
        return false;
      CallInst *fprintfCall = CallInst::Create(funcFprintf, Args, "", &I);
      fprintfCall->setCallingConv(CallingConv::C);
      fprintfCall->setTailCall(true);
    }
    return true;
  }

  virtual bool runOnModule(Module &M) {
    errs() << "Module: " << M.size() << "\n";
    for (auto &F : M) {
      for (auto &BB : F) {
        errs() << "In a function called " << F.getName() << "!\n";
        errs() << "Function body:\n";
        F.print(llvm::errs());

        runOnBasicBlock(BB, M);
      }
    }
    return false;
  };
};
} // namespace

char TracePass::ID = 0;
/*  static void registerSkeletonPass(const PassManagerBuilder &,
                                   legacy::PassManagerBase &PM) {
    PM.add(new TracePass());
  }

  static RegisterStandardPasses
      RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                     registerSkeletonPass);
  */
static RegisterPass<TracePass> X("TracePass", "MyPassPrintf Pass", false,
                                 false);
