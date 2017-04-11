//
// Created by Mingkai Chen on 2017-04-10.
//


#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/CallSite.h"
#include "llvm/Pass.h"

#include <experimental/optional>
#include <iostream>

#include "TokenVectorPass.hpp"


using namespace std::experimental;
using namespace llvm;
using namespace tokenprofiling;


namespace tokenprofiling
{
	char TokenVectorPass::ID = 0;
}


static optional<unsigned> getLineNumber(Module& m, Instruction& inst)
{
	const DebugLoc& loc = inst.getDebugLoc();
	optional<unsigned> line;
	if (loc) {
		line = loc.getLine(); // line numbers start from 1
	}
	return line;
}


bool TokenVectorPass::runOnModule(llvm::Module &module)
{
	std::unordered_set<std::string> structs;
	for (auto* s : module.getIdentifiedStructTypes())
	{
		StringRef sname = s->getName();
		structs.emplace(sname);
	}

	std::vector<std::string> labels;
	std::string modname = module.getSourceFileName();
	std::unordered_set<uint64_t>& err_lines = bugs_[modname];
	std::unordered_map<std::string, std::unordered_set<std::string> > immediate_dependents;

	for (auto& f : module)
	{
		if (f.isDeclaration())
		{
			// mark declaration as a token
			std::string declname = f.getName().str();
			if (0 != declname.compare(0, 9, "llvm.dbg."))
			{
				labels.push_back(declname);
			}
			continue;
		}
		LoopInfo& LI = getAnalysis<LoopInfoWrapperPass>(f).getLoopInfo();
		std::string funcName = f.getName().str();
		std::unordered_set<BasicBlock*> potential_else;
		for (auto& bb : f)
		{
			if (potential_else.end() != potential_else.find(&bb))
			{
				potential_else.erase(&bb);
				labels.push_back("else");
			}
			for (auto& i : bb)
			{
				// check for method calls, control flow statements, allocations
				CallSite cs(&i);
				if (auto* instr = cs.getInstruction())
				{
					if (auto directCall = dyn_cast<llvm::Function>(cs.getCalledValue()->stripPointerCasts()))
					{
						std::string callname = directCall->getName().str();
						if (0 == callname.find("llvm.") ||
							0 == callname.find("__")) continue;
						labels.push_back(callname);
						// this function has a dependency on callee function, record dependence
						immediate_dependents[callname].emplace(funcName);
					}
					else
					{
						// this function has a dependency on callee function, but can't record dependence
						std::string instname = instr->getName().str();
						labels.push_back(instname);
					}
				}
				else if (BranchInst* bi = llvm::dyn_cast<BranchInst>(&i))
				{
					if (bi->isConditional()) // loop or if
					{
						bool loops = false;
						Loop* ourloop = LI.getLoopFor(&bb);
						for (auto succs : successors(&bb))
						{
							Loop* theirloop = LI.getLoopFor(succs);
							if (theirloop != ourloop)
							{
								loops = true;
								break;
							}
						}
						if (loops)
						{
							labels.push_back("while");
						}
						else
						{
							labels.push_back("if");
							if (bi->getNumSuccessors() > 1)
							{
								potential_else.emplace(bi->getSuccessor(1));
							}
						}
					}
					else
					{
						for (size_t i = 0; i < bi->getNumSuccessors(); i++)
						{
							auto it = potential_else.find(bi->getSuccessor(i));
							if (potential_else.end() != it)
							{
								potential_else.erase(it);
							}
						}
					}
				}
				else if (InvokeInst* ii = llvm::dyn_cast<InvokeInst>(&i))
				{
					std::string invname = ii->getName().str();
					if (invname.find("throw") != std::string::npos)
					{
						labels.push_back("throw");
					}
				}
				else if (llvm::dyn_cast<LandingPadInst>(&i))
				{
					labels.push_back("catch");
				}
				else if (llvm::dyn_cast<SwitchInst>(&i))
				{
					labels.push_back("switch");
				}
				else if (llvm::dyn_cast<UnreachableInst>(&i))
				{
					labels.push_back("unreachable");
				}
				else if (llvm::dyn_cast<AllocaInst>(&i) || llvm::dyn_cast<LoadInst>(&i))
				{
					std::string typestr;
					switch(i.getType()->getTypeID())
					{
						case llvm::Type::TypeID::VoidTyID:
							break;
						case llvm::Type::TypeID::HalfTyID:
							typestr = "16bit";
							break;
						case llvm::Type::TypeID::FloatTyID:
							typestr = "32bit";
							break;
						case llvm::Type::TypeID::DoubleTyID:
							typestr = "64bit";
							break;
						case llvm::Type::TypeID::X86_FP80TyID:
							typestr = "80bit";
							break;
						case llvm::Type::TypeID::FP128TyID:
						case llvm::Type::TypeID::PPC_FP128TyID:
							typestr = "128bit";
							break;
						case llvm::Type::TypeID::IntegerTyID:
							typestr = "integer";
							break;
						case llvm::Type::TypeID::FunctionTyID:
							typestr = "function";
							break;
						case llvm::Type::TypeID::StructTyID:
							typestr = "struct";
							break;
						case llvm::Type::TypeID::ArrayTyID:
							typestr = "array";
							break;
						case llvm::Type::TypeID::PointerTyID:
							typestr = "pointer";
							break;
						case llvm::Type::TypeID::VectorTyID:
							typestr = "vector";
							break;
						case llvm::Type::TypeID::LabelTyID:
						case llvm::Type::TypeID::MetadataTyID:
						case llvm::Type::TypeID::X86_MMXTyID:
						case llvm::Type::TypeID::TokenTyID:
							break;
					}
					if (!typestr.empty())
					{
						labels.push_back(typestr);
					}
				}

				optional<unsigned> lineno = getLineNumber(module, i);
				if ((bool) lineno)
				{
					// check for bug
					auto it = err_lines.find(lineno.value());
					if (err_lines.end() != it)
					{
						// paint entire function as buggy
					}
				}
			}
		}
	}

	for (std::string s : labels)
	{
		std::cout << s << ", ";
	}
	std::cout << std::endl;

	return true;
}
