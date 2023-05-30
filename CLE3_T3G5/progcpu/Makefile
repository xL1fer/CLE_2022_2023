CU_APPS=progcpu

all: ${CU_APPS}

%: %.cu
	gcc -Wall -O3 -o -o $@ $<
clean:
	rm -f ${CU_APPS}
