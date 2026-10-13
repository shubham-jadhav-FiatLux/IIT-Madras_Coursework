#!/usr/bin/env bash

# Build the analysis
./build.sh || { echo "Building analysis failed. Exiting."; exit; }

if [ ! -d "testcases" ]
then
    echo "testcases directory not found. Make sure you are running this from the same folder as the script"
    exit
fi

# Run testcases
passed=0

##########################################################################################
# Function to check if no instrumentation have been performed for the given testcase
##########################################################################################
check_no_instrumentation () {
    testcase=$1
    echo "Running $testcase"
    opt -load-pass-plugin ./build/arrayInstrumentation.so -passes='instrument-array-accesses' -S $testcase > $testcase.out
    # The ModuleID line and sometimes the preds differ, even when no instrumentation are performed
    cat $testcase | sed '/; ModuleID/d; s/; preds.*//' > test1.ll
    cat $testcase.out | sed '/; ModuleID/d; s/; preds.*//' > test2.ll
    if [ `diff test1.ll test2.ll | wc -l` -eq 0 ]
    then
        passed=$((passed+1))
        echo -e "[\033[1;32mPASS\033[0m] Testcase \033[1m$testcase\033[0m passed. No instrumentation found"
    else
        echo -e "[\033[1;31mFAIL\033[0m] Testcase \033[1m$testcase\033[0m failed. Expected no instrumentation, found change in IR file"
    fi
    rm test1.ll test2.ll
}

##########################################################################################
# Function to check if instrumentation is working correctly
# Executes the output LLVM IR file using lli
#   - with the arguments as command line parameters
#   - checks if the the output is same as the expected output specified
##########################################################################################
run_lli_and_verify () {
    testcase=$1
    llfile=$2
    arg1=$3
    arg2=$4
    expected_output=$5
    if [ `lli $2 $3 $4 | grep "^$5$" | wc -l` -ne 1 ]
    then
        echo -e "[\033[1;31mFAIL\033[0m] Testcase \033[1m$testcase\033[0m failed. Incorrect instrumentation"
        echo "Error running lli $2 $3 $4. Expected $5. Found :"
        lli $2 $3 $4
        return 0
    fi
    return 1
}

check_instrumentation () {
    testcase=$1
    echo "Running $testcase"
    opt -load-pass-plugin ./build/arrayInstrumentation.so -passes='instrument-array-accesses' -S $testcase > $testcase.out
    if run_lli_and_verify $testcase $testcase.out $2 $3 $4; then
        return
    fi
    if run_lli_and_verify $testcase $testcase.out $5 $6 $7; then
        return
    fi
    if run_lli_and_verify $testcase $testcase.out $8 $9 ${10}; then
        return
    fi
    echo -e "[\033[1;32mPASS\033[0m] Testcase \033[1m$testcase\033[0m passed. Correct instrumentation found"
    passed=$((passed+1))
}


##########################################################################################
# List of testcases and what is tested in each testcase
##########################################################################################

check_no_instrumentation testcases/testcase1.ll
check_instrumentation testcases/testcase2.ll 2 20 0 100 20 -1 -2 20 -1
check_no_instrumentation testcases/testcase3.ll
check_instrumentation testcases/testcase4.ll 2 20 0 10 20 -1 200 20 -1
check_no_instrumentation testcases/testcase5.ll
check_instrumentation testcases/testcase6.ll -85 20 0 -101 20 -1 -80 20 -1
check_no_instrumentation testcases/testcase7.ll
check_instrumentation testcases/testcase8.ll 2 20 0 10 20 -1 200 20 -1
check_no_instrumentation testcases/testcase9.ll
check_instrumentation testcases/testcase10.ll 20 20 0 101 20 -1 200 20 -1

echo -e "\n\n>> Testcases passed: $passed"
