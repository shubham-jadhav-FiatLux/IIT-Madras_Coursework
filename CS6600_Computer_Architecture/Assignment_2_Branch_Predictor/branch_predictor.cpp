// CS6600 : Computer Architecture
// Assignment : Branch Predictor Simulator
// Author : Shubham Balaso Jadhav
// Date : 15 Oct, 2024.

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>        // for using transform
#include <cctype>           // for using toupper
#include <string>
#include<bits/stdc++.h>

class BranchPredictor {
protected:
    std::vector<uint8_t> predictionTable;
    uint32_t m;                             // number of bits used for indexing
    uint32_t n;                             // number of bits in global history
    uint32_t globalHistory;                 // global history register
    uint32_t numBranches;
    uint32_t mispredictions;

public:
    // Constructor
    BranchPredictor(uint32_t m, uint32_t n) : m(m), n(n), globalHistory(0), numBranches(0), mispredictions(0) {
        uint32_t tableSize = 1 << m;            // 2^m
        predictionTable.resize(tableSize, 2);   // initialize to "weakly taken"
    }

    virtual void predict(uint32_t pc, char outcome) = 0;

    // Print Information
    void report() {

        std::cout << "OUTPUT" << std::endl;
        std::cout << "\tnumber of predictions: \t\t" << numBranches << std::endl;
        std::cout << "\tnumber of mispredictions: \t" << mispredictions << std::endl;
        std::cout << "\tmisprediction rate: \t\t" << std::fixed << std::setprecision(2) << static_cast<double>(mispredictions) / numBranches * 100.0 << "%" << std::endl;
    }

    // Print final contents of the prediction table
    void print(std::string type) {

        std::transform(type.begin(), type.end(), type.begin(),::toupper);
        std::cout << "FINAL " <<type<< " CONTENTS"<< std::endl;
        for (size_t i = 0; i < predictionTable.size(); ++i) {
            std::cout << "\t" << i << "\t\t" << static_cast<int>(predictionTable[i]) << std::endl;
        }
    }
};

class BimodalPredictor : public BranchPredictor {
public:
    BimodalPredictor(uint32_t m) : BranchPredictor(m, 0) {}

    void predict(uint32_t pc, char outcome) override {
        numBranches++;

        uint32_t index = (pc >> 2) & ((1 << m) - 1);
        uint8_t &counter = predictionTable[index];

        // Make prediction
        bool prediction = (counter >= 2);

        switch (outcome){
                case 't':
                        if(counter < 3) {
                            counter++;
                        }
                        if(prediction == 0) {
                            mispredictions++;
                        }
                        break;

                case 'n':
                        if(counter > 0) {
                            counter--;
                        }
                        if(prediction == 1) {
                            mispredictions++;
                        }
                        break; 
        }
    }
};

class GsharePredictor : public BranchPredictor {
public:
    GsharePredictor(uint32_t m, uint32_t n) : BranchPredictor(m, n) {}

    void predict(uint32_t pc, char outcome) override {
        
        numBranches++;
    
        uint32_t index = (pc >> 2) & ((1 << m) - 1);
        index = (index ^ (globalHistory << (m-n)));
        uint8_t &counter = predictionTable[index];

        // Make prediction
        bool prediction = (counter >= 2);

        switch (outcome){
                case 't':
                        if(counter < 3) {
                            counter++;
                        }
                        if(prediction == 0) {
                            mispredictions++;
                        }
                        break;

                case 'n':
                        if(counter > 0) {
                            counter--;
                        }
                        if(prediction == 1) {
                            mispredictions++;
                        }
                        break; 
        }

        // Update global history register
        if (outcome=='t'){
            globalHistory = (globalHistory >> 1) | (1<<(n-1));
        }
        else {
            globalHistory = (globalHistory >> 1);
        }
        globalHistory =globalHistory%(1<<n);   
    }   
};

int main(int argc, char *argv[]) {

    // Command Line Argument
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <type> <M> <N> <trace_file>\n";
        return 1;
    }
    std::string type = argv[1];
    uint32_t m = std::stoi(argv[2]);
    uint32_t n = (type == "bimodal") ? 0 : std::stoi(argv[3]);
    std::string traceFile = (type == "bimodal") ? argv[3] : argv[4];


    // Create Objecct
    BranchPredictor *predictor;

    if (type == "bimodal") {
        predictor = new BimodalPredictor(m);        // Bimodal Predictor
    } else if (type == "gshare") {
        predictor = new GsharePredictor(m, n);      // Gshare Predictor
    } else {
        std::cerr << "Unknown predictor type: " << type << std::endl;
        return 1;
    }


    // Parse Trace File
    std::ifstream infile(traceFile);
    if (!infile) {
        std::cerr << "Error opening trace file: " << traceFile << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string pcHex, outcome;
        if (!(iss >> pcHex >> outcome)) break;

        uint32_t pc = std::stoul(pcHex, nullptr, 16);
        predictor->predict(pc, outcome[0]);
    }


    // Printing Desired Output
    std::cout << "COMMAND" << std::endl;
    if (type == "bimodal") {
        std::cout << "\t" << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << std::endl;
    } else if (type == "gshare") {
        std::cout << "\t" << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4] << std::endl;
    }
    
    predictor->report();
    predictor->print(type);
    

    // Uninitialization
    delete predictor;

    return 0;
}
