//program to that will be used to dump our payload on target
#include <Windows.h>


//entry point when in debug mode (this helps to reduce our exe when we build this)
//goal at runtime is to dump the last section (where we store the dll) and save that file on our target's system
//in order to do this we need to create a function that will get the image base from loader and then a function to locate in this loader where our DLL is stored
INT WINAPI WinMain(HMODULE current, HMODULE previous, LPSTR cmd, INT show) {

}


/* WHEN IN RELEASE MODE
//folder to load our payload
#include <Windows.h>
//pragma comment for the linker so it knows what the entry points for our code are (this helps to reduce our exe when we build this)
#pragma comment(linker, "/ENTRY:LoadEntry")

//entry point when in release mode (this helps to reduce our exe when we build this)
void LoadEntry()
{

}

*/