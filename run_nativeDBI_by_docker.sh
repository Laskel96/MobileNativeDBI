#!/bin/bash
entry="app-debug.apk"

if [ -f $entry ]; then
echo "$entry called"

runtime=0

echo "Extract info"

python get_info.py $entry > info

docker run --privileged -d --name mal_test test bash -c "while true; do sleep 1; done"

docker cp $entry mal_test:/$entry

docker cp info mal_test:/info

docker exec mal_test bash -c "adb start-server"

sleep 3s

echo "start emulator"

docker exec mal_test bash -c "emulator -avd test -selinux disabled -writable-system -no-audio -no-boot-anim -no-window -accel on -gpu off -skin 1440x2880 -memory 2024 -sdcard mySdcardFile.img -partition-size 2048 &"

sleep 20s

echo "Install apk"
docker exec mal_test bash -c "adb install /$entry"

sleep 10s

start=$(date +%s%N)

echo "run bootstrap"
docker exec mal_test bash -c "sh /valgrind/bootstrap_valgrind.sh"

timer=0
bump=""

while true; do
	if [ $timer -gt 600 ];then
		bump="bumped"
		break;
	fi
	comp=$(docker exec mal_test bash -c "tail -1 /log_status")
	if [[ "$comp" == *"Complete"* ]]; then
		echo "$entry fully runs"
		break;
	else
		sleep 3
		timer=$(($timer+3))
	fi
done

end=$(date +%s%N)

docker exec mal_test bash -c "adb pull /data/local/tmp/log/logfile ."

docker cp mal_test:/logfile .

log_filename="log/log_$entry"

mv logfile $log_filename

docker kill mal_test
docker rm $(docker ps -a -q)

runtime=$(((end-start)/1000000))
echo "$entry takes $runtime $bump" >> log/runtime
fi
