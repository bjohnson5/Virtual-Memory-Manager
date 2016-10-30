#include <iostream>
#include <fstream>
#include <string>
#include "VirtualMemoryManager.h"
using namespace std;

/*
Opens the input file and reads out the logical addresses and then stores them in an array
params: inputArray[] (the array the addresses will be stored in)
returns: the size of the array
*/
int ReadInputFile(int inputArray[], char* inputFile)
{
	ifstream instream;
	instream.open(inputFile);
	if (instream.fail())
	{
		cout << "Input file opening failed.\n";
	}

	int index = 0;
	string str = "";

	instream >> inputArray[index];
	while (!instream.eof())
	{
		index++;
		instream >> inputArray[index];
		if (inputArray[index] <= 0)
		{
			index--;
		}
	}

	instream.close();

	return index + 1;
}

/*
Extracts the offset value from the logical address
params: address (the logical address from which to extract the offset from)
returns: the offset value
*/
int GetOffset(int address)
{
	return address & OFFSET_MASK;
}

/*
Extracts the page number from the logical address
params: address (the logical address from which to extract the page number from)
returns: the page number
*/
int GetPageNumber(int address)
{
	return address >> OFFSET_BITS;
}

/*
Combines the physical address with the offset to get a physical address
params: frameNumber (the index of the frame in main memory), offset (the offset of the value in the frame)
returns: the physical address
*/
int GetPhysicalAddress(int frameNumber, int offset)
{
	return frameNumber << OFFSET_BITS | offset;
}

/*
Searches the tlb for a given page number
params: pageNum (the page number to search for), TLB[] (the tlb table to search), counter (used by the LRU algorithm)
returns: the frame number (if page not found it returns -1)
*/
int SearchTLB(int pageNum, TLB_entry TLB[], int counter)
{
	int frameNum = -1;

	for (int i = 0; i < TLB_SIZE; i++)
	{
		if (TLB[i].pageNum == pageNum)
		{
			frameNum = TLB[i].frameNum;
			TLB[i].last_used = counter;
		}
	}

	return frameNum;
}

/*
Uses the page number to go to the index of the page table array
params: pageNum (the page number to search for), pageTable[] (the page table to search)
returns: the frame number (if the frame number is not found in the page table 
than it is not in main memory ... page fault and it will return -1)
*/
int SearchPageTable(int pageNum, int pageTable[])
{
	int frameNum = pageTable[pageNum];

	return frameNum;
}

/*
Searches the main memory for a given frame number and offset
params: frameNumber (the frame number to search for), offset (the offset to search for), main[][] (the main memory)
returns: the value at that memory location
*/
char AccessMainMemory(int frameNumber, int offset, char main[MAIN_SIZE][FRAME_SIZE])
{
	return main[frameNumber][offset];
}

/*
If the frame is not in memory yet: access the frame in "Backing_Store", load frame into memory, update tlb and page table
params: pageNumber (the pageNumber to read in), main[][] (the main memory to load the frame into), tlb[] (the tlb table to update), 
pageTable (the page table to update), replace (the replacement strategy to use), counter (used by the LRU algorithm)
returns: the frame number that was just loaded into memory
*/
int HandlePageFault(int pageNumber, char main[MAIN_SIZE][FRAME_SIZE], TLB_entry tlb[], int pageTable[], string replace, int counter, int mainIndex)
{
	int frameNumber = mainIndex;

	//Open the simulated hard drive file "BACKING_STORE"
	FILE *disk = fopen(BACKING_STORE_FILE_PATH, "rb");
	if (disk == NULL)
	{
		cout << "Disk file opening failed.\n";
	}

	//Calculate number of bytes to seek to
	int seekNum = pageNumber * FRAME_SIZE;

	//Seek to the frame in the file
	fseek(disk, seekNum, SEEK_SET);

	//Read in the frame
	char frameBytes[FRAME_SIZE];
	size_t result;
	result = fread(frameBytes, 1, FRAME_SIZE, disk);

	//Assign the frame to the main memory 2d array
	for (int i = 0; i < FRAME_SIZE; i++)
	{
		main[frameNumber][i] = frameBytes[i];
	}

	//Update the page table
	pageTable[pageNumber] = frameNumber;

	//Update the tlb
	TLB_Replace(replace, tlb, pageTable, pageNumber, counter);

	//Close the disk file stream
	fclose(disk);

	return frameNumber;
}

/*
Creates a new tlb entry for the frame that was just loaded into memory and attempts to put it in the tlb table
params: strategy (LRU or FIFO), tlb[] (the tlb table to update), pageTable[] (the page table to update), 
pageNumber (the pageNumber of the new entry), counter (used by the LRU algorithm)
returns: void
*/
void TLB_Replace(string strategy, TLB_entry tlb[], int pageTable[], int pageNumber, int counter)
{
	bool still_looking = true;
	int index = 0;

	//Create a new TLB_entry
	TLB_entry new_entry = TLB_entry();
	new_entry.frameNum = pageTable[pageNumber];
	new_entry.pageNum = pageNumber;
	new_entry.last_used = counter;

	//Check the TLB table to see if there is an available spot, if there is than put the new entry in that spot
	while (still_looking && index < TLB_SIZE)
	{
		if (tlb[index].frameNum == -1 && tlb[index].pageNum == -1)
		{
			tlb[index] = new_entry;
			still_looking = false;
		}

		index++;
	}

	//Checked the whole TLB table and did not find an open spot so now choose a strategy to replace one of the entries
	if (still_looking == true)
	{
		if (strategy == "1")
		{
			//FIFO
			TLB_FIFO(tlb, new_entry);
		}
		else
		{
			//LRU
			TLB_LRU(tlb, new_entry, counter);
		}
	}
}

/*
Replaces one of the TLB entries with a new entry using a FIFO strategy
params: tlb[] (the tlb table to add the entry too), new_entry (the new entry to put in the table)
returns: void
*/
void TLB_FIFO(TLB_entry tlb[], TLB_entry new_entry)
{
	//Shift the tlb entries up getting rid of the first one put in (index:0)
	for (int i = 0; i < TLB_SIZE - 1; i++)
	{
		tlb[i] = tlb[i + 1];
	}

	//Put the new entry in the last position in the array
	tlb[TLB_SIZE - 1] = new_entry;
}

/*
Replaces one of the TLB entries with a new entry using a LRU strategy
params: tlb[] (the tlb table to add the entry too), new_entry (the new entry to put in the table), counter (used to determine which entry to replace
returns: void
*/
void TLB_LRU(TLB_entry tlb[], TLB_entry new_entry, int counter)
{
	//Find the entry that was used least recently
	int index = 0;
	int lru = counter - tlb[0].last_used;
	for (int i = 1; i < TLB_SIZE; i++)
	{
		if (counter - tlb[i].last_used > lru)
		{
			lru = counter - tlb[i].last_used;
			index = i;
		}
	}

	//Replace the lru entry with the new entry
	tlb[index] = new_entry;
}

/*
Writes the output value and the addresses to the screen and to the output file
params: address (the address to write), value (the value at that address), display_phys (determines whether to print the physical addresses or not
returns: void
*/
void WriteOutput(int virt_address, int phys_address, char value, string display_phys)
{
	if (display_phys == "yes")
	{
		//Write to the screen
		cout << "Virtual Address: " << virt_address << "; " << "Physical Address: " << phys_address << "; " << "Value: " << (int)value << "\n";

		//Write to file
		ofstream outstream;
		outstream.open(OUTPUT_FILE_PATH, fstream::app);
		if (outstream.fail())
		{
			cout << "Output file opening failed.\n";
		}

		outstream << "Virtual Address: " << virt_address << "; " << "Physical Address: " << phys_address << "; " << "Value: " << (int)value << "\n";
      
		outstream.close();
	}
	else
	{
		//Write to the screen
		cout << "Virtual Address: " << virt_address << "; " << "Value: " << (int)value << "\n";

		//Write to file
		ofstream outstream;
		outstream.open(OUTPUT_FILE_PATH, fstream::app);
		if (outstream.fail())
		{
			cout << "Output file opening failed.\n";
		}

		outstream << "Virtual Address: " << virt_address << "; " << "Value: " << (int)value << "\n";
      
		outstream.close();
	}
}

/*
Calculates the tlb hit rate and the page fault rate and writes them to the screen and file
params: pageFaults (the number of times a page fault occured), tlbFaults (the number of times a tlb fault occured), 
totalAddresses (the total number of addresses processed)
return: void
*/
void WriteStats(int pageFaults, int tlbFaults, int totalAddresses)
{
	//Calculate rates
	double pageFaultRate = ((double) pageFaults / totalAddresses);
	double tlbHits = (totalAddresses - tlbFaults);
	double tlbHitRate =  (tlbHits / totalAddresses);

	//Write to screen
	cout << "\nTotal TLB Hits: " << tlbHits << "\n"
		<< "Total Page Faults: " << pageFaults << "\n"
		<< "Page Fault Rate: " << pageFaultRate << " %\n"
		<< "TLB Hit Rate: " << tlbHitRate << " %\n\n"
		<< "Check the results in the output file: " << OUTPUT_FILE_PATH;

	//Write to file
	ofstream outstream;
	outstream.open(OUTPUT_FILE_PATH, fstream::app);
	if (outstream.fail())
	{
		cout << "Output file opening failed.\n";
	}

	outstream << "\nTotal TLB Hits: " << tlbHits << "\n" 
			  << "Total Page Faults: " << pageFaults << "\n"
			  << "Page Fault Rate: " << pageFaultRate << " %\n"
			  << "TLB Hit Rate: " << tlbHitRate << " %\n";

	outstream.close();
}

/*
Prints the welcome message and displays the configuration of the constants
params: phys_add (allows the user to select to display the physical addresses or not)
returns: the TLB replacement strategy that the user selects
*/
string Welcome(string& phys_add)
{
	//Print welcome message and constants
	cout << "Welcome to Group 16's VM Simulator Version 1.0" << "\n\n" 
		 << "Number of logical pages: " << PAGE_TABLE_SIZE 
		 << "\n" << "Page size: " << FRAME_SIZE 
		 << "\n" << "Page Table size: " << PAGE_TABLE_SIZE 
		 << "\n" << "TLB size: " << TLB_SIZE 
		 << "\n" << "Number of physical frames: " << MAIN_SIZE 
		 << "\n" << "Physical memory size: " << MAIN_SIZE * FRAME_SIZE 
		 << "\n\n";

	//Determine if the user wants to display physical addresses or not
	do
	{
		cout << "Display Physical Addresses? [yes or no] ";
		cin >> phys_add;
	}	while (phys_add != "yes" && phys_add != "no");

	//Determine what replacement strategy the user would like to use
	string replace = "";
	do
	{
		cout << "Choose TLB Replacement Strategy [1: FIFO, 2: LRU] ";
		cin >> replace;
	} while (replace != "1" && replace != "2");

	return replace;
}

