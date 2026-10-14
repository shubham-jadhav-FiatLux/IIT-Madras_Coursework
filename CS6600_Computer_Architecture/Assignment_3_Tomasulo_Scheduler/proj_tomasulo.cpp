// CS6600:Computer Architecture
// 03 November, 2024
// Author : Shubham Jadhav
// CS24S009
// IIT Madras

/*
Project Description:
In this project, you will construct a simulator for an out-of-order superscalar processor based on
Tomasulo's algorithm that fetches, dispatches, and issues N instructions per
cycle. The primary goal is to only model the dynamic scheduling mechanism in detail. Hence,
perfect caches and perfect branch prediction are assumed.
*/


// Header Files
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <bits/stdc++.h>


using namespace std;


// Global Variables
int nN; 
int nS; 
string strTraceFileName;

int nCycle = 0;
int nTotalInstructions = 0;


// Enumeration for instruction states
enum InstrState { IF, ID, IS, EX, WB };

// Structure
struct sTimingInfo {
    int nBeginCycle;
    int nDuration;
};

struct sInstruction {
    int nTag; 
    unsigned int nPC;
    int nOpType; 
    int nDest; 
    int nSrc1; 
    int nSrc2; 
    InstrState eState; 
    sTimingInfo aTiming[5]; 
    int nSrc1Tag; 
    int nSrc2Tag; 
    bool bSrc1Ready; 
    bool bSrc2Ready; 
    int nExeCyclesLeft; 
    bool bCompleted; 

    sInstruction() {
        nTag = -1;
        nPC = 0;
        nOpType = 0;
        nDest = -1;
        nSrc1 = -1;
        nSrc2 = -1;
        eState = IF;
        memset(aTiming, 0, sizeof(aTiming));
        nSrc1Tag = -1;
        nSrc2Tag = -1;
        bSrc1Ready = true;
        bSrc2Ready = true;
        nExeCyclesLeft = 0;
        bCompleted = false;
    }
};

// Register file entry
struct sRegister {
    int nTag; 
    bool bReady; 

    sRegister() {
        nTag = -1;
        bReady = true;
    }
};

// Register File
sRegister aRegFile[128]; 

// Function Declaration
void Fetch_Instruction(ifstream& fileTrace);
void Dispatch_Instruction();
void Issue_Instruction();
void Execute_Instruction();
void Retire_Instruction();
bool AdvanceCycle();
void PrintResults();
void Uninitialize();


// Data Structures
vector<sInstruction*> aROB;  
vector<sInstruction*> aDispatch_InstructionList;
vector<sInstruction*> aSchedulingQueue; 
vector<sInstruction*> aExecute_InstructionList; 



// Function Definitions
void Fetch_Instruction(ifstream& fileTrace) 
{
    int nFetch_InstructionCount = 0;
    while (nFetch_InstructionCount < nN && aDispatch_InstructionList.size() < 2 * nN) 
    {
        string strLine;
        if (!getline(fileTrace, strLine)) 
        {
            break; 
        }

        if (strLine.empty()) continue;

        sInstruction* pInstr = new sInstruction();
        pInstr->nTag = nTotalInstructions;
        nTotalInstructions++;

        istringstream iss(strLine);
        string strPC;

        if (!(iss >> strPC >> pInstr->nOpType >> pInstr->nDest >> pInstr->nSrc1 >> pInstr->nSrc2)) 
        {
            cerr << "Error : Parsing trace file " << pInstr->nTag << endl;
            exit(1);
        }
        pInstr->nPC = (unsigned int)stoul(strPC, nullptr, 16);

        pInstr->eState = IF;
        pInstr->aTiming[IF].nBeginCycle = nCycle;
        pInstr->aTiming[IF].nDuration = 1;

        aROB.push_back(pInstr);
        aDispatch_InstructionList.push_back(pInstr);

        nFetch_InstructionCount++;
    }
}


void Dispatch_Instruction() 
{
    int nDispatch_InstructionCount = 0;
    vector<sInstruction*> aToRemove;
    vector<sInstruction*> aTempList;

    
    for (auto pInstr : aDispatch_InstructionList) 
    {
        if (pInstr->eState == ID) 
        {
            aTempList.push_back(pInstr);
        }
    }

    
    sort(aTempList.begin(), aTempList.end(), [](sInstruction* pA, sInstruction* pB) 
    {
        return pA->nTag < pB->nTag;
    });

   
    for (auto pInstr : aTempList) 
    {
        if (nDispatch_InstructionCount >= nN) break;
        if (aSchedulingQueue.size() >= nS) break; 

        
        pInstr->eState = IS;
        pInstr->aTiming[ID].nDuration = nCycle - pInstr->aTiming[ID].nBeginCycle;
        pInstr->aTiming[IS].nBeginCycle = nCycle;

        
        if (pInstr->nSrc1 != -1) {
            if (aRegFile[pInstr->nSrc1].bReady) 
            {
                pInstr->bSrc1Ready = true;
            } 
            else 
            {
                pInstr->bSrc1Ready = false;
                pInstr->nSrc1Tag = aRegFile[pInstr->nSrc1].nTag;
            }
        } 
        else 
        {
            pInstr->bSrc1Ready = true;
        }

        
        if (pInstr->nSrc2 != -1) 
        {
            if (aRegFile[pInstr->nSrc2].bReady) 
            {
                pInstr->bSrc2Ready = true;
            } 
            else 
            {
                pInstr->bSrc2Ready = false;
                pInstr->nSrc2Tag = aRegFile[pInstr->nSrc2].nTag;
            }
        } 
        else 
        {
            pInstr->bSrc2Ready = true;
        }

        
        if (pInstr->nDest != -1) 
        {
            aRegFile[pInstr->nDest].bReady = false;
            aRegFile[pInstr->nDest].nTag = pInstr->nTag;
        }

      
        aSchedulingQueue.push_back(pInstr);
        aToRemove.push_back(pInstr);
        nDispatch_InstructionCount++;
    }

    
    for (auto pInstr : aDispatch_InstructionList) 
    {
        if (pInstr->eState == IF) 
        {
            pInstr->eState = ID;
            pInstr->aTiming[ID].nBeginCycle = nCycle;
        }
    }

   
    for (auto pInstr : aToRemove) 
    {
        aDispatch_InstructionList.erase(remove(aDispatch_InstructionList.begin(), aDispatch_InstructionList.end(), pInstr), aDispatch_InstructionList.end());
    }
}


void Issue_Instruction() 
{
    int nIssue_InstructionCount = 0;
    vector<sInstruction*> aReadyInstructions;

    for (auto pInstr : aSchedulingQueue) 
    {
        if (pInstr->bSrc1Ready && pInstr->bSrc2Ready) 
        {
            aReadyInstructions.push_back(pInstr);
        }
    }

   
    sort(aReadyInstructions.begin(), aReadyInstructions.end(), [](sInstruction* pA, sInstruction* pB) 
    {
        return pA->nTag < pB->nTag;
    });

   
    for (auto pInstr : aReadyInstructions) 
    {
        if (nIssue_InstructionCount >= nN) break;

      
        pInstr->eState = EX;
        pInstr->aTiming[IS].nDuration = nCycle - pInstr->aTiming[IS].nBeginCycle;
        pInstr->aTiming[EX].nBeginCycle = nCycle;

   
        if (pInstr->nOpType == 0) 
        {
            pInstr->nExeCyclesLeft = 1;
        } 
        else if (pInstr->nOpType == 1) 
        {
            pInstr->nExeCyclesLeft = 2;
        } 
        else if (pInstr->nOpType == 2) 
        {
            pInstr->nExeCyclesLeft = 10;
        }

   
        aExecute_InstructionList.push_back(pInstr);

   
        aSchedulingQueue.erase(remove(aSchedulingQueue.begin(), aSchedulingQueue.end(), pInstr), aSchedulingQueue.end());

        nIssue_InstructionCount++;
    }
}


void Execute_Instruction() 
{
    vector<sInstruction*> aCompletedInstructions;

 
    for (auto pInstr : aExecute_InstructionList) 
    {
        pInstr->nExeCyclesLeft--;
        if (pInstr->nExeCyclesLeft == 0) 
        {
      
            pInstr->eState = WB;
            pInstr->aTiming[EX].nDuration = nCycle - pInstr->aTiming[EX].nBeginCycle;
            pInstr->aTiming[WB].nBeginCycle = nCycle;
            pInstr->aTiming[WB].nDuration = 1;
            pInstr->bCompleted = true;

            aCompletedInstructions.push_back(pInstr);

       
            if (pInstr->nDest != -1 && aRegFile[pInstr->nDest].nTag == pInstr->nTag) 
            {
                aRegFile[pInstr->nDest].bReady = true;
            }

       
            for (auto pWaitingInstr : aSchedulingQueue) 
            {
                if (!pWaitingInstr->bSrc1Ready && pWaitingInstr->nSrc1Tag == pInstr->nTag) 
                {
                    pWaitingInstr->bSrc1Ready = true;
                }
                if (!pWaitingInstr->bSrc2Ready && pWaitingInstr->nSrc2Tag == pInstr->nTag) 
                {
                    pWaitingInstr->bSrc2Ready = true;
                }
            }
        }
    }


    for (auto pInstr : aCompletedInstructions) 
    {
        aExecute_InstructionList.erase(remove(aExecute_InstructionList.begin(), aExecute_InstructionList.end(), pInstr), aExecute_InstructionList.end());
    }
}


// The ROB doubles as the instruction log that PrintResults() walks, so instructions are never removed from it;
// retirement itself needs no modelling in this simulator (no precise-exception state is kept).
void Retire_Instruction() 
{
}

bool AdvanceCycle() 
{
    nCycle++;


    bool bAllCompleted = true;
    for (auto pInstr : aROB) 
    {
        if (!pInstr->bCompleted) 
        {
            bAllCompleted = false;
            break;
        }
    }

    return !bAllCompleted;
}


void PrintResults() 
{
    
    for (auto pInstr : aROB) 
    {
        cout << pInstr->nTag << "  fu{" << pInstr->nOpType << "} src{" << pInstr->nSrc1 << "," << pInstr->nSrc2 << "} dst{" << pInstr->nDest << "} ";
        cout << "IF{" << pInstr->aTiming[IF].nBeginCycle << "," << pInstr->aTiming[IF].nDuration << "} ";
        cout << "ID{" << pInstr->aTiming[ID].nBeginCycle << "," << pInstr->aTiming[ID].nDuration << "} ";
        cout << "IS{" << pInstr->aTiming[IS].nBeginCycle << "," << pInstr->aTiming[IS].nDuration << "} ";
        cout << "EX{" << pInstr->aTiming[EX].nBeginCycle << "," << pInstr->aTiming[EX].nDuration << "} ";
        cout << "WB{" << pInstr->aTiming[WB].nBeginCycle << "," << pInstr->aTiming[WB].nDuration << "}" << endl;
    }

    // Print configuration
    cout << "CONFIGURATION" << endl;
    cout << " superscalar bandwidth (N)      = " << nN << endl;
    cout << " dispatch queue size (2*N)      = " << 2 * nN << endl;
    cout << " schedule queue size (S)        = " << nS << endl;

    // Print results
    cout << "RESULTS" << endl;
    cout << " number of instructions = " << nTotalInstructions << endl;
    cout << " number of cycles       = " << nCycle << endl;

    // IPC
    double dIPC = (double)nTotalInstructions / nCycle;
    cout << " IPC                    = " << fixed << setprecision(2) << dIPC << endl;
}


int main(int argc, char* argv[]) 
{
    if (argc != 4) 
    {
        cerr << "Usage: ./ooosim <N> <S> <trace_file>" << endl;
        return 1;
    }


    nN = atoi(argv[1]);
    nS = atoi(argv[2]);
    strTraceFileName = argv[3];

    // Open trace file
    ifstream fileTrace(strTraceFileName.c_str());
    if (!fileTrace.is_open()) 
    {
        cerr << "Error: Cannot open trace file " << strTraceFileName << endl;
        return 1;
    }

    // simulation loop
    do {
        
        Retire_Instruction();
        
        Execute_Instruction();
        
        Issue_Instruction();
        
        Dispatch_Instruction();
        
        Fetch_Instruction(fileTrace);

    } while (AdvanceCycle());

    PrintResults();

    // Clean up dynamically allocated memory
    Uninitialize();

    fileTrace.close();
    
    return 0;
}


void Uninitialize() 
{

    for (auto pInstr : aROB) 
    {
        delete pInstr;
    }
}
