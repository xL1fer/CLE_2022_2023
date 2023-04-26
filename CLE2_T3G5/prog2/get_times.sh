array=()

printf "\ndatSeq32.bin"

for i in `seq 5`; do array[$i]=$(mpiexec -n 1 ./sortingSequence datSeq32.bin); done;
./parseTimes ${array[1]} ${array[2]} ${array[3]} ${array[4]} ${array[5]}
for i in `seq 5`; do array[$i]=$(mpiexec -n 2 ./sortingSequence datSeq32.bin); done;
./parseTimes ${array[1]} ${array[2]} ${array[3]} ${array[4]} ${array[5]}
for i in `seq 5`; do array[$i]=$(mpiexec -n 4 ./sortingSequence datSeq32.bin); done;
./parseTimes ${array[1]} ${array[2]} ${array[3]} ${array[4]} ${array[5]}
for i in `seq 5`; do array[$i]=$(mpiexec -n 8 ./sortingSequence datSeq32.bin); done;
./parseTimes ${array[1]} ${array[2]} ${array[3]} ${array[4]} ${array[5]}

printf "\ndatSeq256K.bin"

for i in `seq 5`; do array[$i]=$(mpiexec -n 1 ./sortingSequence datSeq256K.bin); done;
./parseTimes ${array[1]} ${array[2]} ${array[3]} ${array[4]} ${array[5]}
for i in `seq 5`; do array[$i]=$(mpiexec -n 2 ./sortingSequence datSeq256K.bin); done;
./parseTimes ${array[1]} ${array[2]} ${array[3]} ${array[4]} ${array[5]}
for i in `seq 5`; do array[$i]=$(mpiexec -n 4 ./sortingSequence datSeq256K.bin); done;
./parseTimes ${array[1]} ${array[2]} ${array[3]} ${array[4]} ${array[5]}
#for i in `seq 5`; do array[$i]=$(mpiexec -n 8 ./sortingSequence datSeq256K.bin); done;
#./parseTimes ${array[1]} ${array[2]} ${array[3]} ${array[4]} ${array[5]}

printf "\ndatSeq1M.bin"

for i in `seq 5`; do array[$i]=$(mpiexec -n 1 ./sortingSequence datSeq1M.bin); done;
./parseTimes ${array[1]} ${array[2]} ${array[3]} ${array[4]} ${array[5]}
for i in `seq 5`; do array[$i]=$(mpiexec -n 2 ./sortingSequence datSeq1M.bin); done;
./parseTimes ${array[1]} ${array[2]} ${array[3]} ${array[4]} ${array[5]}
for i in `seq 5`; do array[$i]=$(mpiexec -n 4 ./sortingSequence datSeq1M.bin); done;
./parseTimes ${array[1]} ${array[2]} ${array[3]} ${array[4]} ${array[5]}
#for i in `seq 5`; do array[$i]=$(mpiexec -n 8 ./sortingSequence datSeq1M.bin); done;
#./parseTimes ${array[1]} ${array[2]} ${array[3]} ${array[4]} ${array[5]}

printf "\ndatSeq16M.bin"

for i in `seq 5`; do array[$i]=$(mpiexec -n 1 ./sortingSequence datSeq16M.bin); done;
./parseTimes ${array[1]} ${array[2]} ${array[3]} ${array[4]} ${array[5]}
for i in `seq 5`; do array[$i]=$(mpiexec -n 2 ./sortingSequence datSeq16M.bin); done;
./parseTimes ${array[1]} ${array[2]} ${array[3]} ${array[4]} ${array[5]}
for i in `seq 5`; do array[$i]=$(mpiexec -n 4 ./sortingSequence datSeq16M.bin); done;
./parseTimes ${array[1]} ${array[2]} ${array[3]} ${array[4]} ${array[5]}
#for i in `seq 5`; do array[$i]=$(mpiexec -n 8 ./sortingSequence datSeq16M.bin); done;
#./parseTimes ${array[1]} ${array[2]} ${array[3]} ${array[4]} ${array[5]}