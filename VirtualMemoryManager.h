#ifndef VIRTUAL_MEM_MAN_H
#define VIRTUAL_MEM_MAN_H

#include <iostream>
using namespace std;

//Program constants and file paths
const int OFFSET_BITS = 8;
const int OFFSET_MASK = 0xFF;
const int TLB_SIZE = 16;
const int MAIN_SIZE = 256;
const int PAGE_TABLE_SIZE = 256;
const int MAX_INPUT_FILE = 2000;
const int FRAME_SIZE = 256;
const char* const OUTPUT_FILE_PATH = "vm_sim_output.txt";
const char* const BACKING_STORE_FILE_PATH = "BACKING_STORE";

//TLB entry struct
struct TLB_entry
{
	int pageNum;
	int frameNum;
	int last_used;
	TLB_entry()
	{
	}
};

//Functions
int ReadInputFile(int inputArray[], char* inputFile);
int GetOffset(int address);
int GetPageNumber(int address);
int GetPhysicalAddress(int frameNumber, int offset);
int SearchTLB(int pageNum, TLB_entry TLB[], int access_counter);
int SearchPageTable(int pageNum, int pageTable[]);
char AccessMainMemory(int frameNumber, int offset, char main[MAIN_SIZE][FRAME_SIZE]);
int HandlePageFault(int pageNumber, char main[MAIN_SIZE][FRAME_SIZE], TLB_entry tlb[], int pageTable[], string replace, int counter, int mainIndex);
void TLB_Replace(string strategy, TLB_entry tlb[], int pageTable[], int pageNumber, int counter);
void WriteOutput(int address, int phys_address, char value, string display_phys);
string Welcome(string& phys_add);
void TLB_FIFO(TLB_entry tlb[], TLB_entry new_entry);
void TLB_LRU(TLB_entry tlb[], TLB_entry new_entry, int counter);
void WriteStats(int pageFaults, int tlbFaults, int totalAddresses);

#endif
