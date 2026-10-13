// Alias Analysis - Andersen's Pointer Analysis
// Author	: Shubham Jadhav (CS24S009)
// Date		: 9 Sep, 2025

#include <map>
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/CFG.h"

using namespace llvm;

using ValueSet = SmallPtrSet<const Value *, 8>;

enum ConstraintType
{
	COPY,
	LOAD,
	STORE,
	DEREFERENCE,
};

struct PointsToConstraint
{
	const StoreInst *SI;
	ConstraintType type;
	PointsToConstraint(const StoreInst *_SI, ConstraintType _type) : SI(_SI), type(_type) {}
};

class PointerAnalysis
{

	const Function &Fn;

public:
	PointerAnalysis(const Function &F) : Fn(F)
	{
		// READ THIS
		// You are free to use any data structures or logic to implement Andersen’s Analysis.
		// In LLVM IR, each allocated variable has an SSA value (e.g., %2, %3) represented by a Value.
		// The address of this Value, Value* is what we use as the key in the pointsTo map.
		// Example: if "a" corresponds to %2 at 0x123 (Value*) and "b" to %3 at 0x456,
		// and if "a" points to "b", then pointsTo[0x123] should contain 0x456.

		llvm::DenseMap<const Value *, SmallPtrSet<const Value *, 8>> pointsTo; /* You MUST use this Map*/

		// You don’t need to handle conversion from addresses back to variable names.
		// The code below will take care of that automatically as long as you correctly
		// populate the pointsTo map.

		// Write your code here (or anywhere you prefer), just make sure to
		// populate the pointsTo map with the correct values.

		//===================================================================================================================
		
		// Start implementation

		// all constraints i.e. 4 types
		struct Constraint {
		    enum Type { ADDR_OF,
						COPY,
						LOAD, 
						STORE };
		    Type type;
		    const Value *lhs;
		    const Value *rhs;
		};

		std::vector<Constraint> constraints;

		//map for tracking values cominng from load instructions
		DenseMap<const Value*, const Value*> valueToSource;

		// Step 1
		// mapping of load results to their sources
		for (auto &BB : Fn) {
		    for (auto &I : BB) {

				if (auto *LI = dyn_cast<LoadInst>(&I)) {

					if (LI->getType()->isPointerTy()) {
						valueToSource[LI] = LI->getPointerOperand();
					}
				}
		    }
		}

		
		// Step 2
		//extract contraints from store instructions
		for (auto &BB : Fn) {
		    for (auto &I : BB) {

				if (auto *SI = dyn_cast<StoreInst>(&I)) {
					
					const Value *val = SI->getValueOperand();
					const Value *ptr = SI->getPointerOperand();

					// only handling pointer types
					if (!val->getType()->isPointerTy()) 
						continue;

					// checkng storing through a loaded pointer ( *p = val)
					auto ptrSourceIt = valueToSource.find(ptr);
					if (ptrSourceIt != valueToSource.end()) {

						// *p = val ---------> store through pointer
						const Value *ptrSource = ptrSourceIt->second;

						auto valSourceIt = valueToSource.find(val);
						if (valSourceIt != valueToSource.end()) {
							
							constraints.push_back({Constraint::STORE, ptrSource, valSourceIt->second});		// *p = q --------> where q is loaded
						} else if (isa<AllocaInst>(val) || isa<GlobalVariable>(val)) {
							
							constraints.push_back({Constraint::STORE, ptrSource, val});		// *p = &x
						} else {
							
							constraints.push_back({Constraint::STORE, ptrSource, val});		// *p = q ----------> where q is direct
						}

					} else {
					
						// p = val -----------> direct assignment
						if (isa<AllocaInst>(val) || isa<GlobalVariable>(val)) {
							
							constraints.push_back({Constraint::ADDR_OF, ptr, val});			// p = &x ---------> address-of

						} else {

							auto valSourceIt = valueToSource.find(val);
							if (valSourceIt != valueToSource.end()) {

								// p = loaded_value
								const Value *valSource = valSourceIt->second;

								// is this a double dereference (p = *q) ?
								auto valSourceSourceIt = valueToSource.find(valSource);
								if (valSourceSourceIt != valueToSource.end()) {
									constraints.push_back({Constraint::LOAD, ptr, valSourceSourceIt->second});		// p = **q -----> but assignment says no double deref, so treat as load
								} else {
									constraints.push_back({Constraint::COPY, ptr, valSource});	// p = q (copy)
								}

							} else {
								
								constraints.push_back({Constraint::COPY, ptr, val});			// p = q (direct copy)
							}
						}
					}
				}
		    }
		}
		

		/*
		// Debug Information : Printing all extracted constraints
		llvm::errs() << "=== CONSTRAINTS ===\n";
		for (size_t i = 0; i < constraints.size(); i++) {
		    const auto &c = constraints[i];
		    llvm::errs() << i << ": ";
		    switch (c.type) {
				case Constraint::ADDR_OF:	llvm::errs() << "ADDR_OF";
										  	break;
				case Constraint::COPY: 		llvm::errs() << "COPY";
											break;
				case Constraint::LOAD: 		llvm::errs() << "LOAD";
											break;
				case Constraint::STORE: 	llvm::errs() << "STORE";
											break;
		    }
		    llvm::errs() << " " << c.lhs << " <- " << c.rhs << "\n";
		}
		llvm::errs() << "===================\n";
		*/


		// Step 3
		// fixed-point iteration
		bool changed = true;
		int iterations = 0;
		const int MAX_ITERATIONS = 100;

		while (changed && iterations < MAX_ITERATIONS) {

		    changed = false;
		    iterations++;
		    
		    for (const auto &c : constraints) {

				switch (c.type) {

					case Constraint::ADDR_OF: 	{
												if (pointsTo[c.lhs].insert(c.rhs).second) {			// lhs = &rhs  ---->  pts(lhs) ⊇ {rhs}
													changed = true;
												}
												break;
												}
					
					case Constraint::COPY: {
                                            pointsTo[c.lhs];
											for (const Value *v : pointsTo[c.rhs]) {				// lhs = rhs  ----->  pts(lhs) ⊇ pts(rhs)
												if (pointsTo[c.lhs].insert(v).second) {
													changed = true;
												}
											}
											break;
											}
					
					case Constraint::LOAD: {
                                            pointsTo[c.lhs];
											for (const Value *p : pointsTo[c.rhs]) {				// lhs = *rhs  ------->  for all x ∈ pts(rhs), pts(lhs) ⊇ pts(x)
												for (const Value *v : pointsTo[p]) {
													if (pointsTo[c.lhs].insert(v).second) {
														changed = true;
													}
												}
											}
											break;
											}
					
					case Constraint::STORE: {
											for (const Value *p : pointsTo[c.lhs]) {				// *lhs = rhs  ------->  for all x ∈ pts(lhs), pts(x) ⊇ pts(rhs)
                                                if ((isa<AllocaInst>(c.rhs) && !cast<AllocaInst>(c.rhs)->getAllocatedType()->isPointerTy()) || (isa<GlobalVariable>(c.rhs) && !cast<GlobalVariable>(c.rhs)->getValueType()->isPointerTy())) {
								                    if (pointsTo[p].insert(c.rhs).second) changed = true;
							                    } else {
                                                    for (const Value *v : pointsTo[c.rhs]) {
                                                        if (pointsTo[p].insert(v).second) {
                                                            changed = true;
                                                        }
                                                    }
                                                }
											}
											break;
											}
				}
		    }
		}

		// Implementation End
		// The rest of the code handles querying and printing results (dont change it)

		//===================================================================================================================


		// Do not modify the code below.
		std::map<StringRef, Value *> programVariables;
		const Module *m = Fn.getParent();

		for (const GlobalVariable &gv : m->globals())
		{
			if (gv.hasName())
			{
				programVariables[gv.getName()] = const_cast<llvm::GlobalVariable *>(&gv);
			}
		}

		for (auto &BB : Fn)
		{
			for (auto &I : BB)
			{
				// Get all the memory obejects.
				for (DbgVariableRecord &DVR : filterDbgVars(I.getDbgRecordRange()))
					programVariables[DVR.getVariable()->getName()] = DVR.getAddress();
			}
		}

		StringRef varA = "a";
		StringRef varB = "b";

		const Value *valA = programVariables[varA];
		const Value *valB = programVariables[varB];

		auto itA = pointsTo.find(valA);
		auto itB = pointsTo.find(valB);

		if (itA != pointsTo.end() && itB != pointsTo.end())
		{
			const ValueSet &setA = itA->second;
			const ValueSet &setB = itB->second;

			// Collect intersection
			std::vector<std::string> names;
			for (const Value *v : setA)
			{
				if (setB.count(v))
				{
					if (v->hasName())
					{
						names.push_back(v->getName().str());
					}
					else
					{
						for (const auto &entry : programVariables)
						{
							if (entry.second == v)
							{
								names.push_back(entry.first.str());
								break;
							}
						}
					}
				}
			}

			std::sort(names.begin(), names.end());

			// Print
			llvm::errs() << "{ ";
			if (names.empty())
			{
				llvm::errs() << "}";
			}
			else
			{
				for (size_t i = 0; i < names.size(); i++)
				{
					llvm::errs() << names[i];
					if (i + 1 < names.size())
						llvm::errs() << " ";
				}
				llvm::errs() << " }";
			}
			llvm::errs() << "\n";
		}
		else
		{
			llvm::errs() << "Did you forget to actually implement the pass?\n";
		}
	}
};

// Pointer Analysis
void analyseFunction(Module &M)
{

	std::map<StringRef, Value *> programVariables;
	for (Function &F : M)
	{
		PointerAnalysis andersen(F);
	}
}

// Registering the Function Pass (Don't change any code below)
class PointerAnalysisPass : public PassInfoMixin<PointerAnalysisPass>
{
public:
	static bool isRequired() { return true; }

	PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM)
	{
		analyseFunction(M);
		return PreservedAnalyses::all();
	};
};

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo()
{
	return {
		.APIVersion = LLVM_PLUGIN_API_VERSION,
		.PluginName = "Pointer Analysis Pass",
		.PluginVersion = "v0.1",
		.RegisterPassBuilderCallbacks = [](PassBuilder &PB)
		{
			PB.registerPipelineParsingCallback(
				[](StringRef Name, ModulePassManager &MPM, ArrayRef<PassBuilder::PipelineElement>)
				{
					if (Name == "pointer-analysis")
					{
						MPM.addPass(PointerAnalysisPass());
						return true;
					}
					return false;
				});
		}};
}
