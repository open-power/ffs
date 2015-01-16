/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/src/memory_leak_detection.c $                            */
/*                                                                        */
/* OpenPOWER FFS Project                                                  */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2015                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

#define PROGRAM_VERSION  "0.0.0.1"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <execinfo.h>
#include "bb_trace.h"

/*--------------------------------------------------
 * Private Constants
 --------------------------------------------------*/

#define LEAK_SIGNATURE  8283471
#define SENTINEL_VALUE 0xD9B79573

/*--------------------------------------------------
 * Private Type definitions
 --------------------------------------------------*/

typedef unsigned int Sentinel_t;

struct _Memory_Leak_Data {
	unsigned int Signature;	/* Indentify this structure in memory */
	void *Mem_Address;	/* The address actually returned by malloc. */
	unsigned int User_Size;	/* The size of the allocation as requested by the user. */
	unsigned int Alignment;	/* The alignment requested by the user. */
	const char *Module_Name;	/* The name of the module containing the function which allocated this memory. */
	const char *Function_Name;	/* The name of the function which allocated this memory. */
	unsigned int Line_Number;	/* The line number of the MALLOC call in the function which allocated this memory. */
	struct _Memory_Leak_Data *Next;	/* The next allocated block of memory (if any). */
	struct _Memory_Leak_Data *Previous;	/* The previous allocated block of memory (if any). */
	Sentinel_t Starting_Sentinel;	/* The starting sentinel used to detect buffer overrun/memory overwrite errors. */
};

typedef struct _Memory_Leak_Data Memory_Leak_Data_t;

/*--------------------------------------------------
 Private global variables.
--------------------------------------------------*/

static pthread_mutex_t Memory_Leak_Lock = PTHREAD_MUTEX_INITIALIZER;	/* Mutex to serialize access to the chain of allocations. */
static Memory_Leak_Data_t *Memory_Chain = NULL;	/* The start of our chain of allocations. */
static unsigned int Memory_Chain_Entries = 0;	/* The number of items in our chain. */

/*--------------------------------------------------
 Local functions
--------------------------------------------------*/

void print_backtrace(void)
{
#define BT_SIZE  20
	void *bt[BT_SIZE];
	int nr = backtrace(&bt[0], BT_SIZE);

	fprintf(stderr, "========== backtrace ==========\n");
	backtrace_symbols_fd(bt, nr, fileno(stderr));
	fprintf(stderr, "========== backtrace ==========\n");

	return;
}

/*********************************************************************/
/*                                                                   */
/*   Function Name: Check_Leak_List                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output: If Success :                                            */
/*                                                                   */
/*           If Failure :                                            */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:                                                   */
/*                                                                   */
/*   Notes:                                                          */
/*                                                                   */
/*********************************************************************/
static int Check_Leak_List(const void *p)
{
	Memory_Leak_Data_t *Current_Item;	/* Used to walk the memory allocation chain. */
	Memory_Leak_Data_t *Previous_Item;	/* Used to check the memory allocation chain. */
	Sentinel_t *Ending_Sentinel;	/* To allow access to the sentinel at the end of the allocated memory. */
	unsigned int Data_Start;	/* The address of the first byte of the data area of the current allocation. */
	unsigned int Data_End;	/* The first byte after the data area of the current allocation. */
	unsigned int Address_To_Test;	/* The address we are looking for. */
	unsigned int Address_Found = 2;	/* Used to track whether the address was found in the allocation list.
					   If 2, then the address was not found.  If 0, then the address was found and
					   it is the start of the data area of an allocation.  If 1, then the address
					   was found and it lies within the data area of an allocation.                 */
	int rc = 0;		/* Holds the return code.  Assume success. */
	unsigned int Count = 0;

	FUNCTION_ENTRY()

	    PRINT_FUNCTION_PARAMETER("Parameters:\n")
	    PRINT_FUNCTION_PARAMETER("     p = %p\n", p)

	    USER3_PRINT_LINE("Memory_Chain = %p", Memory_Chain)
	    Current_Item = Memory_Chain;
	Previous_Item = NULL;

	if (p != NULL) {
		USER3_PRINT_LINE
		    ("Request: Check the memory allocation list for p = %p.\n",
		     p);
	} else {
		USER3_PRINT_LINE
		    ("Request: Check the integrity of the memory allocation list.\n");
		USER3_PRINT_LINE("         Items expected in list: %d\n",
				 Memory_Chain_Entries);
	}

	while (Current_Item != NULL) {
		/* Perform some simple checks on the items in the memory allocation list. */
		if (Current_Item->Signature == LEAK_SIGNATURE) {

			if (Current_Item->Starting_Sentinel == SENTINEL_VALUE) {

				/* Check the ending sentinel. */
				Ending_Sentinel =
				    (Sentinel_t *) ((unsigned int)Current_Item +
						    sizeof(Memory_Leak_Data_t) +
						    Current_Item->User_Size);
				if (*Ending_Sentinel == SENTINEL_VALUE) {

					/* Check the previous pointer. */
					if (Current_Item->Previous ==
					    Previous_Item) {

						if (p != NULL) {
							/* We have an address to check.  Does this address lie within this memory allocation? */
							Data_End =
							    (unsigned long)
							    Ending_Sentinel;
							Data_Start =
							    (unsigned
							     long)((unsigned
								    int)
								   Current_Item
								   +
								   sizeof
								   (Memory_Leak_Data_t));
							Address_To_Test =
							    (unsigned long)p;

							if (Address_To_Test ==
							    Data_Start) {
								/* This address was returned to the user by MALLOC_func. */
								Address_Found =
								    0;
								break;
							} else {

								if ((Address_To_Test > Data_Start) && (Address_To_Test < Data_End)) {
									/* The address lies within the data area of the current allocation. */
									Address_Found
									    = 1;
									break;
								}

							}

						}

					} else {
						/* We have a pointer problem!  The previous pointer field does not point to the previous entry in the list! */
						rc = 3;
						fprintf(stderr,
							"Link Error!  The previous pointer of the current item does not point to the previous item in the allocation list!\n");
						fprintf(stderr,
							"Current_Item->Previous = %p,  Actual Previous Item is %p.\n",
							Current_Item->Previous,
							Previous_Item);
						fprintf(stderr,
							"Item position in list: %d\n",
							Count);
						print_backtrace();
					}

				} else {
					rc = 3;
					fprintf(stderr,
						"Possible overrun error -- ending sentinel corrupted!\n"
						"     Header Address: %p\n"
						"     User Address: %p\n"
						"     Size: %d\n"
						"     Allocated in module %s, function %s, at line %d\n",
						Current_Item,
						(void *)((unsigned int)
							 Current_Item +
							 sizeof
							 (Memory_Leak_Data_t)),
						Current_Item->User_Size,
						Current_Item->Module_Name,
						Current_Item->Function_Name,
						Current_Item->Line_Number);
					print_backtrace();
				}

			} else {
				rc = 3;
				fprintf(stderr,
					"Memory Overwrite Error!  Starting Sentinel is incorrect!\n     Following data may be incorrect!\n"
					"     Header Address: %p\n"
					"     Address: %p\n" "     Size: %d\n"
					"     Allocated in module %s, function %s, at line %d\n",
					Current_Item,
					(void *)((unsigned int)Current_Item +
						 sizeof(Memory_Leak_Data_t)),
					Current_Item->User_Size,
					Current_Item->Module_Name,
					Current_Item->Function_Name,
					Current_Item->Line_Number);
				print_backtrace();
			}

			Count += 1;

		} else {
			rc = 3;
			fprintf(stderr,
				"Memory Overwrite Error!  Memory management data signature is incorrect!  Data Dump:\n"
				"     Header Address: %p\n"
				"     Signature: Actual[%u, 0x%x], Expected[%u, 0x%x]\n"
				"     User Address: %p\n"
				"     Size: [%u, 0x%x]\n"
				"     Starting Sentinel: Actual[%u, 0x%x], Expected[%u, 0x%x]\n"
				"     Next: %p\n" "     Previous: %p\n"
				"     Module: %p\n" "     Function: %p\n"
				"     Line [%u, ox%x]\n", Current_Item,
				Current_Item->Signature,
				Current_Item->Signature, LEAK_SIGNATURE,
				LEAK_SIGNATURE,
				(void *)((unsigned int)Current_Item +
					 sizeof(Memory_Leak_Data_t)),
				Current_Item->User_Size,
				Current_Item->User_Size,
				Current_Item->Starting_Sentinel,
				Current_Item->Starting_Sentinel, SENTINEL_VALUE,
				SENTINEL_VALUE, Current_Item->Next,
				Current_Item->Previous,
				Current_Item->Module_Name,
				Current_Item->Function_Name,
				Current_Item->Line_Number,
				Current_Item->Line_Number);
			fprintf(stderr,
				"Memory_Chain = %p\nMemory_Chain_Entries = %u\nCurrent_Item = %p, Count = %u\n\n",
				Memory_Chain, Memory_Chain_Entries,
				Current_Item, Count);
			print_backtrace();

			break;
		}

		Previous_Item = Current_Item;
		Current_Item = Current_Item->Next;
	}

	if (p != NULL) {

		if (rc == 0) {
			rc = Address_Found;
		}

	} else {

		if ((Count != Memory_Chain_Entries) && (rc == 0)) {
			fprintf(stderr,
				"The number of entries found in the allocation list [%u] != list count [%u]!!\n\n",
				Count, Memory_Chain_Entries);
			print_backtrace();
			rc = 3;
		}

	}

	FUNCTION_RETURN("%i", rc)
}

/*********************************************************************/
/*                                                                   */
/*   Function Name: Add_Memory_Allocation_To_Chain                   */
/*                                                                   */
/*   Descriptive Name: Adds a memory tracking object to the list of  */
/*                     memory tracking objects which represents all  */
/*                     memory allocations being tracked by this      */
/*                     module.                                       */
/*                                                                   */
/*   Input: Memory_Leak_Data_t * Memory_Leak_Data - The memory       */
/*                               tracking object to add.             */
/*                                                                   */
/*   Output: If Success : None.                                      */
/*                                                                   */
/*           If Failure : None.                                      */
/*                                                                   */
/*   Error Handling: None.                                           */
/*                                                                   */
/*   Side Effects: Other memory tracking objects may be modified.    */
/*                 The count of objects being tracked will be        */
/*                 incremented.                                      */
/*                                                                   */
/*   Notes:                                                          */
/*                                                                   */
/*********************************************************************/
static void Add_Memory_Allocation_To_Chain(Memory_Leak_Data_t *
					   Memory_Leak_Data)
{

	FUNCTION_ENTRY()

	    PRINT_FUNCTION_PARAMETER("Parameters:\n")
	    PRINT_FUNCTION_PARAMETER("     Memory_Leak_Data = %p",
				     Memory_Leak_Data)

	    if (Memory_Chain) {
		/* Other allocations exist.  Insert at the head of the chain. */
		Memory_Leak_Data->Next = Memory_Chain;
		Memory_Chain->Previous = Memory_Leak_Data;
	} else {
		/* This is the first allocation. */
		Memory_Leak_Data->Next = NULL;
	}

	/* Update the pointer to the start of the allocation list. */
	Memory_Chain = Memory_Leak_Data;

	/* Update the count of entries in the memory chain. */
	Memory_Chain_Entries += 1;

	FUNCTION_EXIT()
}

/*********************************************************************/
/*                                                                   */
/*   Function Name: Remove_Memory_Allocation_From_Chain              */
/*                                                                   */
/*   Descriptive Name: Removes a memory tracking object from         */
/*                     the linked list of memory tracking            */
/*                     objects representing current memory           */
/*                     allocations.                                  */
/*                                                                   */
/*   Input: Memory_Leak_Data_t * Memory_Leak_Data - the memory       */
/*                      tracking object to remove.                   */
/*                                                                   */
/*   Output: If Success : None.                                      */
/*                                                                   */
/*           If Failure : None.                                      */
/*                                                                   */
/*   Error Handling: None.                                           */
/*                                                                   */
/*   Side Effects: Other memory tracking objects may be modified.    */
/*                 The memory tracking object being removed will be  */
/*                 zeroed out and freed.                             */
/*                                                                   */
/*   Notes:                                                          */
/*                                                                   */
/*********************************************************************/
static void Remove_Memory_Allocation_From_Chain(Memory_Leak_Data_t *
						Memory_Leak_Data)
{
	Memory_Leak_Data_t *Temp;	/* Used when adjusting the memory allocation chain. */
	Sentinel_t *Ending_Sentinel;	/* To allow access to the sentinel at the end of the allocated memory. */

	FUNCTION_ENTRY()

	    /* Are we the first item in the chain? */
	    if (Memory_Leak_Data->Previous == NULL) {

		/* We are at the head of the chain, which means that Memory_Chain points to us!
		 * Point Memory_Chain to the Memory_Leak_Data_t that follows us (if any).
		 */
		Memory_Chain = Memory_Leak_Data->Next;

		/* Is there another item in the chain?  If so, update it. */
		if (Memory_Chain != NULL)
			Memory_Chain->Previous = NULL;

	} else {

		/* Are we the last item in the chain? */
		if (Memory_Leak_Data->Next == NULL) {

			/* We are at the end of the memory chain with at least 1 entry before us. */
			Temp = Memory_Leak_Data->Previous;
			Temp->Next = NULL;
		} else {
			/* We are in the middle of the chain. */
			Temp = Memory_Leak_Data->Previous;
			Temp->Next = Memory_Leak_Data->Next;
			Temp = Memory_Leak_Data->Next;
			Temp->Previous = Memory_Leak_Data->Previous;
		}

	}

	/* Adjust the count of entries in the memory chain. */
	Memory_Chain_Entries -= 1;

	/* Calculate the address of the ending sentinel so that we can access it. */
	Ending_Sentinel =
	    (Sentinel_t *) ((unsigned int)Memory_Leak_Data +
			    sizeof(Memory_Leak_Data_t) +
			    Memory_Leak_Data->User_Size);

	/* Clean out the memory leak data. */
	Memory_Leak_Data->Function_Name = NULL;
	Memory_Leak_Data->Line_Number = 0;
	Memory_Leak_Data->Module_Name = NULL;
	Memory_Leak_Data->Signature = 0;
	Memory_Leak_Data->Starting_Sentinel = 0;
	Memory_Leak_Data->User_Size = 0;
	Memory_Leak_Data->Next = NULL;
	Memory_Leak_Data->Previous = NULL;
	*Ending_Sentinel = 0;

	free(Memory_Leak_Data->Mem_Address);

	FUNCTION_EXIT()
}

/*--------------------------------------------------
 API Functions
--------------------------------------------------*/

/*********************************************************************/
/*                                                                   */
/*   Function Name: Print_Leak_List                                  */
/*                                                                   */
/*   Descriptive Name: This function walks the list of allocated     */
/*                     memory blocks and prints information about    */
/*                     each one.  If this is done at program exit,   */
/*                     the resulting list of memory blocks most      */
/*                     likely represents leaked memory.              */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: If Success : If there are any memory blocks being       */
/*                        tracked by this module, information about  */
/*                        block still being tracked will be sent to  */
/*                        stderr.                                    */
/*                                                                   */
/*           If Failure : Error messages may be sent to stderr.      */
/*                                                                   */
/*   Error Handling: If errors are detected, then error messages are */
/*                   output on stderr.                               */
/*                                                                   */
/*   Side Effects: The internal structures of this module are checked*/
/*                 for errors with any errors being reported on      */
/*                 stderr.                                           */
/*                                                                   */
/*   Notes:                                                          */
/*                                                                   */
/*********************************************************************/
void Print_Leak_List(void)
{
	Memory_Leak_Data_t *Current_Item;	/* Used to walk the memory allocation chain. */
	Sentinel_t *Ending_Sentinel;	/* To allow access to the sentinel at the end of the allocated memory. */

	API_FUNCTION_ENTRY()

	    /* Get lock. */
	    pthread_mutex_lock(&Memory_Leak_Lock);

	Current_Item = Memory_Chain;

	fprintf(stderr,
		"\n\nMemory_Chain is %p, and Memory_Chain_Entries is %u.\n",
		Memory_Chain, Memory_Chain_Entries);
	fprintf(stderr, "\nCurrent Memory Allocation List\n\n");

	while (Current_Item != NULL) {
		/* Perform some simple checks on the data before we print it. */
		if (Current_Item->Signature == LEAK_SIGNATURE) {

			if (Current_Item->Starting_Sentinel == SENTINEL_VALUE) {

				/* Check the ending sentinel. */
				Ending_Sentinel =
				    (Sentinel_t *) ((unsigned int)Current_Item +
						    sizeof(Memory_Leak_Data_t) +
						    Current_Item->User_Size);
				if (*Ending_Sentinel == SENTINEL_VALUE) {
					fprintf(stderr,
						"Memory Allocation Record\n");
				} else {
					fprintf(stderr,
						"\nPossible overrun error -- ending sentinel corrupted!\n");
				}

				fprintf(stderr,
					"     Allocation Block: %p\n"
					"     Header Address: %p\n"
					"     User Address: %p\n"
					"     Alignment: %u\n"
					"     Size: %d\n"
					"     Allocated in module %s, function %s, at line %d\n\n",
					Current_Item->Mem_Address,
					Current_Item,
					(void *)((unsigned int)Current_Item +
						 sizeof(Memory_Leak_Data_t)),
					Current_Item->Alignment,
					Current_Item->User_Size,
					Current_Item->Module_Name,
					Current_Item->Function_Name,
					Current_Item->Line_Number);
			} else {
				fprintf(stderr,
					"\nMemory Overwrite Error!  Starting Sentinel is incorrect!\n     Following data may be incorrect!\n");
				fprintf(stderr,
					"     Allocation Block: %p\n"
					"     Header Address: %p\n"
					"     User Address: %p\n"
					"     Alignment: %u\n" "     Size: %d\n"
					"     Allocated in module %s, function %s, at line %d\n\n",
					Current_Item->Mem_Address, Current_Item,
					(void *)((unsigned int)Current_Item +
						 sizeof(Memory_Leak_Data_t)),
					Current_Item->Alignment,
					Current_Item->User_Size,
					Current_Item->Module_Name,
					Current_Item->Function_Name,
					Current_Item->Line_Number);
			}

		} else {
			fprintf(stderr,
				"\n\nMemory Overwrite Error!  Memory management data signature is incorrect!  Data Dump:\n");
			fprintf(stderr,
				"     Allocation Block: %p\n"
				"     Header Address: %p\n"
				"     Signature: Actual[%u, 0x%x], Expected[%u, 0x%x]\n"
				"     Address: %p\n" "     Alignment: %u\n"
				"     Size: [%u, 0x%x]\n"
				"     Starting Sentinel: Actual[%u, 0x%x], Expected[%u, 0x%x]\n"
				"     Next: %p\n" "     Previous: %p\n"
				"     Module: %p\n" "     Function: %p\n"
				"     Line [%u, ox%x]\n",
				Current_Item->Mem_Address, Current_Item,
				Current_Item->Signature,
				Current_Item->Signature, LEAK_SIGNATURE,
				LEAK_SIGNATURE,
				(void *)((unsigned int)Current_Item +
					 sizeof(Memory_Leak_Data_t)),
				Current_Item->Alignment,
				Current_Item->User_Size,
				Current_Item->User_Size,
				Current_Item->Starting_Sentinel,
				Current_Item->Starting_Sentinel, SENTINEL_VALUE,
				SENTINEL_VALUE, Current_Item->Next,
				Current_Item->Previous,
				Current_Item->Module_Name,
				Current_Item->Function_Name,
				Current_Item->Line_Number,
				Current_Item->Line_Number);
			fprintf(stderr,
				"Memory_Chain = %p\nMemory_Chain_Entries = %u\nCurrent_Item = %p\n\n",
				Memory_Chain, Memory_Chain_Entries,
				Current_Item);
			break;
		}

		Current_Item = Current_Item->Next;
	}

	/* Release lock. */
	pthread_mutex_unlock(&Memory_Leak_Lock);

	API_FUNCTION_EXIT()

}

/*********************************************************************/
/*                                                                   */
/*   Function Name: MEMORY_func                                      */
/*                                                                   */
/*   Descriptive Name: This function acts as a replacement for       */
/*                     malloc.  It allocates memory (using malloc)   */
/*                     and enters that memory into a tracking        */
/*                     structure so that memory leaks, if any, may   */
/*                     be found.                                     */
/*                                                                   */
/*   Input: size_t sz - The number of bytes to be allocated.         */
/*          unsigned int Alignment - 0 for non-aligned (normal)      */
/*                                   malloc, > 0 to return an        */
/*                                   address aligned on a specific   */
/*                                   memory boundary.  If > 0, then  */
/*                                   Alignment must be a power of 2  */
/*                                   and a multiple of sizeof(void *)*/
/*          void ** Memory_Location - The address of a variable which*/
/*                                   will hold the address of the    */
/*                                   allocated by this function.     */
/*          const char * mod_name - The name of the module from which*/
/*                                  this function was called.        */
/*          const char * func - The name of the function from which  */
/*                              this function was called.            */
/*          const int line - The line number of the code containing  */
/*                           the call to this function.              */
/*                                                                   */
/*   Output: If Success : The function return value will be 0.       */
/*                        *Memory_Location will be set to the address*/
/*                        of the first byte of the user portion of   */
/*                        any memory that was allocated.             */
/*                                                                   */
/*           If Failure : The function return value will be EINVAL or*/
/*                        ENOMEM.  Errors may be reported on stderr. */
/*                        *Memory_Location will be set to NULL.      */
/*                                                                   */
/*   Error Handling: This function will abort if an error is         */
/*                   is detected.  Errors could be lack of memory, or*/
/*                   corruption of the internal structures used to   */
/*                   track allocated blocks of memory.               */
/*                                                                   */
/*   Side Effects: Memory may be allocated, errors may be reported   */
/*                 on stderr.                                        */
/*                                                                   */
/*   Notes:                                                          */
/*                                                                   */
/*********************************************************************/
int MEMORY_func(size_t sz,
		unsigned int Alignment,
		void **Memory_Location,
		const char *mod_name, const char *func, const int line)
{
	Memory_Leak_Data_t *Memory_Leak_Data;	/* For accessing our memory tracking data. */
	void *ptr = NULL;	/* To hold the address that will be returned to the caller. */
	size_t New_Size;	/* Used to adjust the size of the memory allocation. */
	Sentinel_t *Ending_Sentinel;	/* To allow access to the sentinel at the end of the allocated memory. */
	unsigned int Proposed_User_Address;	/* Used to calculate the address to return to the caller. */
	unsigned int Shift_Amount;	/* Used when adjusting the address returned to caller to ensure proper alignment. */
	int Error = 0;
	unsigned int i = 0;	/* Used when calculating powers of two to check the alignment. */
	unsigned int Power_Of_Two = 1;	/* Used when calculating powers of two to check the alignment. */

#define MAX_POWER_OF_TWO   31

	API_FUNCTION_ENTRY()

	    PRINT_API_PARAMETER("Parameters:\n")
	    PRINT_API_PARAMETER("     sz = %d", sz)
	    PRINT_API_PARAMETER("     Alignment = %d", Alignment)
	    PRINT_API_PARAMETER("     Memory_Location = %p", Memory_Location)
	    PRINT_API_PARAMETER("     mod_name = %s", mod_name)
	    PRINT_API_PARAMETER("     func = %s", func)
	    PRINT_API_PARAMETER("     line = %i", line)

	    /* Check the size of the request. */
	    if (sz == 0) {
		Error = EINVAL;
		fprintf(stderr,
			"MALLOC: request is invalid - size[%u] in module %s, function %s at line %d\n",
			sz, mod_name, func, line);
		print_backtrace();
	}

	/* Check the alignment, if one was specified. */
	if (Alignment > 0) {
		if (Alignment % sizeof(void *)) {
			/* Improper alignment! */
			Error = EINVAL;
		} else {
			do {
				Power_Of_Two *= 2;
				i++;
			} while ((Power_Of_Two != Alignment)
				 && (i < MAX_POWER_OF_TWO));

			if (Power_Of_Two != Alignment) {
				Error = EINVAL;
			}

		}

		if (Error) {
			fprintf(stderr,
				"MEMORY_func: request for aligned memory uses invalid alignment! size[%u], alignment [%u] in module %s, function %s at line %d\n",
				sz, Alignment, mod_name, func, line);
		}

	}

	if (Memory_Location == NULL) {
		Error = EINVAL;
		fprintf(stderr,
			"MEMORY_func: Location to place address of allocated memory is NULL! size[%u], alignment [%u] in module %s, function %s at line %d\n",
			sz, Alignment, mod_name, func, line);
	} else
		*Memory_Location = NULL;

	if (Error != 0) {
		API_FUNCTION_RETURN("%i", Error);
	}

	USER3_PRINT_LINE
	    ("MEMORY_func: processing request - size[%u], alignment[%u] from module %s, function %s at line %d\n",
	     sz, Alignment, mod_name, func, line);

	/* Check */

	/* Get lock. */
	pthread_mutex_lock(&Memory_Leak_Lock);

	/* Check the memory allocation list.  If the list is bad, then abort because we are already screwed ... */
	if (!Check_Leak_List(NULL)) {

		/* Adjust the size of the request to account for the additional memory we need to track this request. */
		New_Size =
		    sz + sizeof(Memory_Leak_Data_t) + sizeof(Sentinel_t) +
		    Alignment;

		/* Get the memory. */
		ptr = malloc(New_Size);

		/* Was the memory available? */
		if (ptr != NULL) {
			/* Determine where to put our memory leak data. */
			if (Alignment == 0) {
				/* We can place the memory leak data right at the start of the memory block we got from malloc. */
				Memory_Leak_Data = (Memory_Leak_Data_t *) ptr;
			} else {
				Proposed_User_Address =
				    (unsigned int)ptr +
				    sizeof(Memory_Leak_Data_t);
				Shift_Amount =
				    Alignment -
				    (Proposed_User_Address % Alignment);
				Memory_Leak_Data =
				    (Memory_Leak_Data_t *) ((unsigned int)ptr +
							    Shift_Amount);
			}

			/* Save the address returned by malloc() for use with free(). */
			Memory_Leak_Data->Mem_Address = ptr;

			/* Create the address to return to the caller.  This address should be the first byte after
			   our memory leak data.                                                                      */
			ptr =
			    (void *)((unsigned int)Memory_Leak_Data +
				     sizeof(Memory_Leak_Data_t));

			/* Calculate the address of the trailing sentinel. */
			Ending_Sentinel =
			    (Sentinel_t *) ((unsigned int)ptr + sz);

			/* Initialize our memory leak data. */
			Memory_Leak_Data->Signature = LEAK_SIGNATURE;
			Memory_Leak_Data->User_Size = sz;
			Memory_Leak_Data->Alignment = Alignment;
			Memory_Leak_Data->Module_Name = mod_name;
			Memory_Leak_Data->Function_Name = func;
			Memory_Leak_Data->Line_Number = line;
			Memory_Leak_Data->Previous = NULL;
			Memory_Leak_Data->Next = NULL;
			Memory_Leak_Data->Starting_Sentinel = SENTINEL_VALUE;
			*Ending_Sentinel = SENTINEL_VALUE;

			USER3_PRINT_LINE
			    ("MALLOC: Allocated header at %p ( = user address %p) size[%u] in module %s, function %s at line %d\n",
			     Memory_Leak_Data, ptr, sz, mod_name, func, line);
			Add_Memory_Allocation_To_Chain(Memory_Leak_Data);
			USER3_PRINT_LINE
			    ("MALLOC: Memory_Chain is %p, and Memory_Chain_Entries is %u.\n",
			     Memory_Chain, Memory_Chain_Entries);

			/* Set up return value. */
			*Memory_Location = ptr;
		} else {

			fprintf(stderr,
				"MALLOC: Memory allocation failed - no more memory available at this time!\n");
			print_backtrace();

			USER3_PRINT_LINE
			    ("MALLOC: request was issued from module %s, function %s at line %d\n",
			     mod_name, func, line);

		}

	} else {
		fprintf(stderr,
			"MALLOC: Found the memory leak data to be corrupted! Aborting!\n");
		print_backtrace();
	}

	/* Release lock. */
	pthread_mutex_unlock(&Memory_Leak_Lock);

	API_FUNCTION_RETURN("%i", Error)

}

/*********************************************************************/
/*                                                                   */
/*   Function Name: MALLOC_func                                      */
/*                                                                   */
/*   Descriptive Name: This function acts as a replacement for       */
/*                     malloc.  It allocates memory (using malloc)   */
/*                     and enters that memory into a tracking        */
/*                     structure so that memory leaks, if any, may   */
/*                     be found.                                     */
/*                                                                   */
/*   Input: size_t sz - The number of bytes to be allocated.         */
/*          const char * mod_name - The name of the module from which*/
/*                                  this function was called.        */
/*          const char * func - The name of the function from which  */
/*                              this function was called.            */
/*          const int line - The line number of the code containing  */
/*                           the call to this function.              */
/*                                                                   */
/*   Output: If Success : The function return value will be non-NULL.*/
/*                                                                   */
/*           If Failure : The function return value will be NULL.    */
/*                        Errors may be reported on stderr.          */
/*                                                                   */
/*   Error Handling: This function will abort if an error is         */
/*                   is detected.  Errors could be lack of memory, or*/
/*                   corruption of the internal structures used to   */
/*                   track allocated blocks of memory.               */
/*                                                                   */
/*   Side Effects: Memory may be allocated, errors may be reported   */
/*                 on stderr.                                        */
/*                                                                   */
/*   Notes:                                                          */
/*                                                                   */
/*********************************************************************/
void *MALLOC_func(size_t sz, const char *mod_name, const char *func,
		  const int line)
{
	void *Return_Value;
	int Error;

	Error = MEMORY_func(sz, 0, &Return_Value, mod_name, func, line);

	if (Error != 0)
		return NULL;
	else
		return Return_Value;
}

/*********************************************************************/
/*                                                                   */
/*   Function Name: FREE_func                                        */
/*                                                                   */
/*   Descriptive Name: This function frees a block of memory being   */
/*                     tracked by this module and removes the block  */
/*                     from its tracking structures.                 */
/*                                                                   */
/*   Input: const void * p - The address of the block of memory to   */
/*                           be freed.                               */
/*          const char * mod_name - The name of the module requesting*/
/*                                  the block of memory be freed.    */
/*          const char * func - The name of the function requesting  */
/*                              the block of memory be freed.        */
/*          const int line - The line number of the line of code in  */
/*                           module calling this function.           */
/*                                                                   */
/*   Output: If Success : None.                                      */
/*                                                                   */
/*           If Failure : Errors may be reported to stderr.          */
/*                                                                   */
/*   Error Handling: This function causes the internal structures    */
/*                   of this module to be checked as part of the     */
/*                   process of freeing the address p.  This may     */
/*                   cause errors to be reported on stderr. If any   */
/*                   errors are found, then the address p may not be */
/*                   freed.                                          */
/*                                                                   */
/*   Side Effects: The block of memory associated with the address p */
/*                 will be freed and available for reallocation.     */
/*                 Also, the memory tracking structures in this      */
/*                 module will undergo a series of checks.           */
/*                                                                   */
/*   Notes: This function was not intended to be called directly but */
/*          rather through the macro FREE.                           */
/*                                                                   */
/*********************************************************************/
void FREE_func(const void *p, const char *mod_name, const char *func,
	       const int line)
{
	Memory_Leak_Data_t *Memory_Leak_Data;	/* For accessing our memory tracking data. */

	API_FUNCTION_ENTRY()

	    PRINT_API_PARAMETER("Parameters:\n")
	    PRINT_API_PARAMETER("     p = %p", p)
	    PRINT_API_PARAMETER("     mod_name = %s", mod_name)
	    PRINT_API_PARAMETER("     func = %s", func)
	    PRINT_API_PARAMETER("     line = %i", line)

	    /* Check the address passed to us.  It must not be NULL, and must be > sizeof(Memory_Leak_Data_t) to
	     * prevent wrap around when we subtract sizeof(Memory_Leak_Data_t) to get the starting address of
	     * the memory leak data associated with p.
	     */
	    if ((p == NULL) || ((unsigned int)p <= sizeof(Memory_Leak_Data_t))) {
		fprintf(stderr,
			"FREE: request has invalid user address [%p].  Request came from module %s, function %s, line %d\n",
			p, mod_name, func, line);
		print_backtrace();
		return;
	} else {
		USER3_PRINT_LINE
		    ("FREE: request to free [%p] received from module %s, function %s, line %d\n",
		     p, mod_name, func, line);
	}

	/* Get lock. */
	pthread_mutex_lock(&Memory_Leak_Lock);

	/* Check the memory allocation list for errors. */
	if (!Check_Leak_List(NULL)) {

		/* Is the address given to us in the memory allocation list? */
		if (!Check_Leak_List(p)) {
			/* Get access to the memory leak data. */
			Memory_Leak_Data =
			    (Memory_Leak_Data_t *) ((unsigned int)p -
						    sizeof(Memory_Leak_Data_t));

			USER3_PRINT_LINE
			    ("FREE: Calling free on header address %p ( = user address %p) from module %s, function %s, line %d\n",
			     Memory_Leak_Data, p, mod_name, func, line);
			Remove_Memory_Allocation_From_Chain(Memory_Leak_Data);
			USER3_PRINT_LINE
			    ("FREE: Memory_Chain is %p, and Memory_Chain_Entries is %u.\n",
			     Memory_Chain, Memory_Chain_Entries);

		} else {
			/* The address given to us was not in the memory allocation list!  If we free this address, who knows what will happen! */
			fprintf(stderr,
				"FREE: Invalid address!  The address %p was not provided by MALLOC or has been passed to FREE more than once!\n",
				p);
			fprintf(stderr,
				"      Request came from module %s, function %s, line %d\n",
				mod_name, func, line);
			print_backtrace();
		}

	} else {
		fprintf(stderr,
			"FREE: Aborting due to errors in the memory leak data!\n");
		USER3_PRINT_LINE
		    ("FREE: request was submitted by module %s, function %s at line %d\n",
		     mod_name, func, line);
		print_backtrace();

	}

	pthread_mutex_unlock(&Memory_Leak_Lock);

	USER3_PRINT_LINE("FREE: request is complete.\n");

	API_FUNCTION_EXIT()
}

/*********************************************************************/
/*                                                                   */
/*   Function Name: Test_Address_Allocation                          */
/*                                                                   */
/*   Descriptive Name: This function tests the specified address to  */
/*                     to see if it lies within an allocated block   */
/*                     tracked by this module.                       */
/*                                                                   */
/*   Input: void * p - The address to be tested.                     */
/*                                                                   */
/*   Output: If Success : If the address p was found, then 0 will be */
/*                        returned if the address is the start of    */
/*                        a block of allocated memory.  If the       */
/*                        address p was found within an allocated    */
/*                        block of memory, then 1 is returned.       */
/*                                                                   */
/*           If Failure : If the address p was NOT found, then 2 is  */
/*                        returned.  If there was an error in the    */
/*                        memory tracking system then 3 will be      */
/*                        returned.                                  */
/*                                                                   */
/*   Error Handling: This function relies on the error handling      */
/*                   built into the Check_Leak_List function and     */
/*                   has no error handling of its own.               */
/*                                                                   */
/*   Side Effects: If the list of memory allocations contains errors */
/*                 then those errors will be detected and reported   */
/*                 on stderr.                                        */
/*                                                                   */
/*   Notes: If NULL is passed in as the address to test, then the    */
/*          integrity of the internal tracking structures will be    */
/*          checked, in which case a return value of 0 signifies     */
/*          that the internal tracking structures have passed the    */
/*          checks and a return value of 3 indicates that errors     */
/*          were found.                                              */
/*                                                                   */
/*********************************************************************/
unsigned int Test_Address_Allocation(void *p)
{
	unsigned int rc = 0;

	API_FUNCTION_ENTRY()

	    PRINT_API_PARAMETER("Parameters:\n")
	    PRINT_API_PARAMETER("     p = %p", p)

	    /* Get lock. */
	    pthread_mutex_lock(&Memory_Leak_Lock);

	rc = Check_Leak_List(p);

	/* Release lock. */
	pthread_mutex_unlock(&Memory_Leak_Lock);

	API_FUNCTION_RETURN("%i", rc);
}

/*********************************************************************/
/*                                                                   */
/*   Function Name: Duplicate_String                                 */
/*                                                                   */
/*   Descriptive Name: This function duplicates a string.  The memory*/
/*                     allocated for the duplicate is allocated      */
/*                     using the MALLOC_func routine in this module  */
/*                     and is thus tracked by this module.           */
/*                                                                   */
/*   Input: const char * Source - The string to be copied.           */
/*          const char * mod_name - The name of the module containing*/
/*                                  the function which called this   */
/*                                   function.                       */
/*          const char * func - The name of the function calling     */
/*                              this function.                       */
/*          const int line - The line number of the line of code in  */
/*                           module calling this function.           */
/*                                                                   */
/*   Output: If Success : The function return value will be non-NULL */
/*                        and will point to a duplicate of the       */
/*                        string given in Source.                    */
/*                                                                   */
/*           If Failure : The function return value will be NULL.    */
/*                                                                   */
/*   Error Handling: Any errors detected by this function result in  */
/*                   a function return value of NULL.                */
/*                                                                   */
/*   Side Effects: The memory tracking features of this module are   */
/*                 employed to allocate memory for the duplicate     */
/*                 string produced by this funciton.                 */
/*                                                                   */
/*   Notes:                                                          */
/*                                                                   */
/*********************************************************************/
char *Duplicate_String(const char *Source,
		       const char *mod_name, const char *func, const int line)
{
	char *Result;

	Result = (char *)MALLOC_func(strlen(Source) + 1, mod_name, func, line);
	if (Result != NULL)
		strcpy(Result, Source);

	return Result;
}

/*********************************************************************/
/*                                                                   */
/*   Function Name: Realloc_func                                     */
/*                                                                   */
/*   Descriptive Name: This function performs the same function as   */
/*                     the realloc function in the ANSI C library.   */
/*                                                                   */
/*   Input: const void * p - The address of the block of memory to   */
/*                           be reallocated.                         */
/*          size_t size - The size of the memory block to return.    */
/*          const char * mod_name - The name of the module requesting*/
/*                                  the block of memory be freed.    */
/*          const char * func - The name of the function requesting  */
/*                              the block of memory be freed.        */
/*          const int line - The line number of the line of code in  */
/*                           module calling this function.           */
/*                                                                   */
/*   Output: If Success : The function return value will be a pointer*/
/*                        to the new block of memory.                */
/*                                                                   */
/*           If Failure : NULL will be returned and errno will be set*/
/*                        to a non-null error code.                  */
/*                                                                   */
/*   Error Handling: This function causes the internal structures    */
/*                   of this module to be checked.  This may         */
/*                   cause errors to be reported on stderr. If any   */
/*                   errors are found, then the address p may not be */
/*                   freed.                                          */
/*                                                                   */
/*   Side Effects: A new block of memory of size bytes will be       */
/*                 allocated, the contents of the current block will */
/*                 be copied to the new block (at least as much as   */
/*                 will fit, and the current block will be freed.    */
/*                 This will cause internal structures in this module*/
/*                 to be modified accordingly.                       */
/*                                                                   */
/*   Notes: This function was not intended to be called directly but */
/*          rather through the macro REALLOC.                        */
/*                                                                   */
/*          If p is NULL, then this will cause this function to      */
/*          behave like malloc.                                      */
/*                                                                   */
/*          If size is 0, then this will cause this function to      */
/*          behave like free.                                        */
/*                                                                   */
/*********************************************************************/
void *Realloc_func(void *p, size_t size, const char *mod_name, const char *func,
		   const int line)
{
	Memory_Leak_Data_t *Memory_Leak_Data;	/* For accessing our memory tracking data. */
	int Error = 0;
	unsigned int Copy_Size = 0;
	void *Return_Value = NULL;

	API_FUNCTION_ENTRY()

	    PRINT_API_PARAMETER("Parameters:\n")
	    PRINT_API_PARAMETER("     p = %p", p)
	    PRINT_API_PARAMETER("     size = %u", size)
	    PRINT_API_PARAMETER("     mod_name = %s", mod_name)
	    PRINT_API_PARAMETER("     func = %s", func)
	    PRINT_API_PARAMETER("     line = %i", line)

	    /* Check the address passed to us.  If it is NULL and size > 0, then we are merely doing a malloc.   */
	    if ((p == NULL) && (size > 0)) {
		USER3_PRINT_LINE
		    ("REALLOC: p was NULL and size > 0, acting as MALLOC.\n")
		    Return_Value = MALLOC_func(size, mod_name, func, line);
		if (Return_Value == NULL) {
			errno = ENOMEM;
		}
	} else {
		/* If size is 0 and p is not NULL, then we are doing a free. */
		if ((p != NULL) && (size == 0)) {
			USER3_PRINT_LINE
			    ("REALLOC: p was non-NULL but size is 0, acting as FREE.\n")
			    FREE_func(p, mod_name, func, line);
		} else {
			/* Do we have real work to do? */
			if ((p != NULL) && (size != 0)) {
				USER3_PRINT_LINE
				    ("REALLOC: p was non-NULL and size > 0, we have actual work to do!.\n")

				    /* Get lock. */
				    pthread_mutex_lock(&Memory_Leak_Lock);

				/* Check the memory allocation list for errors. */
				if (!Check_Leak_List(NULL)) {

					/* Is the address given to us in the memory allocation list? */
					if (!Check_Leak_List(p)) {
						/* Get access to the memory leak data. */
						Memory_Leak_Data =
						    (Memory_Leak_Data_t
						     *) ((unsigned int)p -
							 sizeof
							 (Memory_Leak_Data_t));

						/* Release the lock so that MEMORY_func can get it. */
						pthread_mutex_unlock
						    (&Memory_Leak_Lock);

						/* Call MEMORY_func to get the memory we need and add it to the list of memory blocks we are tracking. */
						Error =
						    MEMORY_func(size,
								Memory_Leak_Data->
								Alignment,
								&Return_Value,
								mod_name, func,
								line);
						if (Error != 0) {
							Return_Value = NULL;
							errno = ENOMEM;
						} else {

							/* We have the replacement memory.  Now lets copy what we can from the original block to the new block. */
							if (size >
							    Memory_Leak_Data->
							    User_Size) {
								Copy_Size =
								    Memory_Leak_Data->
								    User_Size;
							} else {
								Copy_Size =
								    size;
							}

							/* Copy the data to be preserved from the original memory block to the new memory block. */
							memcpy(Return_Value, p,
							       Copy_Size);

							/* Zero out the original memory block ... this can catch errors as users should not assume that
							 * realloc will leave their original memory block intact.
							 */
							memset(p, 0x0,
							       Memory_Leak_Data->
							       User_Size);

							/* Get the lock again. */
							pthread_mutex_lock
							    (&Memory_Leak_Lock);

							USER3_PRINT_LINE
							    ("REALLOC: Calling free on header address %p ( = user address %p) from module %s, function %s, line %d\n",
							     Memory_Leak_Data,
							     p, mod_name, func,
							     line);
							Remove_Memory_Allocation_From_Chain
							    (Memory_Leak_Data);
							USER3_PRINT_LINE
							    ("REALLOC: Memory_Chain is %p, and Memory_Chain_Entries is %u.\n",
							     Memory_Chain,
							     Memory_Chain_Entries);

						}

					} else {
						/* The address given to us was not in the memory allocation list!  If we free this address, who knows what will happen! */
						fprintf(stderr,
							"REALLOC: Invalid address!  The address %p was not provided by MALLOC or has been passed to FREE already!\n",
							p);
						fprintf(stderr,
							"      Request came from module %s, function %s, line %d\n",
							mod_name, func, line);
						print_backtrace();
					}

				} else {
					fprintf(stderr,
						"REALLOC: Aborting due to errors in the memory leak data!\n");
					USER3_PRINT_LINE
					    ("REALLOC: request was submitted by module %s, function %s at line %d\n",
					     mod_name, func, line);
					print_backtrace();

				}

				pthread_mutex_unlock(&Memory_Leak_Lock);

			}

		}

	}

	USER3_PRINT_LINE("REALLOC: request is complete.\n");

	API_FUNCTION_RETURN("%p", Return_Value)
}
