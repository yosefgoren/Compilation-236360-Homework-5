for i in $(seq 4 23); do
	printf "\nvoid main(){int x;}\n" >> t${i}.in
done
