
timer_start=`date "+%Y-%m-%d %H:%M:%S"`
start=`date +%s`

python3 src/step1-generateAllRawLog.py
python3 src/step2-summaryRawLog.py
python3 src/step3-plotResult.py


timer_end=`date "+%Y-%m-%d %H:%M:%S"`
end=`date +%s`
time=$(( $end - $start ))
echo "----------This test is: base_src----------"
echo "Begin at: ${timer_start}"
echo "Finish at: ${timer_end}"
echo "Runtime: $(($time / 3600)) hours"