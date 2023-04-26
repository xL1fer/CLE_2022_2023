array=()

for i in `seq 5`; do array[$i]=$(mpiexec -n 2 ./countWords text0.txt text1.txt text2.txt text3.txt text4.txt); done;
./parseTimes ${array[1]} ${array[2]} ${array[3]} ${array[4]} ${array[5]}
for i in `seq 5`; do array[$i]=$(mpiexec -n 3 ./countWords text0.txt text1.txt text2.txt text3.txt text4.txt); done;
./parseTimes ${array[1]} ${array[2]} ${array[3]} ${array[4]} ${array[5]}
for i in `seq 5`; do array[$i]=$(mpiexec -n 5 ./countWords text0.txt text1.txt text2.txt text3.txt text4.txt); done;
./parseTimes ${array[1]} ${array[2]} ${array[3]} ${array[4]} ${array[5]}
for i in `seq 5`; do array[$i]=$(mpiexec -n 9 ./countWords text0.txt text1.txt text2.txt text3.txt text4.txt); done;
./parseTimes ${array[1]} ${array[2]} ${array[3]} ${array[4]} ${array[5]}