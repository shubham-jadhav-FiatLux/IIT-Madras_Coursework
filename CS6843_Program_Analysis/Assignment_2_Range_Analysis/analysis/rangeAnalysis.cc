// Program Analysis : Flow-sensitive Intra-procedural Range Analysis
// Author	: Shubham Jadhav (CS24S009)
// Date		: 1 Sep, 2025


// Header files
#include <map>
#include <set>
#include <queue>
#include <algorithm>

// LLVM related header files
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/CFG.h"

#include "llvm/IR/Instructions.h"

using namespace llvm;

namespace {

//
// Range : Represent a range of integer values
//
class Range {
private:
    int lower, upper;   // Lower and Upper bounds (both inclusive)

public:
    // Constructors
    Range(int a) : lower(a), upper(a) {};
    Range(int lower, int upper) : lower(lower), upper(upper) {};
    Range();

    // Getters
    int getLower() const { return lower; }
    int getUpper() const { return upper; }

    // Functions to perform arithmetic on Ranges
    static Range addRange(Range r1, Range r2);
    static Range subRange(Range r1, Range r2);
    static Range mulRange(Range r1, Range r2);
    static Range mergeRange(Range r1, Range r2);
    static Range intersectRange(Range r1, Range r2);

    void print(raw_ostream &os);
    // Comparison operators for Ranges
    friend bool operator< (const Range& lhs, const Range& rhs);
    friend bool operator==(const Range& lhs, const Range& rhs);
    friend bool operator!=(const Range& lhs, const Range& rhs);
};

// Special Ranges to represent Top and Bottom of the Lattice
static Range FULL_RANGE(INT_MIN, INT_MAX);
static Range EMPTY_RANGE(INT_MAX, INT_MIN);

Range::Range() {
    lower = EMPTY_RANGE.lower;
    upper = EMPTY_RANGE.upper;
}

void Range::print(raw_ostream &os) { 
    if (*this == EMPTY_RANGE) os << "[EMPTY]";
    else os << "[" << lower << "," << upper << "]";
}


/*
 *
Saturating semantics, i.e. if the result of an arithmetic operation is greater than INT MAX, set it to INT MAX. 
Similarly, if the result of an arithmetic operation is smaller than INT MIN, set it to INT MIN.
*
*/
// Helper functions for saturating arithmetic
static int64_t saturatingAdd(int64_t a, int64_t b) {

    int64_t result = a + b;
    if (result > INT_MAX) return INT_MAX;
    if (result < INT_MIN) return INT_MIN;
    
    return result;
}

static int64_t saturatingSub(int64_t a, int64_t b) {
    
    int64_t result = a - b;
    if (result > INT_MAX) return INT_MAX;
    if (result < INT_MIN) return INT_MIN;
    
    return result;
}

static int64_t saturatingMul(int64_t a, int64_t b) {

    int64_t result = a * b;
    
    if (result > INT_MAX) return INT_MAX;
    if (result < INT_MIN) return INT_MIN;
    
    return result;
}

Range Range::addRange(Range r1, Range r2) {

    if (r1 == EMPTY_RANGE || r2 == EMPTY_RANGE) return EMPTY_RANGE;
    
    int64_t newLower = saturatingAdd((int64_t)r1.lower, (int64_t)r2.lower);
    int64_t newUpper = saturatingAdd((int64_t)r1.upper, (int64_t)r2.upper);
    
    return Range(newLower, newUpper);
}

Range Range::subRange(Range r1, Range r2) {

    if (r1 == EMPTY_RANGE || r2 == EMPTY_RANGE) return EMPTY_RANGE;
    
    int64_t newLower = saturatingSub((int64_t)r1.lower, (int64_t)r2.upper);
    int64_t newUpper = saturatingSub((int64_t)r1.upper, (int64_t)r2.lower);
    
    return Range(newLower, newUpper);
}

Range Range::mulRange(Range r1, Range r2) {

    if (r1 == EMPTY_RANGE || r2 == EMPTY_RANGE) return EMPTY_RANGE;
    
    // Calculating all four products
    int64_t prod1 = saturatingMul((int64_t)r1.lower, (int64_t)r2.lower);
    int64_t prod2 = saturatingMul((int64_t)r1.lower, (int64_t)r2.upper);
    int64_t prod3 = saturatingMul((int64_t)r1.upper, (int64_t)r2.lower);
    int64_t prod4 = saturatingMul((int64_t)r1.upper, (int64_t)r2.upper);
    
    int64_t newLower = std::min({prod1, prod2, prod3, prod4});
    int64_t newUpper = std::max({prod1, prod2, prod3, prod4});
    
    return Range(newLower, newUpper);
}

// Meet operation - union of ranges
Range Range::mergeRange(Range r1, Range r2) {
    
    if (r1 == EMPTY_RANGE) return r2;
    if (r2 == EMPTY_RANGE) return r1;
    
    int newLower = std::min(r1.lower, r2.lower);
    int newUpper = std::max(r1.upper, r2.upper);
    
    return Range(newLower, newUpper);
}

// Intersection of ranges
Range Range::intersectRange(Range r1, Range r2) {
    
    if (r1 == EMPTY_RANGE || r2 == EMPTY_RANGE) return EMPTY_RANGE;
    
    int newLower = std::max(r1.lower, r2.lower);
    int newUpper = std::min(r1.upper, r2.upper);
    
    if (newLower > newUpper) return EMPTY_RANGE;
    
    return Range(newLower, newUpper);
}

/// Helper function to print Range
raw_ostream& operator<<(raw_ostream& os, Range r) { r.print(os); return os; }

// Comparison operators for Ranges
bool operator< (const Range& lhs, const Range& rhs) {
    if (lhs == EMPTY_RANGE) return true;
    if (rhs == EMPTY_RANGE) return false;
    return std::tie(lhs.lower, lhs.upper) < std::tie(rhs.lower, rhs.upper);
}
bool operator==(const Range& lhs, const Range& rhs) {
    return std::tie(lhs.lower, lhs.upper) == std::tie(rhs.lower, rhs.upper);
}
bool operator!=(const Range& lhs, const Range& rhs) {
    return std::tie(lhs.lower, lhs.upper) != std::tie(rhs.lower, rhs.upper);
}

class BasicBlockState;
// Out-states of the function being analysed (used to evaluate phi nodes per incoming block)
static std::map<BasicBlock*, BasicBlockState*>* gBlockOut = nullptr;

//
// BasicBlockState : Represent the data flow facts of a Basic Block
//
class BasicBlockState {
private:
    std::map<Value*, Range> varRanges;  // To Map each variable to its range

public:
    BasicBlockState () {}

    void setRange(Value* var, Range r) {

        varRanges[var] = r;
    }

    Range getRange(Value* var) {

        if (varRanges.find(var) != varRanges.end()) {

            return varRanges[var];
        }

        return FULL_RANGE;  // Unknown variables have full range
    }

    // Range of an arbitrary integer value under the current state
    Range evalValue(Value* v) {

        if (ConstantInt* CI = dyn_cast<ConstantInt>(v)) {

            int64_t constVal = CI->getSExtValue();
            if (constVal > INT_MAX) constVal = INT_MAX;
            if (constVal < INT_MIN) constVal = INT_MIN;
            return Range(constVal, constVal);
        }

        if (LoadInst* load = dyn_cast<LoadInst>(v)) {

            return getRange(load->getPointerOperand());		// value read from a variable
        }

        if (BinaryOperator* binOp = dyn_cast<BinaryOperator>(v)) {

            Range r1 = evalValue(binOp->getOperand(0));
            Range r2 = evalValue(binOp->getOperand(1));

            switch (binOp->getOpcode()) {

                case Instruction::Add: return Range::addRange(r1, r2);
                case Instruction::Sub: return Range::subRange(r1, r2);
                case Instruction::Mul: return Range::mulRange(r1, r2);
                default:               return FULL_RANGE;
            }
        }

        if (SelectInst* sel = dyn_cast<SelectInst>(v)) {

            // Ternary operator : result is either of the two values
            return Range::mergeRange(evalValue(sel->getTrueValue()), evalValue(sel->getFalseValue()));
        }

        if (PHINode* phi = dyn_cast<PHINode>(v)) {

            // Ternary / short-circuit merge : evaluate each incoming value in the state at the end of its block
            Range result = EMPTY_RANGE;
            for (unsigned i = 0; i < phi->getNumIncomingValues(); i++) {

                BasicBlock* inBB = phi->getIncomingBlock(i);
                BasicBlockState* inState = nullptr;
                if (gBlockOut && gBlockOut->count(inBB)) inState = (*gBlockOut)[inBB];
                if (!inState) continue;			// predecessor not visited yet
                result = Range::mergeRange(result, inState->evalValue(phi->getIncomingValue(i)));
            }
            return result == EMPTY_RANGE ? FULL_RANGE : result;
        }

        return FULL_RANGE;		// calls (e.g. abs), casts, function arguments, ... : unknown
    }

    // Transfer function for a single instruction
    void transfer(Instruction* inst) {

        if (StoreInst* store = dyn_cast<StoreInst>(inst)) {

            setRange(store->getPointerOperand(), evalValue(store->getValueOperand()));

        } else if (CallInst* call = dyn_cast<CallInst>(inst)) {

            // A callee may modify any local variable whose address is passed to it (scanf, printf, ...)
            for (unsigned i = 0; i < call->arg_size(); i++) {

                Value* arg = call->getArgOperand(i);
                if (arg->getType()->isPointerTy() && isa<AllocaInst>(arg)) {

                    setRange(arg, FULL_RANGE);
                }
            }
        }
    }

    // meet function - merge two states
    static BasicBlockState* meet(BasicBlockState* s1, BasicBlockState* s2) {

        if (!s1) return s2;
        if (!s2) return s1;
        
        BasicBlockState* result = new BasicBlockState();
        
        // Union all variables
        std::set<Value*> allVars;
        for (auto& p : s1->varRanges) allVars.insert(p.first);
        for (auto& p : s2->varRanges) allVars.insert(p.first);
        
        for (Value* var : allVars) {

            Range r1 = s1->getRange(var);
            Range r2 = s2->getRange(var);
            result->setRange(var, Range::mergeRange(r1, r2));
        }
        
        return result;
    }

    // Applying condition constraints for conditional branches
    void applyCondition(ICmpInst* cmp, bool trueBranch) {

        if (!cmp) return;
        
        CmpInst::Predicate pred = cmp->getPredicate();
        Value* op0 = cmp->getOperand(0);
        Value* op1 = cmp->getOperand(1);
        
        // Checking if it is a comparison with a constant
        LoadInst* load = dyn_cast<LoadInst>(op0);
        ConstantInt* CI = dyn_cast<ConstantInt>(op1);
        
        if (load && CI) {

            Value* ptr = load->getPointerOperand();
            int64_t constVal = CI->getSExtValue();
            if (constVal > INT_MAX) constVal = INT_MAX;
            if (constVal < INT_MIN) constVal = INT_MIN;
            
            Range currentRange = getRange(ptr);
            Range newRange = currentRange;
            
            if (trueBranch) {

                switch (pred) {

                    case ICmpInst::ICMP_SGT:  // var > k
                        newRange = Range::intersectRange(currentRange, Range(constVal + 1, INT_MAX));
                        break;
                    case ICmpInst::ICMP_SGE:  // var >= k
                        newRange = Range::intersectRange(currentRange, Range(constVal, INT_MAX));
                        break;
                    case ICmpInst::ICMP_SLT:  // var < k
                        newRange = Range::intersectRange(currentRange, Range(INT_MIN, constVal - 1));
                        break;
                    case ICmpInst::ICMP_SLE:  // var <= k
                        newRange = Range::intersectRange(currentRange, Range(INT_MIN, constVal));
                        break;
                    default:
                        break;
                }
            } else {

                // False branch - negate the condition
                switch (pred) {

                    case ICmpInst::ICMP_SGT:  // !(var > k) => var <= k
                        newRange = Range::intersectRange(currentRange, Range(INT_MIN, constVal));
                        break;
                    case ICmpInst::ICMP_SGE:  // !(var >= k) => var < k
                        newRange = Range::intersectRange(currentRange, Range(INT_MIN, constVal - 1));
                        break;
                    case ICmpInst::ICMP_SLT:  // !(var < k) => var >= k
                        newRange = Range::intersectRange(currentRange, Range(constVal, INT_MAX));
                        break;
                    case ICmpInst::ICMP_SLE:  // !(var <= k) => var > k
                        newRange = Range::intersectRange(currentRange, Range(constVal + 1, INT_MAX));
                        break;
                    default:
                        break;
                }
            }
            
            setRange(ptr, newRange);
        }
    }

    bool equals(BasicBlockState* other) {

        if (!other) return false;
        return varRanges == other->varRanges;
    }

    void print(raw_ostream &os) {

        os << "Basic Block State:\n";
        for (auto& p : varRanges) {
            os << "  " << p.first << ": " << p.second << "\n";
        }
    }

    std::map<Value*, Range>& getVarRanges() { return varRanges; }
};

//
// Intra-procedural Data flow Analysis
//
void analyseFunction(Function &F) {

    std::map<BasicBlock*, BasicBlockState*> BBStateIn;
    std::map<BasicBlock*, BasicBlockState*> BBStateOut;
    gBlockOut = &BBStateOut;
    
    // Initialize all states
    for (BasicBlock &bb : F) {

        BBStateIn[&bb] = nullptr;
        BBStateOut[&bb] = nullptr;
    }
    
    // Initialize entry block
    BasicBlock* entry = &F.getEntryBlock();
    BBStateIn[entry] = new BasicBlockState();
    
    // initialize function arguments with full range
    for (Argument &arg : F.args()) {

        if (arg.getType()->isIntegerTy(32)) {

            for (User* user : arg.users()) {

                if (StoreInst* store = dyn_cast<StoreInst>(user)) {

                    BBStateIn[entry]->setRange(store->getPointerOperand(), FULL_RANGE);
                }
            }
        }
    }


    /*
     *
     To get a list of all back edges in the function, 
     you can use the in-built LLVM function llvm::FindFunctionBackedges(&F,...).
    *
    */
    
    // finding back edges for loop handling
    SmallVector<std::pair<const BasicBlock*, const BasicBlock*>, 8> backEdges;
    FindFunctionBackedges(F, backEdges);
    std::set<std::pair<const BasicBlock*, const BasicBlock*>> backEdgeSet(backEdges.begin(), backEdges.end());
    

    /*
     *
     WORKLIST ALGORITHM:

	1. Initialize:
   	- Create an empty worklist (queue/stack)
   	- Add initial work items to the worklist
   	- Set up any tracking data structures

	2. Main Loop:
   	WHILE worklist is not empty:
       	a) Remove one item from worklist
       	b) Process the item
       	c) If processing generates new work:
          - Add new work items to worklist
          - Update tracking structures

	3. Finish:
   	- Return results or perform cleanup
    *
    */

    // Worklist Algorithm
    std::queue<BasicBlock*> worklist;
    worklist.push(entry);
    std::set<BasicBlock*> inWorklist;
    inWorklist.insert(entry);
    
    while (!worklist.empty()) {

        BasicBlock* bb = worklist.front();
        worklist.pop();
        inWorklist.erase(bb);
        
        // compute input state from predecessors
        BasicBlockState* newIn = nullptr;
        
        for (BasicBlock* pred : predecessors(bb)) {
            BasicBlockState* predOut = BBStateOut[pred];
            if (!predOut) continue;
            
            bool isBackEdge = (backEdgeSet.find({pred, bb}) != backEdgeSet.end());	// checking if this is a back edge
            
            if (isBackEdge) {

                // for back edges, widen to full range for variables that changed
                BasicBlockState* widenedState = new BasicBlockState();
                if (BBStateIn[bb]) {

                    for (auto& p : BBStateIn[bb]->getVarRanges()) {

                        Value* var = p.first;
                        Range bbInRange = p.second;
                        Range predRange = predOut->getRange(var);
                        
                        if (bbInRange != predRange) {
                           
                            widenedState->setRange(var, FULL_RANGE);	// widen to FULL_RANGE as variable in loop changes
                        } else {

                            widenedState->setRange(var, bbInRange);
                        }
                    }
                    // checking variables in predOut but not in bbIn
                    for (auto& p : predOut->getVarRanges()) {

                        Value* var = p.first;
                        if (BBStateIn[bb]->getVarRanges().find(var) == BBStateIn[bb]->getVarRanges().end()) {

                            widenedState->setRange(var, FULL_RANGE);
                        }
                    }
                } else {

                    widenedState = new BasicBlockState(*predOut);
                }
                newIn = BasicBlockState::meet(newIn, widenedState);
            } else {

                // Handling conditional branches
                BasicBlockState* predStateCopy = new BasicBlockState(*predOut);
                
                if (BranchInst* br = dyn_cast<BranchInst>(pred->getTerminator())) {

                    if (br->isConditional()) {

                        if (ICmpInst* cmp = dyn_cast<ICmpInst>(br->getCondition())) {

                            bool isTrueBranch = (br->getSuccessor(0) == bb);
                            predStateCopy->applyCondition(cmp, isTrueBranch);
                        }
                    }
                }
                else if (SwitchInst* sw = dyn_cast<SwitchInst>(pred->getTerminator())) {

                    // Switch-case edge : the switch variable equals one of the case values leading to this block
                    if (LoadInst* condLoad = dyn_cast<LoadInst>(sw->getCondition())) {

                        if (sw->getDefaultDest() != bb) {

                            Range caseRange = EMPTY_RANGE;
                            for (auto& c : sw->cases()) {

                                if (c.getCaseSuccessor() == bb) {

                                    caseRange = Range::mergeRange(caseRange, Range(c.getCaseValue()->getSExtValue()));
                                }
                            }
                            if (caseRange != EMPTY_RANGE) {

                                Value* var = condLoad->getPointerOperand();
                                predStateCopy->setRange(var, Range::intersectRange(predStateCopy->getRange(var), caseRange));
                            }
                        }
                    }
                }
                
                newIn = BasicBlockState::meet(newIn, predStateCopy);
            }
        }
        
        if (!newIn) {
            newIn = new BasicBlockState();
        }
        
        // checking if input state changed
        bool changed = !BBStateIn[bb] || !BBStateIn[bb]->equals(newIn);
        BBStateIn[bb] = newIn;
        
        // process the basic block
        BasicBlockState* currentState = new BasicBlockState(*newIn);
        for (Instruction &inst : *bb) {
            currentState->transfer(&inst);
        }
        
        // check if output state changed
        if (!BBStateOut[bb] || !BBStateOut[bb]->equals(currentState)) {
            BBStateOut[bb] = currentState;
            changed = true;
        }
        
        // Adding successors to worklist if state changed
        if (changed) {
            for (BasicBlock* succ : successors(bb)) {
                if (inWorklist.find(succ) == inWorklist.end()) {
                    worklist.push(succ);
                    inWorklist.insert(succ);
                }
            }
        }
    }
    
    // Print the results
    errs() << "Function " << F.getName() << "\n";
    
    // Find exit blocks and print results
    for (BasicBlock &bb : F) {
        if (succ_size(&bb) == 0) {
            BasicBlockState* finalState = BBStateOut[&bb];
            if (!finalState) continue;
            
            // Collect all variables
            std::map<StringRef, Value*> programVariables;
            for (BasicBlock &b : F) {
                for (Instruction &inst : b) {
                    for (DbgVariableRecord &DVR : filterDbgVars(inst.getDbgRecordRange())) {
                        programVariables[DVR.getVariable()->getName()] = DVR.getAddress();
                    }
                }
            }
            
            // Also include function arguments
            for (Argument &arg : F.args()) {
                if (arg.hasName()) {
                    // Find the alloca for this argument
                    for (User* user : arg.users()) {
                        if (StoreInst* store = dyn_cast<StoreInst>(user)) {
                            if (store->getValueOperand() == &arg) {
                                programVariables[arg.getName()] = store->getPointerOperand();
                                break;
                            }
                        }
                    }
                }
            }
            
            // Print ranges for all variables
            for (auto& entry : programVariables) {
                StringRef variableName = entry.first;
                Value* variableValue = entry.second;
                Range r = finalState->getRange(variableValue);
                errs() << variableName << " : " << r << "\n";
            }
        }
    }
    errs() << "\n";
}

//
// Registering the Function Pass (Don't change any code below)
//
class RangeAnalysisPass : public PassInfoMixin<RangeAnalysisPass> {
public:
    static bool isRequired() { return true; }

    PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
        analyseFunction(F);
        return PreservedAnalyses::all();
    };
};

}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "Range Analysis Pass",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "range-analysis") {
                        FPM.addPass(RangeAnalysisPass());
                        return true;
                    }
                    return false;
                });
        }
    };
}

