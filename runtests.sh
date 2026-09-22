#!/usr/bin/env bash

COMPILER="./main.exe"
ASM_FILE="program.s"
PROGRAM="program"

PASSED=0
FAILED=0

run_test() {
    NAME="$1"
    FILE="$2"
    EXPECTED="$3"

    echo "========================================"
    echo "TEST: $NAME"

    rm -f "$ASM_FILE" "$PROGRAM"

    #run the jkc compiler
    "$COMPILER" "$FILE" > /tmp/jkc_compile.log 2>&1
    COMPILER_STATUS=$?

    if [ $COMPILER_STATUS -ne 0 ]; then
        echo "FAIL - compiler rejected valid program"
        cat /tmp/jkc_compile.log
        FAILED=$((FAILED + 1))
        return
    fi

    if [ ! -f "$ASM_FILE" ]; then
        echo "FAIL - program.s was not generated"
        FAILED=$((FAILED + 1))
        return
    fi

    #assemble/link the risc v generated code
    riscv64-linux-gnu-gcc -static "$ASM_FILE" -o "$PROGRAM" \
        > /tmp/jkc_link.log 2>&1

    LINK_STATUS=$?

    if [ $LINK_STATUS -ne 0 ]; then
        echo "FAIL - generated assembly did not assemble/link"
        cat /tmp/jkc_link.log
        FAILED=$((FAILED + 1))
        return
    fi

    #run qemu
    qemu-riscv64 "./$PROGRAM" > /tmp/jkc_run.log 2>&1
    ACTUAL=$?

    if [ "$ACTUAL" -eq "$EXPECTED" ]; then
        echo "PASS - expected $EXPECTED, got $ACTUAL"
        PASSED=$((PASSED + 1))
    else
        echo "FAIL - expected $EXPECTED, got $ACTUAL"
        FAILED=$((FAILED + 1))
    fi
}


run_error_test() {
    NAME="$1"
    FILE="$2"

    echo "========================================"
    echo "TEST: $NAME"

    rm -f "$ASM_FILE" "$PROGRAM"

    "$COMPILER" "$FILE" > /tmp/jkc_compile.log 2>&1
    STATUS=$?

    if [ $STATUS -ne 0 ]; then
        echo "PASS - compiler correctly rejected program"
        PASSED=$((PASSED + 1))
    else
        echo "FAIL - compiler accepted an invalid program"
        FAILED=$((FAILED + 1))
    fi
}


run_test "Arithmetic"  "examples/arithmetic.jkc" 15
run_test "If / Else"   "examples/if_else.jkc" 25
run_test "While Loop"  "examples/while_loop.jkc" 55
run_test "Factorial"   "examples/factorial.jkc" 120
run_test "Spilling"    "examples/spilling.jkc" 166

run_error_test "Type Error" "examples/type_error.jkc"


echo
echo "<======================================>"
echo "TEST SUMMARY"
echo "<======================================>"
echo "Passed: $PASSED"
echo "Failed: $FAILED"
echo

if [ "$FAILED" -eq 0 ]; then
    echo "ALL TESTS PASSED"
    exit 0
else
    echo "SOME TESTS FAILED"
    exit 1
fi