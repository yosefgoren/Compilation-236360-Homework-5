TESTS_DIR_PATH=$2
if [ -z $2 ]; then
	TESTS_DIR_PATH="yosnkos"
fi
if [ -z $1 ]; then
	echo no test number given
else
	code $TESTS_DIR_PATH/t$1.in $TESTS_DIR_PATH/t$1.exp
fi
