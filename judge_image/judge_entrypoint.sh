#!/bin/bash
# judge_entrypoint.sh — 判题入口脚本
# stdin: JSON { "code": "...", "time_limit_ms": 1000, "memory_limit_kb": 262144,
#                "testcases": [{"input":"...", "expected":"..."}, ...] }

set -e

read -r INPUT_JSON

CODE=$(echo "$INPUT_JSON" | python3 -c "import sys,json; print(json.load(sys.stdin)['code'])")
TIME_LIMIT_MS=$(echo "$INPUT_JSON" | python3 -c "import sys,json; print(json.load(sys.stdin)['time_limit_ms'])")
MEM_LIMIT_KB=$(echo "$INPUT_JSON" | python3 -c "import sys,json; print(json.load(sys.stdin)['memory_limit_kb'])")
TESTCASES=$(echo "$INPUT_JSON" | python3 -c "import sys,json; print(json.dumps(json.load(sys.stdin)['testcases']))")

TIME_LIMIT_SECS=$(( (TIME_LIMIT_MS + 999) / 1000 ))
if [ "$TIME_LIMIT_SECS" -lt 1 ]; then
    TIME_LIMIT_SECS=1
fi

echo "$CODE" > /tmp/judge/solution.cpp

if ! g++ -O2 -std=c++17 /tmp/judge/solution.cpp -o /tmp/judge/solution 2>/tmp/judge/compile_error.txt; then
    ERROR_MSG=$(python3 -c "import json,sys; print(json.dumps(open('/tmp/judge/compile_error.txt').read()))")
    echo "{\"status\":\"compilation_error\",\"error\":$ERROR_MSG,\"time_used_ms\":0,\"memory_used_kb\":0}"
    exit 0
fi

TOTAL_TIME=0
MAX_MEMORY=0
FINAL_STATUS="accepted"
RESULTS="[]"

TESTCASE_COUNT=$(echo "$TESTCASES" | python3 -c "import sys,json; print(len(json.load(sys.stdin)))")

for i in $(seq 0 $((TESTCASE_COUNT - 1))); do
    TC_INPUT=$(echo "$TESTCASES" | python3 -c "import sys,json; print(json.load(sys.stdin)[$i]['input'])")
    TC_EXPECTED=$(echo "$TESTCASES" | python3 -c "import sys,json; print(json.load(sys.stdin)[$i]['expected'])")

    echo "$TC_INPUT" > /tmp/judge/input.txt

    START_TIME=$(date +%s%3N)

    timeout ${TIME_LIMIT_SECS}s bash -c "
        ulimit -v $((MEM_LIMIT_KB * 2)) && \
        /tmp/judge/solution < /tmp/judge/input.txt > /tmp/judge/output.txt 2>/tmp/judge/stderr.txt
    "
    EXIT_CODE=$?

    END_TIME=$(date +%s%3N)
    ELAPSED=$((END_TIME - START_TIME))
    TOTAL_TIME=$((TOTAL_TIME + ELAPSED))

    TC_STATUS="accepted"
    TC_TIME=$ELAPSED
    TC_MEMORY=0

    # 获取内存使用 (通过 /usr/bin/time 或估算)
    if [ -f /tmp/judge/stderr.txt ]; then
        TC_MEMORY=$(wc -c < /tmp/judge/stderr.txt 2>/dev/null || echo 0)
    fi

    if [ $EXIT_CODE -eq 124 ] || [ $EXIT_CODE -eq 137 ]; then
        TC_STATUS="time_limit_exceeded"
        FINAL_STATUS="time_limit_exceeded"
    elif [ $EXIT_CODE -ne 0 ]; then
        TC_STATUS="runtime_error"
        FINAL_STATUS="runtime_error"
    elif [ -f /tmp/judge/output.txt ] && [ -f /tmp/judge/expected.txt ]; then
        echo "$TC_EXPECTED" > /tmp/judge/expected.txt

        diff_output=$(diff -w /tmp/judge/output.txt /tmp/judge/expected.txt 2>&1) || true
        if [ -z "$diff_output" ]; then
            TC_STATUS="accepted"
        else
            ACTUAL=$(python3 -c "import json; print(json.dumps(open('/tmp/judge/output.txt').read()))")
            EXPECTED=$(python3 -c "import json; print(json.dumps(open('/tmp/judge/expected.txt').read()))")

            # Check for presentation error (only whitespace differs)
            ACTUAL_STRIP=$(tr -d '[:space:]' < /tmp/judge/output.txt)
            EXPECTED_STRIP=$(tr -d '[:space:]' < /tmp/judge/expected.txt)
            if [ "$ACTUAL_STRIP" = "$EXPECTED_STRIP" ]; then
                TC_STATUS="presentation_error"
                if [ "$FINAL_STATUS" = "accepted" ] || [ "$FINAL_STATUS" = "presentation_error" ]; then
                    FINAL_STATUS="presentation_error"
                fi
            else
                TC_STATUS="wrong_answer"
                if [ "$FINAL_STATUS" = "accepted" ] || [ "$FINAL_STATUS" = "presentation_error" ]; then
                    FINAL_STATUS="wrong_answer"
                fi
            fi
        fi
    fi

    RESULT_JSON=$(python3 -c "
import json
print(json.dumps({
    'status': '$TC_STATUS',
    'time_used_ms': $TC_TIME,
    'memory_used_kb': $TC_MEMORY,
    'input': open('/tmp/judge/input.txt').read(),
    'expected': open('/tmp/judge/expected.txt').read() if '$TC_STATUS' != 'accepted' else '',
    'actual': open('/tmp/judge/output.txt').read() if '$TC_STATUS' != 'accepted' else ''
}))")

    if [ "$i" -eq 0 ]; then
        RESULTS="[$RESULT_JSON"
    else
        RESULTS="$RESULTS,$RESULT_JSON"
    fi
done

RESULTS="$RESULTS]"

python3 -c "
import json
print(json.dumps({
    'status': '$FINAL_STATUS',
    'time_used_ms': $TOTAL_TIME,
    'memory_used_kb': $MAX_MEMORY,
    'results': json.loads('''$RESULTS''')
}))"
