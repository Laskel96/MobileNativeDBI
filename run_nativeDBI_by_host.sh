#!/bin/bash
#Edit : apk file name
entry="app-debug.apk"

if [ -f $entry ]; then
echo "$entry called"

if [ "$entry" == "run_nativeDBI_by_host.sh" -o "$entry" == "get_info.py" -o "$entry" == "info" ]; then
echo "skip $entry"
else

runtime=0

echo "Extract info"

python get_info.py $entry > info

echo "start emulator"

adb start-server

#Edit : Emulator name
emulator -avd Sample2 &
EMULATOR_PID=$!

sleep 30s

echo "Install apk"
adb install -r $entry

sleep 3s

start=$(date +%s%N)

echo "run bootstrap"
sh valgrind/bootstrap_valgrind.sh

timer=0
bump=""

while true; do
	if [ $timer -gt 600 ];then
		bump="bumped"
		break;
	fi
	comp=$(tail -1 ./log_status)
	if [[ "$comp" == *"Complete"* ]]; then
		echo "$entry fully runs"
		break;
	else
		sleep 3
		timer=$(($timer+3))
	fi
done

end=$(date +%s%N)

adb pull /data/local/tmp/log/logfile .

log_filename="log/log_$entry"

mv logfile $log_filename

kill -9 $EMULATOR_PID

runtime=$(((end-start)/1000000))
echo "$entry takes $runtime $bump" >> log/runtime
fi
fi
#done
