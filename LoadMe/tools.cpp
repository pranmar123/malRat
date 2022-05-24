#include "includes.h"

//PVOID is a pointer to a memory address with no info about the type of that value that is pointing to. 
//later on we will cast the pointer to a type ((char *) varName, (int*) varName) so compiler knows what we are working with
PVOID Tools::GetImageBase()
{
	//PDWORD is a pointer to a DWORD. DWORD is a 32-bit unsigned integer.
	PDWORD virtual_address = PDWORD(&GetImageBase);
	PDWORD image_base = NULL;

	//assembly code
	_asm {
		//move virtual_address content into register eax
		mov eax, virtual_address
		//compare eax AND 0xFFFF0000; and will check each of the bits and see if they are 
		and eax, 0xFFFF0000

		IterateImage:
		cmp WORD PTR[eax], 0x5A4D
			je EndIteration
			sub eax, 0x0010000

	}
} 