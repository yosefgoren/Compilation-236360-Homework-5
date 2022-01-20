# defaults:
DEAFULT_TESTS_DIR='stress'
DEFAULT_MIN_TEST='1'
DEFAULT_MAX_TEST='19'
DEAFULT_EXE_PATH='../hw3'
EXIT_ON_FIRST_FAIL='1'

#call check range of tests:
if [ -z $1 ]; then
	rm files/t*.res
	if [ -z $1 ]; then
		MIN_TEST=$DEFAULT_MIN_TEST
	else
		MIN_TEST=$1
	fi
	if [ -z $2 ]; then
		MAX_TEST=$DEFAULT_MAX_TEST
	else
		MAX_TEST=$2
	fi

	for i in $(seq $MIN_TEST $MAX_TEST)
	do 
		./check.sh $i
		if [ $? -eq 1 ]; then
			if [ $EXIT_ON_FIRST_FAIL -eq 1 ]; then
				exit 1
			fi
		fi
	done
else
#check single test:
	RED='\033[0;31m'
	GREEN='\033[0;32m'
	BLUE='\033[0;34m'
	NC='\033[0m'

	TEST=$DEAFULT_TESTS_DIR/t$1
	
	if [ -z $2 ]; then
		EXE=$DEAFULT_EXE_PATH
	else
		EXE=$2
	fi
	if [ ! -f $EXE ]; then
		printf "${RED}Error: executable: '${EXE}'  -  not found! ${NC}\n"
		exit 1
	fi

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
