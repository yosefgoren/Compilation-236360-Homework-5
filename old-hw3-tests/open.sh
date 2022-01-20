TESTS_DIR_PATH='big'
if [ -z $1 ]; then
	echo no test number given
else
	code $TESTS_DIR_PATH/t$1.in $TESTS_DIR_PATH/t$1.exp
fi
