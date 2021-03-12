#!/system/bin/sh

PACKAGE=$(cat /data/local/tmp/package_name)
VGPARAMS='--error-limit=no --sigill-diagnostics=no --tool=none --main-stacksize=64 --log-file=/data/local/tmp/log/logfile'
export TMPDIR=/data/data/$PACKAGE

exec /data/local/tmp/Inst/bin/valgrind $VGPARAMS $* 
