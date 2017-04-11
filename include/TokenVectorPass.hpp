//
// Created by Mingkai Chen on 2017-04-10.
//

#ifndef DEFECTOR_TOKENVECTORPASS_HPP
#define DEFECTOR_TOKENVECTORPASS_HPP


#include "llvm/Analysis/LoopInfo.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

#include <unordered_set>
#include <unordered_map>


using BUG_REPO = std::unordered_map<std::string, std::unordered_set<uint64_t> >;


namespace tokenprofiling
{


struct TokenVectorPass : public llvm::ModulePass
{
	static char ID;

	BUG_REPO bugs_;

	TokenVectorPass() : llvm::ModulePass(ID) {}

	TokenVectorPass(BUG_REPO& bugs) :
		llvm::ModulePass(ID), bugs_(bugs) {}

	void getAnalysisUsage(llvm::AnalysisUsage& au) const override
	{
		au.addRequired<llvm::DominatorTreeWrapperPass>();
		au.addRequired<llvm::LoopInfoWrapperPass>();
	}

	bool runOnModule(llvm::Module& m) override;
};


} // namespace tokenprofiling

#endif //DEFECTOR_TOKENVECTORPASS_HPP
