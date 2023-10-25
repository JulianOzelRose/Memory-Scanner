# Memory Scanner
This program is an x86/x64-based memory scanner. It can search for and modify bytes, integers, and strings.
It is essentially a clone of Cheat Engine. This scanner allows the user to search memory addresses for
values, and then save those values.

Once a memory value is saved, you can change its data type, add a description,
change its address, and more. To run the program, navigate to the proper release folder for your target binary (x86/x64),
then download ```Memory-Scanner.exe```, then open it. Upon opening the scanner, it will prompt you for administrative
privileges -- which is required for the memory scanning functions to work properly.

#### Release folders
- [Release (x86)](https://github.com/JulianOzelRose/Memory-Scanner/tree/master/Memory-Scanner/Win32/Release)
- [Release (x64)](https://github.com/JulianOzelRose/Memory-Scanner/tree/master/Memory-Scanner/x64/Release)

#### Technologies used
- Visual C++
- .NET Framework 4.8
- WinForms
- Windows API

## Program screenshots
#### UI after search completed
![Memory-Scanner-Sample-1](https://github.com/JulianOzelRose/Memory-Scanner/assets/95890436/3254909a-fd89-4472-a5af-a9123c2d19c4)
#### UI when changing an address
![Memory-Scanner-Sample-3](https://github.com/JulianOzelRose/Memory-Scanner/assets/95890436/201fa3da-970e-42fc-9b87-4475acdd6d1b)

## Memory read functions
These functions read memory values using the ```ReadProcessMemory``` function from the Windows API. They return the result as a
managed string, but this can easily be changed to return as a number type. A couple things to note is that when reading memory
from an .exe, you must factor in the module base address. You must also pass in the process handle, which can be done with:<br>

```HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, dwProcessId);```<br>

All you need is the process ID, and the Windows header, which can be included by using the ```#include <Windows.h>``` preprocessor directive.
For the string value function, you can modify ```MAX_STRING_LENGTH``` to your desired string length. The extra byte added to the string
size is to account for the null terminator.

#### Byte value
```
String^ ReadByteFromMemory(HANDLE hProcess, BYTE* cbAddress)
{
	BYTE bValue = 0;

	if (!ReadProcessMemory(hProcess, reinterpret_cast<LPVOID>(cbAddress), &bValue, sizeof(bValue), nullptr))
	{
		return "???";
	}

	return bValue.ToString();
}
```

#### 2-byte value
```
String^ ReadUInt16FromMemory(HANDLE hProcess, BYTE* cbAddress)
{
	uint16_t u16Value = 0;

	if (!ReadProcessMemory(hProcess, reinterpret_cast<LPVOID>(cbAddress), &u16Value, sizeof(u16Value), nullptr))
	{
		return "???";
	}

	return u16Value.ToString();
}
```

#### 4-byte value
```
String^ ReadDWORDFromMemory(HANDLE hProcess, BYTE* cbAddress)
{
	DWORD dwValue = 0;

	if (!ReadProcessMemory(hProcess, reinterpret_cast<LPVOID>(cbAddress), &dwValue, sizeof(dwValue), nullptr))
	{
		return "???";
	}

	return dwValue.ToString();
}
```

#### 8-byte value
```
String^ ReadUInt64FromMemory(HANDLE hProcess, BYTE* cbAddress)
{
	uint64_t u64Value = 0;

	if (!ReadProcessMemory(hProcess, reinterpret_cast<LPVOID>(cbAddress), &u64Value, sizeof(u64Value), nullptr))
	{
		return "???";
	}

	return u64Value.ToString();
}
```

#### String value
```
String^ ReadStringFromMemory(BYTE* cbAddress)
{
	const int MAX_STRING_LENGTH = 32;
	array<BYTE>^ cbBufferArray = gcnew array<BYTE>(MAX_STRING_LENGTH);
	pin_ptr<BYTE> cbptrBuffer = &cbBufferArray[0];

	if (ReadProcessMemory(hProcess, (LPCVOID)cbAddress, cbptrBuffer, MAX_STRING_LENGTH, nullptr))
	{
		// Convert the bytes in the buffer to a string using UTF-8 encoding
		System::Text::Encoding^ encoding = System::Text::Encoding::UTF8;
		String^ strValue = encoding->GetString(cbBufferArray);

		// Remove any null characters and trim the string
		int iNullTerminatorIndex = strValue->IndexOf('\0');
		if (iNullTerminatorIndex >= 0)
		{
			strValue = strValue->Substring(0, iNullTerminatorIndex);
		}

		return strValue;
	}

	return "???";
}
```

## Memory write functions
These functions write to memory using the ```WriteProcessMemory``` function from the Windows API. Similar to
the read functions, you must also declare the process handle in order to pass it into the function.
However, for writing to strings, the declaration is slightly different:<br>

```HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);```<br>

You also need to factor in the module base address when passing in the address. Also, note that you can easily
change the encoding from UTF-8 to UTF-16. Since these functions are written for Visual C++ WinForms, a failed
write will launch a message box that will specify the error code. This format can be easily changed to work on
a CLI-based application.

#### Byte value
```
void WriteByteToMemory(HANDLE hProc, BYTE* cbAddress, BYTE bValue)
{
	if (!WriteProcessMemory(hProcess, (BYTE*)cbAddress, &bValue, sizeof(bValue), NULL))
	{
		DWORD dwError = GetLastError();
		MessageBox::Show("Failed to write to address, error code: " + dwError.ToString(), "ERROR");
	}
}
```

#### 2-byte value
```
void WriteUInt16ToMemory(HANDLE hProc, BYTE* cbAddress, uint16_t u16Value)
{
	if (!WriteProcessMemory(hProcess, (BYTE*)cbAddress, &u16Value, sizeof(u16Value), NULL))
	{
		DWORD dwError = GetLastError();
		MessageBox::Show("Failed to write to address, error code: " + dwError.ToString(), "ERROR");
	}
}
```

#### 4-byte value
```
void WriteDWORDToMemory(HANDLE hProc, BYTE* cbAddress, DWORD dwValue)
{
	if (!WriteProcessMemory(hProcess, (BYTE*)cbAddress, &dwValue, sizeof(dwValue), NULL))
	{
		DWORD dwError = GetLastError();
		MessageBox::Show("Failed to write to address, error code: " + dwError.ToString(), "ERROR");
	}
}
```

#### 8-byte value
```
void WriteUInt64ToMemory(HANDLE hProc, BYTE* cbAddress, uint64_t u64Value)
{
	if (!WriteProcessMemory(hProcess, (BYTE*)cbAddress, &u64Value, sizeof(u64Value), NULL))
	{
		DWORD dwError = GetLastError();
		MessageBox::Show("Failed to write to address, error code: " + dwError.ToString(), "ERROR");
	}
}
```

#### String value
```
void WriteStringToMemory(HANDLE hProc, BYTE* cbAddress, const char* cNewString)
{
	// Calculate the size of the string, including the null terminator
	size_t uDataSize = strlen(cNewString) + 1;

	if (!WriteProcessMemory(hProcess, (LPVOID)cbAddress, cNewString, uDataSize, 0))
	{
		DWORD dwError = GetLastError();
		MessageBox::Show("Failed to write to address, error code: " + dwError.ToString(), "ERROR");
	}
}
```

## Sources
- [Microsoft Learn - OpenProcess function (processthreadsapi.h)](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocess)
- [Microsoft Learn - ReadProcessMemory function (memoryapi.h)](https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-readprocessmemory)
- [Microsoft Learn - WriteProcessMemory function (memoryapi.h)](https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-writeprocessmemory)
- [Microsoft Learn - EnumProcessModules function (psapi.h)](https://learn.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-enumprocessmodules)
- [Microsoft Learn - Process Security and Access Rights](https://learn.microsoft.com/en-us/windows/win32/procthread/process-security-and-access-rights)
