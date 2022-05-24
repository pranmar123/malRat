#include <Windows.h>
//Function detect when it dll has attached to something
BOOL APIENTRY DllMain(HMODULE Base, DWORD Callback, LPVOID Param)
{
	//creating switch to detect if DLL is attached or detached
	switch (Callback)
	{
	case DLL_PROCESS_ATTACH:

		break;
	case DLL_PROCESS_DETACH:

		break;

	default:
		break;
	}

	//returning 1 at the end because function APIENTRY requires return of 1
	return 1;

}


//message box used to display "Testing from dll when run at Entry with cmd below
// rundll32.exe "C:\Users\IEUser\source\repos\malRat\Debug\funDLL.dll",Entry 
extern "C" __declspec(dllexport) int Entry() {
	return MessageBoxA(0, "Testing from dll", 0, 0);
}

