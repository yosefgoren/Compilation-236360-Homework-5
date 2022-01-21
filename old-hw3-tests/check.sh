# defaults:
DEFAULT_MIN_TEST='1'
DEFAULT_MAX_TEST='26'
EXIT_ON_FIRST_FAIL='1'
DEFAULT_TESTS_DIR='ret2'
EXE='../hw5'

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

function check_test () {
	TEST=$TESTS_DIR/t$1
	if [ -f $TEST.res ]; then
		rm $TEST.res
	fi

	if [ ! -f $EXE ]; then
		printf "${RED}Error: executable: '${EXE}'  -  not found! ${NC}\n"
		exit 1
	fi

	if [ ! -f $TEST.in ]; then
		printf "$TEST: ${BLUE} NOT FOUND ${NC}\n"
	else
		$EXE > $TEST.res < $TEST.in
		diff $TEST.exp $TEST.res
		if [ $? -eq 0 ]; then
			printf "$TEST: ${GREEN} SUCCESS ${NC}\n"
		else
			printf "$TEST: ${RED} FAILURE ${NC}\n"
			printf "\t${BLUE}< expected but not found${NC}\n"
			printf "\t${BLUE}> found but not expected${NC}\n"
			exit 1
		fi
	fi
}


if [ -z $1 ]; then
	TESTS_DIR=$DEFAULT_TESTS_DIR
else
	TESTS_DIR=$1
fi
if [ ! -d $TESTS_DIR ]; then
	printf "${RED}error: no such directory: \"$1\"${NC}\n"
	exit 1
fi

NUM_REGEX="^[0-9]+$"
if [ -z $2 ]; then
	MIN_TEST=$DEFAULT_MIN_TEST
else
	MIN_TEST=$2
fi
if ! [[ $MIN_TEST =~ $NUM_REGEX ]]; then
	printf "${RED}error: min test should be a number but is: \"$MIN_TEST\"${NC}\n"
	exit 1
fi

if [ -z $3 ]; then
	MAX_TEST=$DEFAULT_MAX_TEST
else
	MAX_TEST=$3
fi
if ! [[ $MAX_TEST =~ $NUM_REGEX ]]; then
	printf "${RED}error: max test should be a number but is: \"$MAX_TEST\"${NC}\n"
	exit 1
fi

for i in $(seq $MIN_TEST $MAX_TEST)
do 
	check_test $i
	if [ $? -eq 1 ]; then
		if [ $EXIT_ON_FIRST_FAIL -eq 1 ]; then
			exit 1
		fi
	fi
done