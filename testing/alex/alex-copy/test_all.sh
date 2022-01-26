make

if [[ ! -f "./hw5" ]]; then
    exit 1
fi

for f in `ls ./hw5-tests/*.in`; do
    code_file=`sed "s/\.in/.llvm/g" <<< "$f"`
	out_file=`sed "s/\.in/.out/g" <<< "$f"`	
	res_file=`sed "s/\.in/.res/g" <<< "$f"`

    ./hw5 < $f > $code_file

    lli < $code_file > $res_file
    # lli_return_code=$?
    
    echo "$f runned"

    # if [ $lli_return_code -eq 1 ]; then
    #     echo "$f has diff <---------------------------------------"
    #     continue
    # fi

    diff_res=$(diff $res_file $out_file)
    
    if [[ ! -z $diff_res ]]; then
        echo "$f has diff <---------------------------------------"
        # echo "./hw5 < ./hw5-tests/t${i}.in"
    else
        echo "$f passed"
    fi

done