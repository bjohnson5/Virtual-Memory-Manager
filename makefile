CC = g++

all: vm_sim
vmm.o: VirtualMemoryManager.cpp
	$(CC) -o vmm.o -c VirtualMemoryManager.cpp
vmm_functions.o: VMM_Functions.cpp
	$(CC) -o vmm_functions.o -c VMM_Functions.cpp
vm_sim: vmm.o vmm_functions.o
	$(CC) -o vm_sim vmm.o vmm_functions.o -lm
clean:
	\rm *.o