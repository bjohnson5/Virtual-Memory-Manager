#include <iostream>
#include <fstream>
#include <string>
#include "VirtualMemoryManager.h"
using namespace std;

int main(int argc, char* argv[])
{
	//Local Variables
	int logicalAddresses[MAX_INPUT_FILE];
	int logicalAddresses_size;
	int physicalAddress;
	int pageNumber;
	int offset;
	int frameNumber;
	int outputValue;
	int access_counter = 0;
	char mainMemory[MAIN_SIZE][FRAME_SIZE];
	int mainMemory_counter;
	TLB_entry TLB[TLB_SIZE];
	int pageTable[PAGE_TABLE_SIZE];
	string replace_strategy = "";
	string display_phys_add = "";

	//Variables to keep up with the stats of the program
	int pageFaults = 0;
	int tlbFaults = 0;

	//Print welcome message
	replace_strategy = Welcome(display_phys_add);
   
   	//Fill PageTable with all -1's (this is how the program will detect which page table positions are full
	for (int i = 0; i < PAGE_TABLE_SIZE; i++)
	{
		pageTable[i] = -1;
	}

	//Fill TLB with all -1's (this is how the program will detect which tlb table positions are full
	TLB_entry tempEntry;
	for (int i = 0; i < TLB_SIZE; i++)
	{
		tempEntry = TLB_entry();
		tempEntry.frameNum = -1;
		tempEntry.pageNum = -1;
		TLB[i] = tempEntry;
	}

	//Initialize mainMemory_counter to keep track of the next available index
	mainMemory_counter = 0;

	//Load integers from file
	logicalAddresses_size = ReadInputFile(logicalAddresses, argv[1]);

	//For each logical address:
	for (int i = 0; i < logicalAddresses_size; i++)
	{
		pageNumber = GetPageNumber(logicalAddresses[i]);
		offset = GetOffset(logicalAddresses[i]);

		//Search TLB for the given page number (if not there then return -1)
		frameNumber = SearchTLB(pageNumber, TLB, i);

		//If page number is not in the TLB
		if (frameNumber == -1)
		{
			//Update tlb fault counter
			tlbFaults++;

			//Then search the page table
			frameNumber = SearchPageTable(pageNumber, pageTable);

			//If the frame is not in the page table that means that it is not in main memory so we have a page fault
			if (frameNumber == -1)
			{
				//Update the page fault counter
				pageFaults++;

				//Handle the page fault
				frameNumber = HandlePageFault(pageNumber, mainMemory, TLB, pageTable, replace_strategy, i, mainMemory_counter);
				mainMemory_counter++;
			}
			//The frame was found in the page table, so move that frame into the tlb
			else
			{
				TLB_Replace(replace_strategy, TLB, pageTable, pageNumber, i);
			}
		}

		//Once we have the frame number than use it to access the value at that position in main memory
		outputValue = AccessMainMemory(frameNumber, offset, mainMemory);

		//Get the physical address to print
		physicalAddress = GetPhysicalAddress(frameNumber, offset);

		//Print data to screen and file
		WriteOutput(logicalAddresses[i], physicalAddress, outputValue, display_phys_add);
	}

	WriteStats(pageFaults, tlbFaults, logicalAddresses_size);
}


