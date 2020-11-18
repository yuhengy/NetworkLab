
#---------------------config begin---------------------
set -v
parallel=8

runHome=${PWD}


maxQueueSize_all=(20 50 100 200 500 1000)
#maxQueueSize_all=(50)
rawResultsDict_all=(${maxQueueSize_all[@]/#/rawResults/qlen-})
echo "rawResultsDict_all: ${rawResultsDict_all[@]}"
rawReultsCwnd_all=(${rawResultsDict_all[@]/%/\/cwnd.txt})
echo "rawReultsCwnd_all: ${rawReultsCwnd_all[@]}"
rawReultsQueueLen_all=(${rawResultsDict_all[@]/%/\/qlen.txt})
echo "rawReultsQueueLen_all: ${rawReultsQueueLen_all[@]}"
rawReultsRtt_all=(${rawResultsDict_all[@]/%/\/rtt.txt})
echo "rawReultsRtt_all: ${rawReultsRtt_all[@]}"

algo_all=(taildrop red codel)
STEP2rawResultsDict_all=(${algo_all[@]/#/rawResults/})
STEP2rawResults_all=(${STEP2rawResultsDict_all[@]/%/\/rtt.txt})
echo "STEP2rawResults_all: ${STEP2rawResults_all[@]}"

#---------------------config end---------------------

#---------------------parallel control begin---------------------
[ -e ~/fd1 ] || mkfifo ~/fd1
exec 3<> ~/fd1
for((i=0; i<${parallel}; i++)); do
  echo >&3
done
#---------------------parallel control end---------------------
rm -rf rawResults plots
mkdir -p rawResults plots plots
# *****************************************************
  # Step 1 reproduce bufferbloat: a. generate raw results
# *****************************************************
cd rawResults
for maxQueueSize in ${maxQueueSize_all[@]}; do
  read -u3
  {
    sudo python ${runHome}/mininet/reproduce_bufferbloat.py --maxq ${maxQueueSize}
    echo >&3
  } &
  done
wait
cd ..
# *****************************************************
# Step 1 reproduce bufferbloat: b. plot raw results
# *****************************************************
python3 ${runHome}/plotScripts/plot_STEP1cwnd.py ${rawReultsCwnd_all[@]} ${maxQueueSize_all[@]}
python3 ${runHome}/plotScripts/plot_STEP1queueLen.py ${rawReultsQueueLen_all[@]} ${maxQueueSize_all[@]}
python3 ${runHome}/plotScripts/plot_STEP1rtt.py ${rawReultsRtt_all[@]} ${maxQueueSize_all[@]}


# *****************************************************
# Step 2 mitigate bufferbloat: a. generate raw results
# *****************************************************
cd rawResults
for algo in ${algo_all[@]}; do
  read -u3
  {
    sudo python ${runHome}/mininet/mitigate_bufferbloat.py --algo ${algo}
    echo >&3
  } &
done
wait
cd ..
# *****************************************************
# Step 2 mitigate bufferbloat: b. plot raw results
# *****************************************************
python3 ${runHome}/plotScripts/plot_STEP2rtt.py ${STEP2rawResults_all[@]} ${algo_all[@]}




#---------------------parallel control begin---------------------
for((i=0; i<${parallel}; i++)); do
  read -u3
done

exec 3<&-
exec 3>&-
rm -rf ~/fd1
#---------------------parallel control end---------------------
