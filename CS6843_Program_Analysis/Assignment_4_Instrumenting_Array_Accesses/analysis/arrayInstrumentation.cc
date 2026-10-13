// Assignment 4 : Instrumenting Array Accesses
// Author	: Shubham Jadhav
// Date		: 6 Oct, 2025

#include <map>
#include <set>
#include <limits>
#include <vector>
#include <algorithm>
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/Dominators.h"

using namespace llvm;

namespace {

struct Range {
    int low, high;

    Range() : low(std::numeric_limits<int>::min()), high(std::numeric_limits<int>::max()) {}
    Range(int l, int h) : low(l), high(h) {}

    bool isEmpty() const { return low > high; }

    Range meet(const Range& other) const {
       

	// feasibility-aware meet
	if (isEmpty()) 
		return other;
        if (other.isEmpty()) 
		return *this;

    	/*	    
	if (isEmpty())
		return *this;
	if (other.isEmpty())
		return other;
        */

	return Range(std::min(low, other.low), std::max(high, other.high));
    }

    Range intersect(const Range& other) const {
        return Range(std::max(low, other.low), std::min(high, other.high));
    }

    Range add(const Range& other) const {
        
	if (isEmpty() || other.isEmpty()) 
		return Range(1, 0);
        int64_t l = (int64_t)low + other.low;
        int64_t h = (int64_t)high + other.high;
        if (l < std::numeric_limits<int>::min()) l = std::numeric_limits<int>::min();
        if (h > std::numeric_limits<int>::max()) h = std::numeric_limits<int>::max();
        
	return Range(l, h);
    }

    Range sub(const Range& other) const {
        
	if (isEmpty() || other.isEmpty()) 
		return Range(1, 0);
        int64_t l = (int64_t)low - other.high;
        int64_t h = (int64_t)high - other.low;
        if (l < std::numeric_limits<int>::min()) l = std::numeric_limits<int>::min();
        if (h > std::numeric_limits<int>::max()) h = std::numeric_limits<int>::max();
        
	return Range(l, h);
    }

    Range mul(const Range& other) const {
        
	if (isEmpty() || other.isEmpty()) 
		return Range(1, 0);
        int64_t ll = (int64_t)low * other.low;
        int64_t lh = (int64_t)low * other.high;
        int64_t hl = (int64_t)high * other.low;
        int64_t hh = (int64_t)high * other.high;
        int64_t min_val = std::min({ll, lh, hl, hh});
        int64_t max_val = std::max({ll, lh, hl, hh});
        if (min_val < std::numeric_limits<int>::min()) min_val = std::numeric_limits<int>::min();
        if (max_val > std::numeric_limits<int>::max()) max_val = std::numeric_limits<int>::max();
        
	return Range(min_val, max_val);
    }
};

class RangeAnalysis {
private:
    std::map<std::pair<BasicBlock*, Value*>, Range> blockEntryRanges;
    std::map<std::pair<BasicBlock*, std::pair<Value*, int>>, Range> blockEntryArrayElements;
    std::set<BasicBlock*> unreachableBlocks;
    
    bool isInfiniteLoop(BasicBlock *BB) {
        auto *br = dyn_cast<BranchInst>(BB->getTerminator());
        if (!br || !br->isConditional()) 
		return false;
        
        // checking if BB loops back to itself or a successor loops back ??
        for (auto *succ : successors(BB)) {
            if (succ == BB || isPotentiallyReachable(succ, BB)) {
                // its a loop - check if condition variable is modified in loop
                if (auto *cmp = dyn_cast<ICmpInst>(br->getCondition())) {
                    Value *op0 = cmp->getOperand(0);
                    if (auto *load = dyn_cast<LoadInst>(op0)) {
                        Value *ptr = load->getPointerOperand();
                        
                        // check if ptr is modified in the loop body
                        std::set<BasicBlock*> loopBlocks;
                        std::vector<BasicBlock*> worklist = {BB};
                        loopBlocks.insert(BB);
                        
                        while (!worklist.empty()) {
                            BasicBlock *cur = worklist.back();
                            worklist.pop_back();
                            
                            for (auto *s : successors(cur)) {
                                if (s == BB) continue; 
                                if (loopBlocks.insert(s).second) {
                                    worklist.push_back(s);
                                }
                            }
                        }
                        
                        // check if ptr is stored to in loop
                        for (auto *lb : loopBlocks) {
                            for (auto &I : *lb) {
                                if (auto *store = dyn_cast<StoreInst>(&I)) {
                                    if (store->getPointerOperand() == ptr) {
                                        return false; // modified -> not infinite
                                    }
                                }
                            }
                        }
                        return true; // never modified -> infinite loop
                    }
                }
            }
        }
        return false;
    }

    Range getRange(Value *V, std::map<Value*, Range> &currentRanges, std::map<std::pair<Value*, int>, Range> &currentArrayElements) {
        
	if (auto *c = dyn_cast<ConstantInt>(V)) {
            return Range(c->getSExtValue(), c->getSExtValue());
        }
        if (auto *sext = dyn_cast<SExtInst>(V)) {
            return getRange(sext->getOperand(0), currentRanges, currentArrayElements);
        }
        
	return currentRanges.count(V) ? currentRanges[V] : Range();
    }

    void processInstruction(Instruction *I, std::map<Value*, Range> &currentRanges, std::map<std::pair<Value*, int>, Range> &currentArrayElements) {
        
	if (auto *store = dyn_cast<StoreInst>(I)) {		// stroe
            Value *val = store->getValueOperand();
            Value *ptr = store->getPointerOperand();
            Range r = getRange(val, currentRanges, currentArrayElements);
            
            if (auto *gep = dyn_cast<GetElementPtrInst>(ptr)) {
                Value *arrayPtr = gep->getPointerOperand();
                if (gep->getNumOperands() > 2) {
                    Value *indexVal = gep->getOperand(gep->getNumOperands() - 1);
                    if (auto *sext = dyn_cast<SExtInst>(indexVal)) indexVal = sext->getOperand(0);
                    
                    if (auto *ci = dyn_cast<ConstantInt>(indexVal)) {
                        currentArrayElements[{arrayPtr, ci->getSExtValue()}] = r;
                    } else {
                        Range idxRange = getRange(indexVal, currentRanges, currentArrayElements);
                        if (idxRange.low == idxRange.high) {
                            currentArrayElements[{arrayPtr, idxRange.low}] = r;
                        }
                    }
                }
            } else {
                currentRanges[ptr] = r;
            }
        
	} else if (auto *load = dyn_cast<LoadInst>(I)) {		// load
            Value *ptr = load->getPointerOperand();
            
            if (auto *gep = dyn_cast<GetElementPtrInst>(ptr)) {
                Value *arrayPtr = gep->getPointerOperand();
                if (gep->getNumOperands() > 2) {
                    Value *indexVal = gep->getOperand(gep->getNumOperands() - 1);
                    if (auto *sext = dyn_cast<SExtInst>(indexVal)) indexVal = sext->getOperand(0);
                    
                    Range loadedRange(0, 0);
                    if (auto *ci = dyn_cast<ConstantInt>(indexVal)) {
                        int idx = ci->getSExtValue();
                        loadedRange = currentArrayElements.count({arrayPtr, idx}) ? 
                                     currentArrayElements[{arrayPtr, idx}] : Range(0, 0);
                    } else {
                        Range idxRange = getRange(indexVal, currentRanges, currentArrayElements);
                        if (idxRange.low == idxRange.high && 
                            currentArrayElements.count({arrayPtr, idxRange.low})) {
                            loadedRange = currentArrayElements[{arrayPtr, idxRange.low}];
                        } else if (!idxRange.isEmpty() && idxRange.low >= 0 && idxRange.high < 100) {
                            bool found = false;
                            for (int i = idxRange.low; i <= idxRange.high; i++) {
                                if (currentArrayElements.count({arrayPtr, i})) {
                                    loadedRange = found ? loadedRange.meet(currentArrayElements[{arrayPtr, i}]) : 
                                                         currentArrayElements[{arrayPtr, i}];
                                    found = true;
                                }
                            }
                        }
                    }
                    currentRanges[load] = loadedRange;
                } else {
                    currentRanges[load] = currentRanges.count(ptr) ? currentRanges[ptr] : Range(0, 0);
                }
            } else {
                currentRanges[load] = currentRanges.count(ptr) ? currentRanges[ptr] : Range(0, 0);
            }
        
	} else if (auto *binOp = dyn_cast<BinaryOperator>(I)) {		// binary operations
            Range r0 = getRange(binOp->getOperand(0), currentRanges, currentArrayElements);
            Range r1 = getRange(binOp->getOperand(1), currentRanges, currentArrayElements);
            Range result;
            switch (binOp->getOpcode()) {
                case Instruction::Add: result = r0.add(r1); 
				       break;
                case Instruction::Sub: result = r0.sub(r1); 
				       break;
                case Instruction::Mul: result = r0.mul(r1); 
				       break;
                default: result = Range(); 
			 break;
            }
            currentRanges[binOp] = result;
        
	} else if (auto *sext = dyn_cast<SExtInst>(I)) {
            currentRanges[sext] = getRange(sext->getOperand(0), currentRanges, currentArrayElements);
        
	} else if (auto *phi = dyn_cast<PHINode>(I)) {
    		// Only analyze integer PHIs (ignore pointers)
    		if (!phi->getType()->isIntegerTy())
        		return;

    		Range merged; // starts as [INT_MIN, INT_MAX] by default in Range()
    		bool first = true;

    		for (unsigned i = 0; i < phi->getNumIncomingValues(); ++i) {
        		Value *inc = phi->getIncomingValue(i);
        		Range incRange = getRange(inc, currentRanges, currentArrayElements);

        		// If the first incoming value, initialize directly
        		if (first) {
            			merged = incRange;
            			first = false;
        		} else {
            			merged = merged.meet(incRange);
        		}
    		}

    		// Save merged range for this PHI node
    		currentRanges[phi] = merged;
    
	} else if (auto *select = dyn_cast<SelectInst>(I)) {		// ternary operator: c ? a : b
    		Value *trueVal = select->getTrueValue();
    		Value *falseVal = select->getFalseValue();

    		Range trueRange = getRange(trueVal, currentRanges, currentArrayElements);
    		Range falseRange = getRange(falseVal, currentRanges, currentArrayElements);

    		// Merge both branches (conservative)
    		currentRanges[select] = trueRange.meet(falseRange);

	} else if (auto *call = dyn_cast<CallInst>(I)) {		// function calls
    		
	
    		// Conservative handling of all function calls (updated from reference range analysis solution)
    		if (call->getType()->isIntegerTy()) {
        		// Any integer return value -> assume full range [INT_MIN, INT_MAX]
        		currentRanges[call] = Range();
    		}

    		// If any argument is a pointer to a local variable (alloca), conservatively assume it could be modified by the call.
    		for (unsigned i = 0; i < call->arg_size(); ++i) {
        		Value *arg = call->getArgOperand(i);
        		if (auto *allocaInst = dyn_cast<AllocaInst>(arg)) {
            			currentRanges[allocaInst] = Range();
        		}
    		}


		/*
		Function *calledFunc = call->getCalledFunction();
			
    		if (calledFunc && calledFunc->getName() == "llvm.abs.i32") {		// abs() intrinsic
        		Value *arg = call->getArgOperand(0);
        		Range argRange = getRange(arg, currentRanges, currentArrayElements);

        		if (argRange.isEmpty()) {
            			currentRanges[call] = Range(1, 0);
        		} else {
            			int absLow = 0, absHigh = 0;

            			if (argRange.low <= 0 && argRange.high >= 0) {
                			// Range crosses zero
                			absLow = 0;
                			absHigh = std::max(std::abs(argRange.low), std::abs(argRange.high));
            			} else if (argRange.high < 0) {
                			//all negative
                			absLow = std::abs(argRange.high);
                			absHigh = std::abs(argRange.low);
            			} else {
                			//all positive
                			absLow = argRange.low;
                			absHigh = argRange.high;
            			}

            			currentRanges[call] = Range(absLow, absHigh);
        		}

    		} else if (calledFunc && (calledFunc->getName().starts_with("llvm.smax") || calledFunc->getName().starts_with("llvm.smin") ||
                          calledFunc->getName().starts_with("llvm.umax") || calledFunc->getName().starts_with("llvm.umin"))) {			// min/max intrinsics
                    if (call->getNumOperands() >= 2) {
                        Range r0 = getRange(call->getArgOperand(0), currentRanges, currentArrayElements);
                        Range r1 = getRange(call->getArgOperand(1), currentRanges, currentArrayElements);
                        
                        if (calledFunc->getName().contains("smax")) {
                            currentRanges[call] = Range(std::max(r0.low, r1.low), std::max(r0.high, r1.high));
                        } else if (calledFunc->getName().contains("smin")) {
                            currentRanges[call] = Range(std::min(r0.low, r1.low), std::min(r0.high, r1.high));
                        } else {
                            currentRanges[call] = r0.meet(r1);
                        }
                    }
    		} else {
        		// Unknown function - be conservative
        		currentRanges[call] = Range();
    		}
		*/
	}
    }

public:
    void analyze(Function &F) {
        std::map<Value*, Range> ranges;
        for (auto &arg : F.args()) ranges[&arg] = Range();
        for (auto &I : F.getEntryBlock()) {
            if (auto *alloca = dyn_cast<AllocaInst>(&I)) {
                ranges[alloca] = Range(0, 0);
            }
        }

        // Detect infinite loops and mark code after them as unreachable
        for (auto &BB : F) {
            if (isInfiniteLoop(&BB)) {
                auto *br = dyn_cast<BranchInst>(BB.getTerminator());
                if (br && br->isConditional()) {
                    // Mark the false branch (loop exit) as unreachable
                    unreachableBlocks.insert(br->getSuccessor(1));
                }
            }
        }

        for (int iter = 0; iter < 20; iter++) {
            std::vector<BasicBlock*> rpo;
            std::set<BasicBlock*> visited;
            
            // Build reverse postorder
            std::function<void(BasicBlock*)> dfs = [&](BasicBlock *BB) {
                if (visited.count(BB)) return;
                visited.insert(BB);
                for (auto *succ : successors(BB)) {
                    if (!unreachableBlocks.count(succ)) {
                        dfs(succ);
                    }
                }
                rpo.push_back(BB);
            };
            dfs(&F.getEntryBlock());
            std::reverse(rpo.begin(), rpo.end());
            
            visited.clear();
            std::map<BasicBlock*, std::map<Value*, Range>> blockExitRanges;
            std::map<BasicBlock*, std::map<std::pair<Value*, int>, Range>> blockExitArrayRanges;

            for (auto *BB : rpo) {
                if (unreachableBlocks.count(BB)) continue;
                visited.insert(BB);

                std::map<Value*, Range> currentRanges = ranges;
                std::map<std::pair<Value*, int>, Range> currentArrayElements;

                // Merge from predecessors
                bool firstPred = true;
                for (auto *pred : predecessors(BB)) {
                    if (!visited.count(pred) && pred != &F.getEntryBlock()) continue;
                    
                    for (auto &[v, r] : blockExitRanges[pred]) {
			if (currentRanges[v].isEmpty() && r.isEmpty())
        			continue; // both infeasible -> skip merge entirely
                        currentRanges[v] = firstPred ? r : currentRanges[v].meet(r);
                    }
                    firstPred = false;
                }
                
                // merging array elements separately
                firstPred = true;
                for (auto *pred : predecessors(BB)) {
                    if (!visited.count(pred) && pred != &F.getEntryBlock()) continue;
                    
                    for (auto &[k, r] : blockExitArrayRanges[pred]) {
                        
			    Range existing = currentArrayElements[k];

        		    // Skip merge if both paths are infeasible (empty ranges)
        		    if (existing.isEmpty() && r.isEmpty())
            			continue;

        		    currentArrayElements[k] = firstPred ? r : existing.meet(r);
			    
			    //currentArrayElements[k] = firstPred ? r : currentArrayElements[k].meet(r);
                    }
                    firstPred = false;
                }

                //branch refinements
                for (auto *pred : predecessors(BB)) {
                    if (auto *br = dyn_cast<BranchInst>(pred->getTerminator())) {
                        if (br->isConditional()) {
                            if (auto *cmp = dyn_cast<ICmpInst>(br->getCondition())) {
                                Value *op0 = cmp->getOperand(0);
                                if (auto *c = dyn_cast<ConstantInt>(cmp->getOperand(1))) {
                                    int64_t val = c->getSExtValue();
                                    Range cur = currentRanges.count(op0) ? currentRanges[op0] : Range();
                                    Range refined;
                                    bool isTrue = (br->getSuccessor(0) == BB);
                                    
                                    switch (cmp->getPredicate()) {
                                        case ICmpInst::ICMP_SLT:
                                            refined = isTrue ? cur.intersect(Range(std::numeric_limits<int>::min(), val-1)) :
                                                              cur.intersect(Range(val, std::numeric_limits<int>::max()));
                                            break;
                                        case ICmpInst::ICMP_SLE:
                                            refined = isTrue ? cur.intersect(Range(std::numeric_limits<int>::min(), val)) :
                                                              cur.intersect(Range(val+1, std::numeric_limits<int>::max()));
                                            break;
                                        case ICmpInst::ICMP_SGT:
                                            refined = isTrue ? cur.intersect(Range(val+1, std::numeric_limits<int>::max())) :
                                                              cur.intersect(Range(std::numeric_limits<int>::min(), val));
                                            break;
                                        case ICmpInst::ICMP_SGE:
                                            refined = isTrue ? cur.intersect(Range(val, std::numeric_limits<int>::max())) :
                                                              cur.intersect(Range(std::numeric_limits<int>::min(), val-1));
                                            break;
                                        default: continue;
                                    }
                                    currentRanges[op0] = refined;
                                    if (auto *load = dyn_cast<LoadInst>(op0)) {
                                        currentRanges[load->getPointerOperand()] = refined;
                                    }
                                }
                            }
                        }
                    } else if (auto *sw = dyn_cast<SwitchInst>(pred->getTerminator())) {
                        for (auto &c : sw->cases()) {
                            if (c.getCaseSuccessor() == BB) {
                                int64_t val = c.getCaseValue()->getSExtValue();
                                currentRanges[sw->getCondition()] = Range(val, val);
                                if (auto *load = dyn_cast<LoadInst>(sw->getCondition())) {
                                    currentRanges[load->getPointerOperand()] = Range(val, val);
                                }
                            }
                        }
                    }
                }

                // save entry state
                for (auto &[v, r] : currentRanges) blockEntryRanges[{BB, v}] = r;
                for (auto &[k, r] : currentArrayElements) blockEntryArrayElements[{BB, k}] = r;

                // process instructions
                for (auto &I : *BB) {
                    processInstruction(&I, currentRanges, currentArrayElements);
                }

                //save exit state
                blockExitRanges[BB] = currentRanges;
                blockExitArrayRanges[BB] = currentArrayElements;
            }
        }
    }

    Range getRangeAt(BasicBlock *BB, Instruction *I, Value *V) {
        if (auto *c = dyn_cast<ConstantInt>(V)) {
            return Range(c->getSExtValue(), c->getSExtValue());
        }
        if (auto *sext = dyn_cast<SExtInst>(V)) {
            return getRangeAt(BB, I, sext->getOperand(0));
        }

        // Replay instructions up to I to get current state
        std::map<Value*, Range> currentRanges;
        std::map<std::pair<Value*, int>, Range> currentArrayElements;
        
        for (auto &[key, r] : blockEntryRanges) {
            if (key.first == BB) currentRanges[key.second] = r;
        }
        for (auto &[key, r] : blockEntryArrayElements) {
            if (key.first == BB) currentArrayElements[key.second] = r;
        }

        for (auto &inst : *BB) {
            if (&inst == I) break;
            processInstruction(&inst, currentRanges, currentArrayElements);
        }

        return currentRanges.count(V) ? currentRanges[V] : Range();
    }

    bool isUnreachable(BasicBlock *BB) {
        return unreachableBlocks.count(BB);
    }
};

void instrumentGEP(GetElementPtrInst *gep) {
    Value *basePtr = gep->getPointerOperand();
    auto *alloca = dyn_cast<AllocaInst>(basePtr);
    if (!alloca) 
	    return;

    auto *arrayType = dyn_cast<ArrayType>(alloca->getAllocatedType());
    if (!arrayType) 
	    return;

    uint64_t arraySize = arrayType->getNumElements();
    Value *indexValue = gep->getOperand(gep->getNumOperands() - 1);

    IRBuilder<> builder(gep);
    Value *upperCheck = builder.CreateICmpSGE(indexValue, builder.getInt64(arraySize));
    Value *lowerCheck = builder.CreateICmpSLT(indexValue, builder.getInt64(0));
    Value *outOfBounds = builder.CreateOr(upperCheck, lowerCheck);

    BasicBlock *currentBB = gep->getParent();
    BasicBlock *continueBB = currentBB->splitBasicBlock(gep->getIterator());

    Function *F = currentBB->getParent();
    BasicBlock *sinkBB = BasicBlock::Create(F->getContext(), "sink", F);
    IRBuilder<> sinkBuilder(sinkBB);
    sinkBuilder.CreateRet(sinkBuilder.getInt32(-1));

    currentBB->getTerminator()->eraseFromParent();
    builder.SetInsertPoint(currentBB);
    builder.CreateCondBr(outOfBounds, sinkBB, continueBB);
}

PreservedAnalyses instrumentFunction(Function &F) {
    if (F.getName() != "test") 
	    return PreservedAnalyses::all();

    RangeAnalysis rangeAnalysis;
    rangeAnalysis.analyze(F);

    std::map<Value*, uint64_t> arraySizes;
    for (auto &I : F.getEntryBlock()) {
        if (auto *alloca = dyn_cast<AllocaInst>(&I)) {
            if (auto *arrayType = dyn_cast<ArrayType>(alloca->getAllocatedType())) {
                arraySizes[alloca] = arrayType->getNumElements();
            }
        }
    }

    std::vector<GetElementPtrInst*> gepsToInstrument;

    for (auto &BB : F) {
        for (auto &I : BB) {
            if (auto *gep = dyn_cast<GetElementPtrInst>(&I)) {
                Value *basePtr = gep->getPointerOperand();
                if (arraySizes.count(basePtr) && gep->getNumOperands() > 2) {
                    //check unreachability first
                    if (rangeAnalysis.isUnreachable(&BB)) {
                        errs() << "Skipped instrumentation for " << *gep << "\n";
                        continue;
                    }

                    Value *indexValue = gep->getOperand(gep->getNumOperands() - 1);
                    Value *actualIndex = indexValue;
                    if (auto *sext = dyn_cast<SExtInst>(indexValue)) {
                        actualIndex = sext->getOperand(0);
                    }

                    Range indexRange = rangeAnalysis.getRangeAt(&BB, gep, actualIndex);

		    /*
                    // DEBUG
                    errs() << "GEP: " << *gep << "\n";
                    errs() << "  Index: " << *actualIndex << "\n";
                    errs() << "  Range: [" << indexRange.low << ", " << indexRange.high << "]\n";
                    errs() << "  Array size: " << arraySizes[basePtr] << "\n";
		    */


                    if (indexRange.isEmpty()) {
                        errs() << "Skipped instrumentation for " << *gep << " (empty range)\n";
                        continue;
                    }

                    uint64_t arraySize = arraySizes[basePtr];
                    bool needsInstrumentation = (indexRange.low < 0 || indexRange.high >= (int)arraySize);

                    if (needsInstrumentation) {
                        errs() << "Instrumenting " << *gep << "\n";
                        gepsToInstrument.push_back(gep);
                    } else {
                        errs() << "Skipped instrumentation for " << *gep << "\n";
                    }
                }
            }
        }
    }

    for (auto *gep : gepsToInstrument) {
        instrumentGEP(gep);
    }

    return PreservedAnalyses::none();
}

// =============================================================================================================================

//
// Registering the Function Pass (Don't change any code below)
//
class ArrayInstrumentationPass : public PassInfoMixin<ArrayInstrumentationPass> {
public:
    static bool isRequired() { return true; }

    PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
        return instrumentFunction(F);
    };
};

}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "Array InstrumentationPass",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "instrument-array-accesses") {
                        FPM.addPass(ArrayInstrumentationPass());
                        return true;
                    }
                    return false;
                });
        }
    };
}


