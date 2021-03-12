#!/usr/bin/env bash

PACKAGE=$(cat info | cut -f2 -d:)
ACTIVITY=$(cat info | cut -f3 -d:)

echo "Pakage : $PACKAGE"
echo "ACTIVITY : $ACTIVITY"
echo $PACKAGE > valgrind/package_name

echo "Get oat from device"

oatfile="/data/dalvik-cache/x86/data@app@$PACKAGE-1@base.apk@classes.dex"
adb shell oatdump --oat-file=$oatfile  > valgrind/addition/DB/dumpoat
fileSize=$(wc -c valgrind/addition/DB/dumpoat | awk '{print $1}')
if [fileSize -lt 3000]
then
	oatfile="/data/dalvik-cache/x86/data@app@$PACKAGE-2@base.apk@classes.dex"
	adb shell oatdump --oat-file=$oatfile  > valgrind/addition/DB/dumpoat
fi

sleep 1s 

echo "Parsing oat..."
python valgrind/addition/DB/extract_symbols.py valgrind/addition/DB/dumpoat

sleep 1s

echo "Push DB data to device"
adb push valgrind/addition/DB/DB_method /data/local/tmp
adb push valgrind/addition/DB/DB_String /data/local/tmp

sleep 1s
echo "Push signature to device"
adb push valgrind/addition/DB/signature_string /data/local/tmp

sleep 1s
echo "Push offset and Pakage Name to device"
adb push valgrind/addition/DB/DB_offset /data/local/tmp
adb push valgrind/package_name /data/local/tmp

sleep 1s
echo "Push DBI to device"
adb push valgrind/Inst/data /

echo "Push wrapper to device"
adb push valgrind/start_valgrind.sh /data/local/tmp
adb shell chmod 777 /data/local/tmp/start_valgrind.sh 

adb root
adb shell setenforce 0
adb shell setprop wrap.$PACKAGE "'logwrapper /data/local/tmp/start_valgrind.sh'"

echo "wrap.$PACKAGE: $(adb shell getprop wrap.$PACKAGE)"

adb logcat -c
adb logcat -c

echo "kill $PACKAGE"
adb shell am force-stop $PACKAGE

sleep 3s
echo "run $PACKAGE"
adb shell am start -W -a android.intent.action.MAIN -n $PACKAGE/$ACTIVITY > log_status

exit 0
