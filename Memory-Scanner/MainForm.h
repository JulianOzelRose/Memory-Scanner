/*
	Memory-Scanner
	by Julian O. Rose
	10-25-2023
*/

#pragma once
#pragma comment(lib, "user32.lib")
#include "ProcessListForm.h"
#include "NewDescriptionForm.h"
#include "NewValueForm.h"
#include "AboutForm.h"
#include "AddAddressForm.h"
#include "ChangeTypeForm.h"
#include "MemoryValue.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <msclr/marshal_cppstd.h>

namespace MemoryScanner {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Text::RegularExpressions;
	using namespace System::Diagnostics;
	using namespace msclr::interop;
	using namespace msclr;

	public ref class MainForm : public System::Windows::Forms::Form
	{
	private:
		System::Collections::Generic::List<MemoryValue^>^ memoryValues;
	private: System::Windows::Forms::ToolStripMenuItem^ tsmiClearAddressList;




		   System::Collections::Generic::List<MemoryValue^>^ savedMemoryValues;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^ dgvMemoryAddressAddress;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^ dgvMemoryAddressValue;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^ dgvMemoryAddressPrevious;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^ dgvMemoryAddressFirst;




		   System::Collections::Generic::List<MemoryValue^>^ firstScanResult;
	public:
		MainForm(void)
		{
			InitializeComponent();

			cbScanType->SelectedIndex = 0;
			cbScanOptions->SelectedIndex = 0;
			cbValueType->SelectedIndex = 2;

			memoryValues = gcnew System::Collections::Generic::List<MemoryValue^>();
			savedMemoryValues = gcnew System::Collections::Generic::List<MemoryValue^>();
			firstScanResult = gcnew System::Collections::Generic::List<MemoryValue^>();
		}

		DWORD_PTR GetModuleBaseAddress32()
		{
			DWORD_PTR dwptrModuleBaseAddress = 0;
			HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwProcessId);

			if (hSnap != INVALID_HANDLE_VALUE)
			{
				// Convert marshaled string to C-style string
				const wchar_t* wModuleName = static_cast<const wchar_t*>(System::Runtime::InteropServices::Marshal::StringToHGlobalUni(strProcessName + ".exe").ToPointer());

				MODULEENTRY32 modEntry;
				modEntry.dwSize = sizeof(modEntry);

				if (Module32First(hSnap, &modEntry))
				{
					do
					{
						if (_wcsicmp(modEntry.szModule, wModuleName) == 0)
						{
							dwptrModuleBaseAddress = (DWORD_PTR)modEntry.modBaseAddr;
							break;
						}
					} while (Module32Next(hSnap, &modEntry));
				}

				// Free the marshaled string
				System::Runtime::InteropServices::Marshal::FreeHGlobal(IntPtr(const_cast<wchar_t*>(wModuleName)));
			}

			CloseHandle(hSnap);

			return dwptrModuleBaseAddress;
		}

		DWORD_PTR GetModuleBaseAddress64()
		{
			DWORD_PTR dwptrBaseAddress = 0;
			HANDLE hProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
			HMODULE* hModuleArray;
			LPBYTE lpbModuleArrayBytes;
			DWORD cbNeeded;

			if (hProcessHandle)
			{
				if (EnumProcessModules(hProcessHandle, NULL, 0, &cbNeeded))
				{
					if (cbNeeded)
					{
						lpbModuleArrayBytes = (LPBYTE)LocalAlloc(LPTR, cbNeeded);

						if (lpbModuleArrayBytes)
						{
							unsigned int iModuleCount;

							iModuleCount = cbNeeded / sizeof(HMODULE);
							hModuleArray = (HMODULE*)lpbModuleArrayBytes;

							if (EnumProcessModules(hProcessHandle, hModuleArray, cbNeeded, &cbNeeded))
							{
								dwptrBaseAddress = (DWORD_PTR)hModuleArray[0];
							}

							LocalFree(lpbModuleArrayBytes);
						}
					}
				}

				CloseHandle(hProcessHandle);
			}

			return dwptrBaseAddress;
		}

		String^ ReadByteFromMemory(BYTE* cbAddress)
		{
			BYTE bValue = 0;

			if (!ReadProcessMemory(hProcess, reinterpret_cast<LPVOID>(cbAddress), &bValue, sizeof(bValue), nullptr))
			{
				return "???";
			}

			return bValue.ToString();
		}

		String^ ReadUInt16FromMemory(BYTE* cbAddress)
		{
			uint16_t u16Value = 0;

			if (!ReadProcessMemory(hProcess, reinterpret_cast<LPVOID>(cbAddress), &u16Value, sizeof(u16Value), nullptr))
			{
				return "???";
			}

			return u16Value.ToString();
		}

		String^ ReadDWORDFromMemory(BYTE* cbAddress)
		{
			DWORD dwValue = 0;

			Debug::WriteLine("Reading from address: " + IntPtr(cbAddress).ToString("X"));

			if (!ReadProcessMemory(hProcess, reinterpret_cast<LPVOID>(cbAddress), &dwValue, sizeof(dwValue), nullptr))
			{
				return "???";
			}

			return dwValue.ToString();
		}

		String^ ReadUInt64FromMemory(BYTE* cbAddress)
		{
			uint64_t u64Value = 0;

			if (!ReadProcessMemory(hProcess, reinterpret_cast<LPVOID>(cbAddress), &u64Value, sizeof(u64Value), nullptr))
			{
				return "???";
			}

			return u64Value.ToString();
		}

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

		bool IsProcessX64(DWORD dwProcId)
		{
			HANDLE hProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcId);

			if (hProcessHandle != NULL)
			{
				BOOL bIsWow64 = FALSE;

				// Check if the process is running as a 32-bit process on a 64-bit system
				if (IsWow64Process(hProcessHandle, &bIsWow64))
				{
					// If bIsWow64 is true, it's a 32-bit process
					return !bIsWow64;
				}

				CloseHandle(hProcessHandle);
			}

			return false;
		}

		void ScanMemoryForStrings(String^ strSearchQuery, bool bIsCaseSensitive, bool bIsWritable)
		{
			// Update button states
			cbScanOptions->Enabled = false;
			cbScanType->Enabled = false;
			cbValueType->Enabled = false;
			txtAddressStart->Enabled = false;
			txtAddressStop->Enabled = false;
			chkWritable->Enabled = false;
			chkNot->Enabled = false;

			// Clear the DataGridView and scan result List
			dgvMemoryAddress->Rows->Clear();
			firstScanResult->Clear();

			// Open process handle
			hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, dwProcessId);

			// Set max string length to include null terminator
			const int MAX_STRING_LENGTH = strSearchQuery->Length + 1;
			array<BYTE>^ cbBufferArray = gcnew array<BYTE>(MAX_STRING_LENGTH);

			// Initialize match count
			int iNumFound = 0;
			lblFound->Text = "Found: " + iNumFound;
			bool bIsFound = false;

			double dTotalRange = static_cast<double>(MAX_ADDRESS - MIN_ADDRESS);

			// Determine the increment based on String data type
			int addressIncrement = GetAddressIncrement(4);

			for (DWORD_PTR dwptrAddress = MIN_ADDRESS; dwptrAddress <= MAX_ADDRESS; dwptrAddress += addressIncrement)
			{
				// Update progress bar
				int iPercentageComplete = static_cast<int>(((dwptrAddress - MIN_ADDRESS) / dTotalRange) * 100);
				pbScan->Value = iPercentageComplete;

				// Check if address is writable
				if (!bIsWritable)
				{
					MEMORY_BASIC_INFORMATION memInfo;
					if (VirtualQueryEx(hProcess, reinterpret_cast<LPVOID>(dwptrAddress), &memInfo, sizeof(memInfo)))
					{
						// Check if memory region is writable
						if (!(memInfo.Protect & PAGE_READWRITE) && !(memInfo.Protect & PAGE_WRITECOPY))
						{
							// Skip this address if it's not writable
							continue;
						}
					}
				}

				pin_ptr<BYTE> cbptrBuffer = &cbBufferArray[0];

				if (ReadProcessMemory(hProcess, reinterpret_cast<LPVOID>(dwptrAddress), cbptrBuffer, MAX_STRING_LENGTH, nullptr))
				{
					// Convert the buffer to a string using UTF-8 encoding
					System::Text::Encoding^ encoding = System::Text::Encoding::UTF8;
					String^ strValue = encoding->GetString(cbBufferArray);

					// Search for null-terminator in the string
					int iNullTerminatorIndex = strValue->IndexOf('\0');

					if (iNullTerminatorIndex >= 0)
					{
						strValue = strValue->Substring(0, iNullTerminatorIndex);
					}

					if (!bIsCaseSensitive)
					{
						strValue = strValue->ToLower();
						strSearchQuery = strSearchQuery->ToLower();
					}

					if (strValue->Equals(strSearchQuery))
					{
						iNumFound++;
						lblFound->Text = "Found: " + iNumFound;
						bIsFound = true;
					}
					else if (strValue->Contains(strSearchQuery))
					{
						iNumFound++;
						lblFound->Text = "Found: " + iNumFound;
						bIsFound = true;
					}

					if (bIsFound)
					{
						int iRowIndexBeforeAdd = dgvMemoryAddress->RowCount;

						dgvMemoryAddress->Rows->Add(String::Format(strProcessName + ".exe + 0x{0:X}",
							static_cast<unsigned long long>(dwptrAddress - dwModuleBaseAddress)), strValue, nullptr);

						int iRowIndexAfterAdd = dgvMemoryAddress->RowCount - 1;

						// Initialize MemoryValue object with appropriate values
						MemoryValue^ newMemoryValue = gcnew MemoryValue();
						newMemoryValue->Address = (BYTE*)dwptrAddress;
						newMemoryValue->Value = strValue;
						newMemoryValue->Type = 4;
						newMemoryValue->ModuleName = strProcessName + ".exe";

						// Add MemoryValue object to List
						memoryValues->Add(newMemoryValue);
						firstScanResult->Add(newMemoryValue);

						// Associate the MemoryValue object with the DataGridView row
						dgvMemoryAddress->Rows[iRowIndexAfterAdd]->Tag = newMemoryValue;
					}
				}

				Application::DoEvents();
			}

			// Reset progress bar
			pbScan->Value = 0;

			// Update button states
			cbScanOptions->Enabled = true;
			cbScanType->Enabled = true;
			cbValueType->Enabled = true;
			txtAddressStart->Enabled = true;
			txtAddressStop->Enabled = true;
			chkWritable->Enabled = true;
			chkNot->Enabled = true;
			btnNextScan->Enabled = true;
			btnFirstScan->Text = "New Scan";
		}

		void ScanMemoryForNumbers(uint64_t u64SearchQuery, int iValueType, int iScanType, bool bIsWritable)
		{
			// Clear the DataGridView and scan result List
			dgvMemoryAddress->Rows->Clear();
			firstScanResult->Clear();

			// Update button states
			cbScanOptions->Enabled = false;
			cbScanType->Enabled = false;
			cbValueType->Enabled = false;
			txtAddressStart->Enabled = false;
			txtAddressStop->Enabled = false;
			chkWritable->Enabled = false;
			chkNot->Enabled = false;

			hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, dwProcessId);

			BYTE* cbCurrentAddress = new BYTE[sizeof(BYTE)];

			if (!cbCurrentAddress)
			{
				// Handle memory allocation error
				delete[] cbCurrentAddress;
				return;
			}

			// Initialize match count
			int iNumFound = 0;
			lblFound->Text = "Found: " + iNumFound;

			// Calculate the total range as a percentage
			double dTotalRange = static_cast<double>(MAX_ADDRESS - MIN_ADDRESS);
			int iPercentageComplete = 0;

			// Determine the increment based on the selected data type
			int addressIncrement = GetAddressIncrement(iValueType);

			for (DWORD_PTR dwptrAddress = MIN_ADDRESS; dwptrAddress <= MAX_ADDRESS; dwptrAddress += addressIncrement)
			{
				// Calculate the percentage based on the actual range
				iPercentageComplete = static_cast<int>(((dwptrAddress - MIN_ADDRESS) / dTotalRange) * 100);

				// Set the progress bar value directly to the percentage
				pbScan->Value = iPercentageComplete;

				// Check if the address is writable
				if (!bIsWritable)
				{
					MEMORY_BASIC_INFORMATION memInfo;
					if (VirtualQueryEx(hProcess, reinterpret_cast<LPVOID>(dwptrAddress), &memInfo, sizeof(memInfo)))
					{
						// Check if the memory region is writable
						if (!(memInfo.Protect & PAGE_READWRITE) && !(memInfo.Protect & PAGE_WRITECOPY))
						{
							// Skip this address if it's not writable
							continue;
						}
					}
				}

				// Initialize data types for ReadProcessMemory
				BYTE bValue;
				uint16_t u16Value;
				DWORD dwValue;
				uint64_t u64Value;
				uint64_t uValueToCompare;

				bool bIsMatch;
				int iRetCode;

				if (iValueType == 0)
				{
					iRetCode = ReadProcessMemory(hProcess, reinterpret_cast<LPVOID>(dwptrAddress), &bValue, sizeof(bValue), nullptr);
				}
				else if (iValueType == 1)
				{
					iRetCode = ReadProcessMemory(hProcess, reinterpret_cast<LPVOID>(dwptrAddress), &u16Value, sizeof(u16Value), nullptr);
				}
				else if (iValueType == 2)
				{
					iRetCode = ReadProcessMemory(hProcess, reinterpret_cast<LPVOID>(dwptrAddress), &dwValue, sizeof(dwValue), nullptr);
				}
				else if (iValueType == 3)
				{
					iRetCode = ReadProcessMemory(hProcess, reinterpret_cast<LPVOID>(dwptrAddress), &u64Value, sizeof(u64Value), nullptr);
				}

				// Process result if ReadProcessMemory success
				if (iRetCode != -1)
				{
					// Convert the address to a hexadecimal string
					String^ strHexAddress = String::Format(strProcessName + ".exe + 0x{0:X}", static_cast<unsigned long long>(dwptrAddress - dwModuleBaseAddress));

					if (iValueType == 0)
					{
						uValueToCompare = (BYTE)bValue;
					}
					else if (iValueType == 1)
					{
						uValueToCompare = (uint16_t)u16Value;
					}
					else if (iValueType == 2)
					{
						uValueToCompare = (DWORD)dwValue;

					}
					else if (iValueType == 3)
					{
						uValueToCompare = (uint64_t)dwValue;
					}

					if (iScanType == 0)
					{
						// Exact value
						bIsMatch = (uValueToCompare == u64SearchQuery);
					}
					else if (iScanType == 1)
					{
						// Bigger than
						bIsMatch = (uValueToCompare > u64SearchQuery);
					}
					else if (iScanType == 2)
					{
						// Smaller than
						bIsMatch = (uValueToCompare < u64SearchQuery);
					}


					// If "Not" is checked, invert bIsMatch result
					if (chkNot->Checked)
					{
						bIsMatch = !bIsMatch;
					}

					if (bIsMatch)
					{
						// Update match counter
						iNumFound++;
						lblFound->Text = "Found: " + iNumFound;

						// Initialize MemoryValue object with appropriate values
						MemoryValue^ newMemoryValue = gcnew MemoryValue();
						newMemoryValue->Address = (BYTE*)dwptrAddress;
						newMemoryValue->Type = iValueType;
						newMemoryValue->IsModuleBaseIncluded = false;
						newMemoryValue->ModuleName = strProcessName + ".exe";

						// Set MemoryValue object's value to correct data type
						if (iValueType == 0)
						{
							newMemoryValue->Value = bValue;
						}
						else if (iValueType == 1)
						{
							newMemoryValue->Value = u16Value;
						}
						else if (iValueType == 2)
						{
							newMemoryValue->Value = dwValue;
						}
						else if (iValueType == 3)
						{
							newMemoryValue->Value = u64Value;
						}

						// Add MemoryValue object to List
						memoryValues->Add(newMemoryValue);
						firstScanResult->Add(newMemoryValue);

						// Create a new DataGridView row
						int iRowIndex = dgvMemoryAddress->Rows->Add(strHexAddress, dwValue, nullptr);

						// Associate the MemoryValue object with the DataGridView row
						dgvMemoryAddress->Rows[iRowIndex]->Tag = newMemoryValue;
					}
				}

				Application::DoEvents();
			}

			// Reset progress bar
			pbScan->Value = 0;

			// Update Button states
			btnFirstScan->Text = "New Scan";
			cbScanOptions->Enabled = true;
			cbScanType->Enabled = true;
			cbValueType->Enabled = true;
			txtAddressStart->Enabled = true;
			txtAddressStop->Enabled = true;
			chkWritable->Enabled = true;
			chkNot->Enabled = true;
			//btnNextScan->Enabled = true;
		}

		int GetAddressIncrement(int iValueType)
		{
			int addressIncrement = 0;

			switch (iValueType)
			{
			case 0: // BYTE
				addressIncrement = 1;
				break;
			case 1: // 2 bytes
				addressIncrement = 2;
				break;
			case 2: // 4 bytes
				addressIncrement = 4;
				break;
			case 3: // 8 bytes
				addressIncrement = 8;
				break;
			case 4: // String
				addressIncrement = 2;
				break;
			}

			return addressIncrement;
		}

		void ClearAddressList()
		{
			System::Windows::Forms::DialogResult dlgResult = MessageBox::Show(
				"Are you sure you want to delete all addresses?",
				"Confirmation",
				MessageBoxButtons::YesNo,
				MessageBoxIcon::Question
			);

			// Check if the dialog result is Yes
			if (dlgResult == System::Windows::Forms::DialogResult::Yes)
			{
				dgvAddressList->Rows->Clear();
				btnDeleteAddresses->Enabled = false;
			}
		}

		BYTE* ExtractHexAddress(String^ strAddress)
		{
			try
			{
				if (strAddress->StartsWith("0x", StringComparison::OrdinalIgnoreCase))
				{
					// Remove the "0x" prefix if it exists
					strAddress = strAddress->Substring(2);
				}

				System::UInt64 uParsedValue = System::UInt64::Parse(strAddress, System::Globalization::NumberStyles::HexNumber);

				// Create a BYTE* pointer and assign the parsed address directly
				BYTE* cbValue = reinterpret_cast<BYTE*>(uParsedValue);

				return cbValue;
			}
			catch (FormatException^)
			{
				// Handle the case where the input string is not a valid hexadecimal value
				return nullptr;
			}
		}

		void CopyToClipboard(String^ strTextToCopy)
		{
			// Convert System::String to std::wstring
			std::wstring wstrTextToCopy = msclr::interop::marshal_as<std::wstring>(strTextToCopy);

			if (OpenClipboard(nullptr))
			{
				// Clear the clipboard.
				EmptyClipboard();

				// Allocate global memory for the string.
				HGLOBAL hClipboardData = GlobalAlloc(GMEM_MOVEABLE, (wstrTextToCopy.size() + 1) * sizeof(wchar_t));
				if (hClipboardData)
				{
					// Lock the memory and copy the string.
					wchar_t* wClipboardData = static_cast<wchar_t*>(GlobalLock(hClipboardData));
					wcscpy_s(wClipboardData, wstrTextToCopy.size() + 1, wstrTextToCopy.c_str());
					GlobalUnlock(hClipboardData);

					// Set the data to the clipboard.
					SetClipboardData(CF_UNICODETEXT, hClipboardData);
				}

				CloseClipboard();
			}
		}

		String^ BytePointerToHexString(BYTE* cbBytePointer)
		{
			return String::Format("0x{0:X}", reinterpret_cast<unsigned long long>(cbBytePointer));
		}

		void UpdateAddressListValues(DataGridView^ dgv, int iValueColumnIndex)
		{
			for (int i = 0; i < dgv->RowCount; i++)
			{
				DataGridViewRow^ dgvRow = dgv->Rows[i];
				MemoryValue^ memoryValue = dynamic_cast<MemoryValue^>(dgvRow->Tag);
				BYTE* cbAddress = memoryValue->Address;
				String^ strCurrentValue;

				if (memoryValue != nullptr)
				{
					if (memoryValue->Type == 0)
					{
						// Read BYTE number from memory
						strCurrentValue = ReadByteFromMemory(dwModuleBaseAddress + cbAddress);
					}
					else if (memoryValue->Type == 1)
					{
						// Read 2-byte number from memory
						strCurrentValue = ReadUInt16FromMemory(dwModuleBaseAddress + cbAddress);
					}
					else if (memoryValue->Type == 2)
					{
						// Read 4-byte from memory
						strCurrentValue = ReadDWORDFromMemory(dwModuleBaseAddress + cbAddress);
					}
					else if (memoryValue->Type == 3)
					{
						// Read 8-byte number from memory
						strCurrentValue = ReadUInt64FromMemory(dwModuleBaseAddress + cbAddress);
					}
					else if (memoryValue->Type == 4)
					{
						// Read string from memory
						strCurrentValue = ReadStringFromMemory((BYTE*)(dwModuleBaseAddress + cbAddress));
					}

					// Update DataGridView cell value
					dgv->Rows[i]->Cells[iValueColumnIndex]->Value = strCurrentValue;
				}
			}
		}

		void UpdateMemoryAddressValues(DataGridView^ dgv, int iValueColumnIndex)
		{
			for (int i = 0; i < dgv->RowCount; i++)
			{
				DataGridViewRow^ dgvRow = dgv->Rows[i];
				MemoryValue^ memoryValue = dynamic_cast<MemoryValue^>(dgvRow->Tag);
				String^ strCurrentValue;
				BYTE* cbAddress = memoryValue->Address;

				if (memoryValue != nullptr)
				{
					if (memoryValue->Type == 0)
					{
						// Read BYTE number from memory
						strCurrentValue = ReadByteFromMemory(cbAddress);
					}
					else if (memoryValue->Type == 1)
					{
						// Read 2-byte number from memory
						strCurrentValue = ReadUInt16FromMemory(cbAddress);
					}
					else if (memoryValue->Type == 2)
					{
						// Read 4-byte number from memory
						strCurrentValue = ReadDWORDFromMemory(cbAddress);
					}
					else if (memoryValue->Type == 3)
					{
						// Read 8-byte number from memory
						strCurrentValue = ReadUInt64FromMemory(cbAddress);
					}
					else if (memoryValue->Type == 4)
					{
						// Read string from memory
						strCurrentValue = ReadStringFromMemory((BYTE*)(memoryValue->Address));
					}

					// Update DataGridView cell
					dgv->Rows[i]->Cells[iValueColumnIndex]->Value = strCurrentValue;
				}
			}
		}

		void AddSelectedAddressesToAddressList()
		{
			DataGridViewSelectedRowCollection^ dgvSelectedRows = dgvMemoryAddress->SelectedRows;

			for (int i = 0; i < dgvSelectedRows->Count; i++)
			{
				DataGridViewRow^ dgvSelectedRow = dgvSelectedRows[i];
				int iNewRowIndex = dgvAddressList->Rows->Add();

				// Retrieve the MemoryValue object associated with the selected row and replicate it
				MemoryValue^ selectedMemoryValue = dynamic_cast<MemoryValue^>(dgvSelectedRow->Tag);
				MemoryValue^ newMemoryValue = gcnew MemoryValue();

				// Get the address from the dgvMemoryAddress cell
				String^ strOriginalAddress = dgvSelectedRow->Cells["dgvMemoryAddressAddress"]->Value->ToString();

				// Extract the part after module name to get the hex address
				int iAddressStart = strOriginalAddress->IndexOf(".exe + ");
				String^ strHexValue;

				if (iAddressStart != -1)
				{
					strHexValue = strOriginalAddress->Substring(iAddressStart + 7);
				}
				else
				{
					strHexValue = strOriginalAddress;
				}

				// Format hexadecimal address
				strHexValue = strHexValue->Substring(2);
				DWORD dwOffset = Int32::Parse(strHexValue, System::Globalization::NumberStyles::HexNumber);
				DWORD dwAddress = dwModuleBaseAddress + dwOffset;
				String^ strFormattedHexAddress = "0x" + dwAddress.ToString("X");

				// Get column index
				int iValueColumnIndex = dgvMemoryAddress->Columns["dgvMemoryAddressValue"]->Index;

				// Set the properties of the new MemoryValue object
				newMemoryValue->ModuleName = strProcessName + ".exe";
				newMemoryValue->Address = reinterpret_cast<BYTE*>(dwOffset);
				newMemoryValue->Type = selectedMemoryValue->Type;
				newMemoryValue->Value = dgvSelectedRow->Cells[iValueColumnIndex]->Value;

				// Associate the MemoryValue object with the DataGridView row
				dgvAddressList->Rows[iNewRowIndex]->Tag = newMemoryValue;

				// Set the value in new DataGridView cell
				dgvAddressList->Rows[iNewRowIndex]->Cells["dgvMemoryListAddress"]->Value = strFormattedHexAddress;
				dgvAddressList->Rows[iNewRowIndex]->Cells["dgvMemoryListValue"]->Value = dgvSelectedRow->Cells[iValueColumnIndex]->Value;
				dgvAddressList->Rows[iNewRowIndex]->Cells["dgvMemoryListDescription"]->Value = "No description";
				dgvAddressList->Rows[iNewRowIndex]->Cells["dgvMemoryListType"]->Value = cbValueType->SelectedItem->ToString();
			}

			btnDeleteAddresses->Enabled = true;
		}

		String^ GetTypeString(int iDataType)
		{
			if (iDataType == 0)
			{
				return "Byte";
			}
			else if (iDataType == 1)
			{
				return "2 Bytes";
			}
			else if (iDataType == 2)
			{
				return "4 Bytes";
			}
			else if (iDataType == 3)
			{
				return "8 Bytes";
			}
			else if (iDataType == 4)
			{
				return "String";
			}

			return "Unknown";
		}

	protected:
		~MainForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private:
		DWORD_PTR MIN_ADDRESS;
		DWORD_PTR MAX_ADDRESS;
		DWORD_PTR dwModuleBaseAddress;
		DWORD dwProcessId = -1;
		HANDLE hProcess;
		String^ strProcessName;
	private: System::Windows::Forms::DataGridView^ dgvMemoryAddress;
	private: System::Windows::Forms::ToolStrip^ toolStripMain;
	private: System::Windows::Forms::ToolStripDropDownButton^ tsddbFile;
	private: System::Windows::Forms::ToolStripMenuItem^ tsmiOpenProcess;
	private: System::Windows::Forms::ToolStripSeparator^ tsmiFileSeparator;

	protected:
	protected:

	private: System::Windows::Forms::ToolStripMenuItem^ tsmiExit;
	private: System::Windows::Forms::Button^ btnFirstScan;
	private: System::Windows::Forms::Button^ btnNextScan;
	private: System::Windows::Forms::Button^ btnAddAddressManually;
	private: System::Windows::Forms::TextBox^ txtValue;
	private: System::Windows::Forms::Label^ lblValue;

	private: System::Windows::Forms::ComboBox^ cbValueType;
	private: System::Windows::Forms::Label^ lblValueType;

	private: System::Windows::Forms::Label^ lblScanType;

	private: System::Windows::Forms::ComboBox^ cbScanType;
	private: System::Windows::Forms::GroupBox^ grpMemoryScanOptions;

	private: System::Windows::Forms::ComboBox^ cbScanOptions;
	private: System::Windows::Forms::TextBox^ txtAddressStop;
	private: System::Windows::Forms::TextBox^ txtAddressStart;
	private: System::Windows::Forms::DataGridView^ dgvAddressList;

	private: System::Windows::Forms::ProgressBar^ pbScan;
	private: System::Windows::Forms::Label^ lblProcessInfo;








	private: System::Windows::Forms::Button^ btnDeleteAddresses;
	private: System::Windows::Forms::Label^ lblFound;
	private: System::Windows::Forms::CheckBox^ chkWritable;
	private: System::Windows::Forms::CheckBox^ chkNot;
	private: System::Windows::Forms::ToolStripDropDownButton^ tsddbHelp;
	private: System::Windows::Forms::ToolStripMenuItem^ tsmiAbout;





	private: System::Windows::Forms::ContextMenuStrip^ cmsMemoryAddress;
	private: System::Windows::Forms::ToolStripMenuItem^ tsmiAddToAddressList;
	private: System::Windows::Forms::ToolStripMenuItem^ tsmiChangeValueOfSelectedAddresses;
	private: System::Windows::Forms::ToolStripMenuItem^ tsmiCopySelectedAddresses;
	private: System::Windows::Forms::ContextMenuStrip^ cmsAddressList;
	private: System::Windows::Forms::ToolStripMenuItem^ tsmiDeleteThisRecord;
	private: System::Windows::Forms::ToolStripMenuItem^ tsmiChangeRecord;
	private: System::Windows::Forms::ToolStripMenuItem^ tsmiChangeDescription;
	private: System::Windows::Forms::ToolStripMenuItem^ tsmiChangeAddress;
	private: System::Windows::Forms::ToolStripMenuItem^ tsmiChangeType;
	private: System::Windows::Forms::ToolStripMenuItem^ tsmiChangeValue;




	private: System::Windows::Forms::Timer^ tmrUpdateValues;



	private: System::Windows::Forms::ToolStripMenuItem^ tsmiViewREADME;
	private: System::Windows::Forms::ToolStripSeparator^ tsmiHelpSeparator;




	private: System::Windows::Forms::DataGridViewTextBoxColumn^ dgvMemoryListDescription;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^ dgvMemoryListAddress;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^ dgvMemoryListType;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^ dgvMemoryListValue;









	private: System::ComponentModel::IContainer^ components;

	private:


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(MainForm::typeid));
			this->dgvMemoryAddress = (gcnew System::Windows::Forms::DataGridView());
			this->cmsMemoryAddress = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->tsmiAddToAddressList = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->tsmiChangeValueOfSelectedAddresses = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->tsmiCopySelectedAddresses = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->toolStripMain = (gcnew System::Windows::Forms::ToolStrip());
			this->tsddbFile = (gcnew System::Windows::Forms::ToolStripDropDownButton());
			this->tsmiOpenProcess = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->tsmiClearAddressList = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->tsmiFileSeparator = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->tsmiExit = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->tsddbHelp = (gcnew System::Windows::Forms::ToolStripDropDownButton());
			this->tsmiViewREADME = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->tsmiHelpSeparator = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->tsmiAbout = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->btnFirstScan = (gcnew System::Windows::Forms::Button());
			this->btnNextScan = (gcnew System::Windows::Forms::Button());
			this->txtValue = (gcnew System::Windows::Forms::TextBox());
			this->lblValue = (gcnew System::Windows::Forms::Label());
			this->cbValueType = (gcnew System::Windows::Forms::ComboBox());
			this->lblValueType = (gcnew System::Windows::Forms::Label());
			this->lblScanType = (gcnew System::Windows::Forms::Label());
			this->cbScanType = (gcnew System::Windows::Forms::ComboBox());
			this->grpMemoryScanOptions = (gcnew System::Windows::Forms::GroupBox());
			this->chkWritable = (gcnew System::Windows::Forms::CheckBox());
			this->txtAddressStop = (gcnew System::Windows::Forms::TextBox());
			this->txtAddressStart = (gcnew System::Windows::Forms::TextBox());
			this->cbScanOptions = (gcnew System::Windows::Forms::ComboBox());
			this->dgvAddressList = (gcnew System::Windows::Forms::DataGridView());
			this->dgvMemoryListDescription = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->dgvMemoryListAddress = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->dgvMemoryListType = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->dgvMemoryListValue = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->cmsAddressList = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->tsmiDeleteThisRecord = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->tsmiChangeRecord = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->tsmiChangeDescription = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->tsmiChangeAddress = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->tsmiChangeType = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->tsmiChangeValue = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->pbScan = (gcnew System::Windows::Forms::ProgressBar());
			this->lblProcessInfo = (gcnew System::Windows::Forms::Label());
			this->btnDeleteAddresses = (gcnew System::Windows::Forms::Button());
			this->lblFound = (gcnew System::Windows::Forms::Label());
			this->chkNot = (gcnew System::Windows::Forms::CheckBox());
			this->tmrUpdateValues = (gcnew System::Windows::Forms::Timer(this->components));
			this->btnAddAddressManually = (gcnew System::Windows::Forms::Button());
			this->dgvMemoryAddressAddress = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->dgvMemoryAddressValue = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->dgvMemoryAddressPrevious = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->dgvMemoryAddressFirst = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dgvMemoryAddress))->BeginInit();
			this->cmsMemoryAddress->SuspendLayout();
			this->toolStripMain->SuspendLayout();
			this->grpMemoryScanOptions->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dgvAddressList))->BeginInit();
			this->cmsAddressList->SuspendLayout();
			this->SuspendLayout();
			// 
			// dgvMemoryAddress
			// 
			this->dgvMemoryAddress->AllowUserToAddRows = false;
			this->dgvMemoryAddress->AllowUserToResizeColumns = false;
			this->dgvMemoryAddress->AllowUserToResizeRows = false;
			this->dgvMemoryAddress->BackgroundColor = System::Drawing::SystemColors::ControlLightLight;
			this->dgvMemoryAddress->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->dgvMemoryAddress->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(4) {
				this->dgvMemoryAddressAddress,
					this->dgvMemoryAddressValue, this->dgvMemoryAddressPrevious, this->dgvMemoryAddressFirst
			});
			this->dgvMemoryAddress->ContextMenuStrip = this->cmsMemoryAddress;
			this->dgvMemoryAddress->Location = System::Drawing::Point(13, 84);
			this->dgvMemoryAddress->Name = L"dgvMemoryAddress";
			this->dgvMemoryAddress->ReadOnly = true;
			this->dgvMemoryAddress->RowHeadersVisible = false;
			this->dgvMemoryAddress->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::FullRowSelect;
			this->dgvMemoryAddress->Size = System::Drawing::Size(649, 282);
			this->dgvMemoryAddress->TabIndex = 0;
			this->dgvMemoryAddress->CellMouseDoubleClick += gcnew System::Windows::Forms::DataGridViewCellMouseEventHandler(this, &MainForm::dgvMemoryAddress_CellMouseDoubleClick);
			this->dgvMemoryAddress->RowsAdded += gcnew System::Windows::Forms::DataGridViewRowsAddedEventHandler(this, &MainForm::dgvMemoryAddress_RowsAdded);
			this->dgvMemoryAddress->RowsRemoved += gcnew System::Windows::Forms::DataGridViewRowsRemovedEventHandler(this, &MainForm::dgvMemoryAddress_RowsRemoved);
			this->dgvMemoryAddress->UserDeletingRow += gcnew System::Windows::Forms::DataGridViewRowCancelEventHandler(this, &MainForm::dgvMemoryAddress_UserDeletingRow);
			// 
			// cmsMemoryAddress
			// 
			this->cmsMemoryAddress->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {
				this->tsmiAddToAddressList,
					this->tsmiChangeValueOfSelectedAddresses, this->tsmiCopySelectedAddresses
			});
			this->cmsMemoryAddress->Name = L"cmsMemoryAddress";
			this->cmsMemoryAddress->Size = System::Drawing::Size(272, 70);
			// 
			// tsmiAddToAddressList
			// 
			this->tsmiAddToAddressList->Enabled = false;
			this->tsmiAddToAddressList->Name = L"tsmiAddToAddressList";
			this->tsmiAddToAddressList->Size = System::Drawing::Size(271, 22);
			this->tsmiAddToAddressList->Text = L"Add selected addresses to address list";
			this->tsmiAddToAddressList->Click += gcnew System::EventHandler(this, &MainForm::tsmiAddToAddressList_Click);
			// 
			// tsmiChangeValueOfSelectedAddresses
			// 
			this->tsmiChangeValueOfSelectedAddresses->Enabled = false;
			this->tsmiChangeValueOfSelectedAddresses->Name = L"tsmiChangeValueOfSelectedAddresses";
			this->tsmiChangeValueOfSelectedAddresses->Size = System::Drawing::Size(271, 22);
			this->tsmiChangeValueOfSelectedAddresses->Text = L"Change value of selected addresses";
			// 
			// tsmiCopySelectedAddresses
			// 
			this->tsmiCopySelectedAddresses->Enabled = false;
			this->tsmiCopySelectedAddresses->Name = L"tsmiCopySelectedAddresses";
			this->tsmiCopySelectedAddresses->Size = System::Drawing::Size(271, 22);
			this->tsmiCopySelectedAddresses->Text = L"Copy selected addresses";
			this->tsmiCopySelectedAddresses->Click += gcnew System::EventHandler(this, &MainForm::tsmiCopySelectedAddresses_Click);
			// 
			// toolStripMain
			// 
			this->toolStripMain->GripStyle = System::Windows::Forms::ToolStripGripStyle::Hidden;
			this->toolStripMain->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) { this->tsddbFile, this->tsddbHelp });
			this->toolStripMain->Location = System::Drawing::Point(0, 0);
			this->toolStripMain->Name = L"toolStripMain";
			this->toolStripMain->Size = System::Drawing::Size(986, 25);
			this->toolStripMain->TabIndex = 1;
			this->toolStripMain->Text = L"toolStrip";
			// 
			// tsddbFile
			// 
			this->tsddbFile->AutoToolTip = false;
			this->tsddbFile->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->tsddbFile->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {
				this->tsmiOpenProcess,
					this->tsmiClearAddressList, this->tsmiFileSeparator, this->tsmiExit
			});
			this->tsddbFile->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"tsddbFile.Image")));
			this->tsddbFile->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->tsddbFile->Name = L"tsddbFile";
			this->tsddbFile->ShowDropDownArrow = false;
			this->tsddbFile->Size = System::Drawing::Size(29, 22);
			this->tsddbFile->Text = L"File";
			// 
			// tsmiOpenProcess
			// 
			this->tsmiOpenProcess->Name = L"tsmiOpenProcess";
			this->tsmiOpenProcess->Size = System::Drawing::Size(167, 22);
			this->tsmiOpenProcess->Text = L"Open Process";
			this->tsmiOpenProcess->Click += gcnew System::EventHandler(this, &MainForm::tsmiOpenProcess_Click);
			// 
			// tsmiClearAddressList
			// 
			this->tsmiClearAddressList->Name = L"tsmiClearAddressList";
			this->tsmiClearAddressList->Size = System::Drawing::Size(167, 22);
			this->tsmiClearAddressList->Text = L"Clear Address List";
			this->tsmiClearAddressList->Click += gcnew System::EventHandler(this, &MainForm::tsmiClearAddressList_Click);
			// 
			// tsmiFileSeparator
			// 
			this->tsmiFileSeparator->Name = L"tsmiFileSeparator";
			this->tsmiFileSeparator->Size = System::Drawing::Size(164, 6);
			// 
			// tsmiExit
			// 
			this->tsmiExit->Name = L"tsmiExit";
			this->tsmiExit->Size = System::Drawing::Size(167, 22);
			this->tsmiExit->Text = L"Exit";
			this->tsmiExit->Click += gcnew System::EventHandler(this, &MainForm::tsmiExit_Click);
			// 
			// tsddbHelp
			// 
			this->tsddbHelp->AutoToolTip = false;
			this->tsddbHelp->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->tsddbHelp->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {
				this->tsmiViewREADME,
					this->tsmiHelpSeparator, this->tsmiAbout
			});
			this->tsddbHelp->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"tsddbHelp.Image")));
			this->tsddbHelp->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->tsddbHelp->Name = L"tsddbHelp";
			this->tsddbHelp->ShowDropDownArrow = false;
			this->tsddbHelp->Size = System::Drawing::Size(36, 22);
			this->tsddbHelp->Text = L"Help";
			// 
			// tsmiViewREADME
			// 
			this->tsmiViewREADME->Name = L"tsmiViewREADME";
			this->tsmiViewREADME->Size = System::Drawing::Size(148, 22);
			this->tsmiViewREADME->Text = L"View README";
			this->tsmiViewREADME->Click += gcnew System::EventHandler(this, &MainForm::tsmiViewREADME_Click);
			// 
			// tsmiHelpSeparator
			// 
			this->tsmiHelpSeparator->Name = L"tsmiHelpSeparator";
			this->tsmiHelpSeparator->Size = System::Drawing::Size(145, 6);
			// 
			// tsmiAbout
			// 
			this->tsmiAbout->Name = L"tsmiAbout";
			this->tsmiAbout->Size = System::Drawing::Size(148, 22);
			this->tsmiAbout->Text = L"About";
			this->tsmiAbout->Click += gcnew System::EventHandler(this, &MainForm::tsmiAbout_Click);
			// 
			// btnFirstScan
			// 
			this->btnFirstScan->Enabled = false;
			this->btnFirstScan->Location = System::Drawing::Point(697, 84);
			this->btnFirstScan->Name = L"btnFirstScan";
			this->btnFirstScan->Size = System::Drawing::Size(75, 23);
			this->btnFirstScan->TabIndex = 2;
			this->btnFirstScan->Text = L"First Scan";
			this->btnFirstScan->UseVisualStyleBackColor = true;
			this->btnFirstScan->Click += gcnew System::EventHandler(this, &MainForm::btnFirstScan_Click);
			// 
			// btnNextScan
			// 
			this->btnNextScan->Enabled = false;
			this->btnNextScan->Location = System::Drawing::Point(778, 84);
			this->btnNextScan->Name = L"btnNextScan";
			this->btnNextScan->Size = System::Drawing::Size(75, 23);
			this->btnNextScan->TabIndex = 3;
			this->btnNextScan->Text = L"Next Scan";
			this->btnNextScan->UseVisualStyleBackColor = true;
			// 
			// txtValue
			// 
			this->txtValue->Location = System::Drawing::Point(743, 121);
			this->txtValue->Name = L"txtValue";
			this->txtValue->Size = System::Drawing::Size(229, 20);
			this->txtValue->TabIndex = 4;
			// 
			// lblValue
			// 
			this->lblValue->AutoSize = true;
			this->lblValue->Location = System::Drawing::Point(694, 124);
			this->lblValue->Name = L"lblValue";
			this->lblValue->Size = System::Drawing::Size(37, 13);
			this->lblValue->TabIndex = 5;
			this->lblValue->Text = L"Value:";
			// 
			// cbValueType
			// 
			this->cbValueType->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cbValueType->FormattingEnabled = true;
			this->cbValueType->Items->AddRange(gcnew cli::array< System::Object^  >(5) {
				L"Byte", L"2 Bytes", L"4 Bytes", L"8 Bytes",
					L"String"
			});
			this->cbValueType->Location = System::Drawing::Point(749, 189);
			this->cbValueType->Name = L"cbValueType";
			this->cbValueType->Size = System::Drawing::Size(121, 21);
			this->cbValueType->TabIndex = 6;
			this->cbValueType->SelectedIndexChanged += gcnew System::EventHandler(this, &MainForm::cbValueType_SelectedIndexChanged);
			// 
			// lblValueType
			// 
			this->lblValueType->AutoSize = true;
			this->lblValueType->Location = System::Drawing::Point(677, 192);
			this->lblValueType->Name = L"lblValueType";
			this->lblValueType->Size = System::Drawing::Size(64, 13);
			this->lblValueType->TabIndex = 7;
			this->lblValueType->Text = L"Value Type:";
			// 
			// lblScanType
			// 
			this->lblScanType->AutoSize = true;
			this->lblScanType->Location = System::Drawing::Point(677, 165);
			this->lblScanType->Name = L"lblScanType";
			this->lblScanType->Size = System::Drawing::Size(62, 13);
			this->lblScanType->TabIndex = 9;
			this->lblScanType->Text = L"Scan Type:";
			// 
			// cbScanType
			// 
			this->cbScanType->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cbScanType->FormattingEnabled = true;
			this->cbScanType->Items->AddRange(gcnew cli::array< System::Object^  >(3) { L"Exact Value", L"Bigger than...", L"Smaller than..." });
			this->cbScanType->Location = System::Drawing::Point(749, 162);
			this->cbScanType->Name = L"cbScanType";
			this->cbScanType->Size = System::Drawing::Size(121, 21);
			this->cbScanType->TabIndex = 8;
			// 
			// grpMemoryScanOptions
			// 
			this->grpMemoryScanOptions->Controls->Add(this->chkWritable);
			this->grpMemoryScanOptions->Controls->Add(this->txtAddressStop);
			this->grpMemoryScanOptions->Controls->Add(this->txtAddressStart);
			this->grpMemoryScanOptions->Controls->Add(this->cbScanOptions);
			this->grpMemoryScanOptions->Location = System::Drawing::Point(697, 224);
			this->grpMemoryScanOptions->Name = L"grpMemoryScanOptions";
			this->grpMemoryScanOptions->Size = System::Drawing::Size(215, 142);
			this->grpMemoryScanOptions->TabIndex = 10;
			this->grpMemoryScanOptions->TabStop = false;
			this->grpMemoryScanOptions->Text = L"Memory Scan Options";
			// 
			// chkWritable
			// 
			this->chkWritable->AutoSize = true;
			this->chkWritable->Checked = true;
			this->chkWritable->CheckState = System::Windows::Forms::CheckState::Checked;
			this->chkWritable->Location = System::Drawing::Point(11, 109);
			this->chkWritable->Name = L"chkWritable";
			this->chkWritable->Size = System::Drawing::Size(65, 17);
			this->chkWritable->TabIndex = 15;
			this->chkWritable->Text = L"Writable";
			this->chkWritable->UseVisualStyleBackColor = true;
			// 
			// txtAddressStop
			// 
			this->txtAddressStop->Location = System::Drawing::Point(45, 72);
			this->txtAddressStop->Name = L"txtAddressStop";
			this->txtAddressStop->Size = System::Drawing::Size(161, 20);
			this->txtAddressStop->TabIndex = 13;
			this->txtAddressStop->Text = L"00007FFFFFFFFFFF";
			// 
			// txtAddressStart
			// 
			this->txtAddressStart->Location = System::Drawing::Point(45, 46);
			this->txtAddressStart->Name = L"txtAddressStart";
			this->txtAddressStart->Size = System::Drawing::Size(161, 20);
			this->txtAddressStart->TabIndex = 12;
			this->txtAddressStart->Text = L"0000000000000000";
			// 
			// cbScanOptions
			// 
			this->cbScanOptions->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cbScanOptions->FormattingEnabled = true;
			this->cbScanOptions->Items->AddRange(gcnew cli::array< System::Object^  >(1) { L"All" });
			this->cbScanOptions->Location = System::Drawing::Point(6, 19);
			this->cbScanOptions->Name = L"cbScanOptions";
			this->cbScanOptions->Size = System::Drawing::Size(200, 21);
			this->cbScanOptions->TabIndex = 11;
			// 
			// dgvAddressList
			// 
			this->dgvAddressList->AllowUserToAddRows = false;
			this->dgvAddressList->AllowUserToResizeColumns = false;
			this->dgvAddressList->AllowUserToResizeRows = false;
			this->dgvAddressList->BackgroundColor = System::Drawing::SystemColors::ControlLightLight;
			this->dgvAddressList->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->dgvAddressList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(4) {
				this->dgvMemoryListDescription,
					this->dgvMemoryListAddress, this->dgvMemoryListType, this->dgvMemoryListValue
			});
			this->dgvAddressList->ContextMenuStrip = this->cmsAddressList;
			this->dgvAddressList->Location = System::Drawing::Point(13, 401);
			this->dgvAddressList->Name = L"dgvAddressList";
			this->dgvAddressList->ReadOnly = true;
			this->dgvAddressList->RowHeadersVisible = false;
			this->dgvAddressList->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::FullRowSelect;
			this->dgvAddressList->Size = System::Drawing::Size(959, 176);
			this->dgvAddressList->TabIndex = 11;
			this->dgvAddressList->CellMouseDoubleClick += gcnew System::Windows::Forms::DataGridViewCellMouseEventHandler(this, &MainForm::dgvAddressList_CellMouseDoubleClick);
			this->dgvAddressList->RowsRemoved += gcnew System::Windows::Forms::DataGridViewRowsRemovedEventHandler(this, &MainForm::dgvAddressList_RowsRemoved);
			this->dgvAddressList->SelectionChanged += gcnew System::EventHandler(this, &MainForm::dgvAddressList_SelectionChanged);
			// 
			// dgvMemoryListDescription
			// 
			this->dgvMemoryListDescription->HeaderText = L"Description";
			this->dgvMemoryListDescription->Name = L"dgvMemoryListDescription";
			this->dgvMemoryListDescription->ReadOnly = true;
			this->dgvMemoryListDescription->Resizable = System::Windows::Forms::DataGridViewTriState::False;
			this->dgvMemoryListDescription->Width = 456;
			// 
			// dgvMemoryListAddress
			// 
			this->dgvMemoryListAddress->HeaderText = L"Address";
			this->dgvMemoryListAddress->Name = L"dgvMemoryListAddress";
			this->dgvMemoryListAddress->ReadOnly = true;
			this->dgvMemoryListAddress->Resizable = System::Windows::Forms::DataGridViewTriState::False;
			this->dgvMemoryListAddress->Width = 220;
			// 
			// dgvMemoryListType
			// 
			this->dgvMemoryListType->HeaderText = L"Type";
			this->dgvMemoryListType->Name = L"dgvMemoryListType";
			this->dgvMemoryListType->ReadOnly = true;
			this->dgvMemoryListType->Resizable = System::Windows::Forms::DataGridViewTriState::False;
			// 
			// dgvMemoryListValue
			// 
			this->dgvMemoryListValue->HeaderText = L"Value";
			this->dgvMemoryListValue->Name = L"dgvMemoryListValue";
			this->dgvMemoryListValue->ReadOnly = true;
			this->dgvMemoryListValue->Resizable = System::Windows::Forms::DataGridViewTriState::False;
			this->dgvMemoryListValue->Width = 180;
			// 
			// cmsAddressList
			// 
			this->cmsAddressList->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {
				this->tsmiDeleteThisRecord,
					this->tsmiChangeRecord
			});
			this->cmsAddressList->Name = L"cmsAddressList";
			this->cmsAddressList->Size = System::Drawing::Size(191, 48);
			// 
			// tsmiDeleteThisRecord
			// 
			this->tsmiDeleteThisRecord->Enabled = false;
			this->tsmiDeleteThisRecord->Name = L"tsmiDeleteThisRecord";
			this->tsmiDeleteThisRecord->ShortcutKeys = System::Windows::Forms::Keys::Delete;
			this->tsmiDeleteThisRecord->Size = System::Drawing::Size(190, 22);
			this->tsmiDeleteThisRecord->Text = L"Delete this record";
			this->tsmiDeleteThisRecord->Click += gcnew System::EventHandler(this, &MainForm::tsmiDeleteThisRecord_Click);
			// 
			// tsmiChangeRecord
			// 
			this->tsmiChangeRecord->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {
				this->tsmiChangeDescription,
					this->tsmiChangeAddress, this->tsmiChangeType, this->tsmiChangeValue
			});
			this->tsmiChangeRecord->Enabled = false;
			this->tsmiChangeRecord->Name = L"tsmiChangeRecord";
			this->tsmiChangeRecord->Size = System::Drawing::Size(190, 22);
			this->tsmiChangeRecord->Text = L"Change record";
			// 
			// tsmiChangeDescription
			// 
			this->tsmiChangeDescription->Name = L"tsmiChangeDescription";
			this->tsmiChangeDescription->Size = System::Drawing::Size(134, 22);
			this->tsmiChangeDescription->Text = L"Description";
			this->tsmiChangeDescription->Click += gcnew System::EventHandler(this, &MainForm::tsmiChangeDescription_Click);
			// 
			// tsmiChangeAddress
			// 
			this->tsmiChangeAddress->Name = L"tsmiChangeAddress";
			this->tsmiChangeAddress->Size = System::Drawing::Size(134, 22);
			this->tsmiChangeAddress->Text = L"Address";
			this->tsmiChangeAddress->Click += gcnew System::EventHandler(this, &MainForm::tsmiChangeAddress_Click);
			// 
			// tsmiChangeType
			// 
			this->tsmiChangeType->Name = L"tsmiChangeType";
			this->tsmiChangeType->Size = System::Drawing::Size(134, 22);
			this->tsmiChangeType->Text = L"Type";
			this->tsmiChangeType->Click += gcnew System::EventHandler(this, &MainForm::tsmiChangeType_Click);
			// 
			// tsmiChangeValue
			// 
			this->tsmiChangeValue->Name = L"tsmiChangeValue";
			this->tsmiChangeValue->Size = System::Drawing::Size(134, 22);
			this->tsmiChangeValue->Text = L"Value";
			this->tsmiChangeValue->Click += gcnew System::EventHandler(this, &MainForm::tsmiChangeValue_Click);
			// 
			// pbScan
			// 
			this->pbScan->Location = System::Drawing::Point(13, 42);
			this->pbScan->Name = L"pbScan";
			this->pbScan->Size = System::Drawing::Size(959, 23);
			this->pbScan->TabIndex = 12;
			// 
			// lblProcessInfo
			// 
			this->lblProcessInfo->AutoSize = true;
			this->lblProcessInfo->Location = System::Drawing::Point(420, 26);
			this->lblProcessInfo->Name = L"lblProcessInfo";
			this->lblProcessInfo->Size = System::Drawing::Size(107, 13);
			this->lblProcessInfo->TabIndex = 13;
			this->lblProcessInfo->Text = L"No Process Selected";
			// 
			// btnDeleteAddresses
			// 
			this->btnDeleteAddresses->Enabled = false;
			this->btnDeleteAddresses->Location = System::Drawing::Point(589, 372);
			this->btnDeleteAddresses->Name = L"btnDeleteAddresses";
			this->btnDeleteAddresses->Size = System::Drawing::Size(75, 23);
			this->btnDeleteAddresses->TabIndex = 14;
			this->btnDeleteAddresses->Text = L"Delete All";
			this->btnDeleteAddresses->UseVisualStyleBackColor = true;
			this->btnDeleteAddresses->Click += gcnew System::EventHandler(this, &MainForm::btnDeleteAddresses_Click);
			// 
			// lblFound
			// 
			this->lblFound->AutoSize = true;
			this->lblFound->Location = System::Drawing::Point(12, 68);
			this->lblFound->Name = L"lblFound";
			this->lblFound->Size = System::Drawing::Size(49, 13);
			this->lblFound->TabIndex = 15;
			this->lblFound->Text = L"Found: 0";
			// 
			// chkNot
			// 
			this->chkNot->AutoSize = true;
			this->chkNot->Location = System::Drawing::Point(884, 192);
			this->chkNot->Name = L"chkNot";
			this->chkNot->Size = System::Drawing::Size(43, 17);
			this->chkNot->TabIndex = 16;
			this->chkNot->Text = L"Not";
			this->chkNot->UseVisualStyleBackColor = true;
			// 
			// tmrUpdateValues
			// 
			this->tmrUpdateValues->Enabled = true;
			this->tmrUpdateValues->Interval = 500;
			this->tmrUpdateValues->Tick += gcnew System::EventHandler(this, &MainForm::tmrUpdateValues_Tick);
			// 
			// btnAddAddressManually
			// 
			this->btnAddAddressManually->Enabled = false;
			this->btnAddAddressManually->Location = System::Drawing::Point(835, 372);
			this->btnAddAddressManually->Name = L"btnAddAddressManually";
			this->btnAddAddressManually->Size = System::Drawing::Size(137, 23);
			this->btnAddAddressManually->TabIndex = 17;
			this->btnAddAddressManually->Text = L"Add address manually";
			this->btnAddAddressManually->UseVisualStyleBackColor = true;
			this->btnAddAddressManually->Click += gcnew System::EventHandler(this, &MainForm::btnAddAddressManually_Click);
			// 
			// dgvMemoryAddressAddress
			// 
			this->dgvMemoryAddressAddress->HeaderText = L"Address";
			this->dgvMemoryAddressAddress->Name = L"dgvMemoryAddressAddress";
			this->dgvMemoryAddressAddress->ReadOnly = true;
			this->dgvMemoryAddressAddress->Resizable = System::Windows::Forms::DataGridViewTriState::False;
			this->dgvMemoryAddressAddress->Width = 316;
			// 
			// dgvMemoryAddressValue
			// 
			this->dgvMemoryAddressValue->HeaderText = L"Value";
			this->dgvMemoryAddressValue->Name = L"dgvMemoryAddressValue";
			this->dgvMemoryAddressValue->ReadOnly = true;
			this->dgvMemoryAddressValue->Resizable = System::Windows::Forms::DataGridViewTriState::False;
			this->dgvMemoryAddressValue->Width = 110;
			// 
			// dgvMemoryAddressPrevious
			// 
			this->dgvMemoryAddressPrevious->HeaderText = L"Previous";
			this->dgvMemoryAddressPrevious->Name = L"dgvMemoryAddressPrevious";
			this->dgvMemoryAddressPrevious->ReadOnly = true;
			this->dgvMemoryAddressPrevious->Resizable = System::Windows::Forms::DataGridViewTriState::False;
			this->dgvMemoryAddressPrevious->Width = 110;
			// 
			// dgvMemoryAddressFirst
			// 
			this->dgvMemoryAddressFirst->HeaderText = L"First";
			this->dgvMemoryAddressFirst->Name = L"dgvMemoryAddressFirst";
			this->dgvMemoryAddressFirst->ReadOnly = true;
			this->dgvMemoryAddressFirst->Resizable = System::Windows::Forms::DataGridViewTriState::False;
			this->dgvMemoryAddressFirst->Width = 110;
			// 
			// MainForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(986, 589);
			this->Controls->Add(this->btnAddAddressManually);
			this->Controls->Add(this->chkNot);
			this->Controls->Add(this->lblFound);
			this->Controls->Add(this->btnDeleteAddresses);
			this->Controls->Add(this->lblProcessInfo);
			this->Controls->Add(this->pbScan);
			this->Controls->Add(this->dgvAddressList);
			this->Controls->Add(this->grpMemoryScanOptions);
			this->Controls->Add(this->lblScanType);
			this->Controls->Add(this->cbScanType);
			this->Controls->Add(this->lblValueType);
			this->Controls->Add(this->cbValueType);
			this->Controls->Add(this->lblValue);
			this->Controls->Add(this->txtValue);
			this->Controls->Add(this->btnNextScan);
			this->Controls->Add(this->btnFirstScan);
			this->Controls->Add(this->toolStripMain);
			this->Controls->Add(this->dgvMemoryAddress);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->Name = L"MainForm";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"Memory Scanner";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dgvMemoryAddress))->EndInit();
			this->cmsMemoryAddress->ResumeLayout(false);
			this->toolStripMain->ResumeLayout(false);
			this->toolStripMain->PerformLayout();
			this->grpMemoryScanOptions->ResumeLayout(false);
			this->grpMemoryScanOptions->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dgvAddressList))->EndInit();
			this->cmsAddressList->ResumeLayout(false);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void tsmiOpenProcess_Click(System::Object^ sender, System::EventArgs^ e) {
		ProcessListForm^ frmProcessListForm = gcnew ProcessListForm();
		frmProcessListForm->ShowDialog();

		// Check if the dialog result is OK
		if (frmProcessListForm->DialogResult == System::Windows::Forms::DialogResult::OK)
		{
			dwProcessId = frmProcessListForm->GetSelectedProcessId();
			strProcessName = frmProcessListForm->GetSelectedProcessName();

			lblProcessInfo->Text = dwProcessId.ToString() + " - " + strProcessName + ".exe";
			btnFirstScan->Enabled = true;
			btnAddAddressManually->Enabled = true;

			cbScanOptions->Items->Clear();
			cbScanOptions->Items->Add(strProcessName + ".exe");
			cbScanOptions->SelectedIndex = 0;

			dgvMemoryAddress->Rows->Clear();
			dgvAddressList->Rows->Clear();

			if (IsProcessX64(dwProcessId))
			{
				dwModuleBaseAddress = GetModuleBaseAddress64();
			}
			else
			{
				dwModuleBaseAddress = GetModuleBaseAddress32();
			}

			hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, dwProcessId);

			// Set min and max values by base address + a constant (0x253000)
			MIN_ADDRESS = dwModuleBaseAddress;
			MAX_ADDRESS = dwModuleBaseAddress + 0x253000;

			txtAddressStart->Clear();
			txtAddressStart->Text = MIN_ADDRESS.ToString("X");

			txtAddressStop->Clear();
			txtAddressStop->Text = MAX_ADDRESS.ToString("X");
		}
	}
	private: System::Void tsmiExit_Click(System::Object^ sender, System::EventArgs^ e) {
		Application::Exit();
	}
	private: System::Void btnFirstScan_Click(System::Object^ sender, System::EventArgs^ e) {
		if (txtValue->Text == "")
		{
			MessageBox::Show("Please enter a value.", "ERROR");
		}
		else
		{
			System::UInt64 uValueStart;
			System::UInt64 uValueStop;

			// Parse MIN and MAX values
			if (System::UInt64::TryParse(txtAddressStart->Text, System::Globalization::NumberStyles::HexNumber, nullptr, uValueStart) &&
				System::UInt64::TryParse(txtAddressStop->Text, System::Globalization::NumberStyles::HexNumber, nullptr, uValueStop))
			{
				MIN_ADDRESS = static_cast<DWORD_PTR>(uValueStart);
				MAX_ADDRESS = static_cast<DWORD_PTR>(uValueStop);
			}
			else
			{
				MessageBox::Show("Can't parse memory addresses", "ERROR");
				return;
			}

			int iValueType = cbValueType->SelectedIndex;
			bool bIsWritable = chkWritable->Checked;

			// Handle numeric searches
			if (iValueType == 0 || iValueType == 1 || iValueType == 2 || iValueType == 3)
			{
				int iScanType = cbScanType->SelectedIndex;
				System::UInt64 u64SearchQuery;

				if (!System::UInt64::TryParse(txtValue->Text, u64SearchQuery))
				{
					// Handle invalid query input
					MessageBox::Show("Invalid entry.", "ERROR");
					return;
				}

				ScanMemoryForNumbers(u64SearchQuery, iValueType, iScanType, bIsWritable);
			}
			// Handle text searches
			else if (iValueType == 4)
			{
				String^ strSearchQuery = txtValue->Text;

				// chkNot's text changes to "Case sensitive" when type String is selected, then changes back to "Not" when a numeric value is selected
				bool bIsCaseSensitive = chkNot->Checked;

				ScanMemoryForStrings(strSearchQuery, bIsCaseSensitive, bIsWritable);
			}
		}
	}
	private: System::Void dgvMemoryAddress_CellMouseDoubleClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellMouseEventArgs^ e) {
		AddSelectedAddressesToAddressList();
	}
	private: System::Void btnDeleteAddresses_Click(System::Object^ sender, System::EventArgs^ e) {
		ClearAddressList();
	}
	private: System::Void dgvAddressList_CellMouseDoubleClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellMouseEventArgs^ e) {
		if (e->RowIndex >= 0)
		{
			int iColumnIndex = e->ColumnIndex;

			// Determine which cell in the selected row was double-clicked
			if (iColumnIndex == dgvMemoryListAddress->Index)
			{
				int iRowIndex = e->RowIndex;
				MemoryValue^ selectedMemoryValue = (MemoryValue^)dgvAddressList->Rows[iRowIndex]->Tag;
				int iDataType = selectedMemoryValue->Type;
				String^ strHexAddress = dgvAddressList->Rows[iRowIndex]->Cells[dgvMemoryListAddress->Index]->Value->ToString();
				String^ strPreviousDescription = dgvAddressList->Rows[iRowIndex]->Cells[dgvMemoryListDescription->Index]->Value->ToString();

				AddAddressForm^ frmAddAddressForm = gcnew AddAddressForm(false, strPreviousDescription, dwProcessId, dwModuleBaseAddress, strHexAddress, iDataType);
				frmAddAddressForm->ShowDialog();

				// Check if the dialog result is OK
				if (frmAddAddressForm->DialogResult == System::Windows::Forms::DialogResult::OK)
				{
					BYTE* cbNewAddress = frmAddAddressForm->GetNewAddress();
					System::Diagnostics::Debug::WriteLine("cbNewAddress: " + System::Convert::ToString(reinterpret_cast<int>(cbNewAddress), 16));
					int iNewDataType = frmAddAddressForm->GetNewDataType();
					String^ strNewDescription = frmAddAddressForm->GetNewDescription();

					selectedMemoryValue->Address = cbNewAddress;
					selectedMemoryValue->Type = iNewDataType;

					selectedMemoryValue->DisplayAddress = String::Format("0x{0:X}", IntPtr(cbNewAddress).ToInt64() + dwModuleBaseAddress);
					dgvAddressList->Rows[iRowIndex]->Cells["dgvMemoryListAddress"]->Value = selectedMemoryValue->DisplayAddress;
					dgvAddressList->Rows[iRowIndex]->Cells["dgvMemoryListType"]->Value = GetTypeString(iNewDataType);
					dgvAddressList->Rows[iRowIndex]->Cells["dgvMemoryListDescription"]->Value = strNewDescription;
				}
			}
			else if (iColumnIndex == dgvMemoryListValue->Index)
			{
				int iRowIndex = e->RowIndex;
				MemoryValue^ selectedMemoryValue = (MemoryValue^)dgvAddressList->Rows[iRowIndex]->Tag;
				String^ strHexAddress = dgvAddressList->Rows[iRowIndex]->Cells[dgvMemoryListAddress->Index]->Value->ToString();
				BYTE* cbAddress = ExtractHexAddress(strHexAddress);
				String^ strValue = dgvAddressList->Rows[iRowIndex]->Cells[dgvMemoryListValue->Index]->Value->ToString();
				int iDataType = selectedMemoryValue->Type;

				NewValueForm^ frmNewValueForm = gcnew NewValueForm(strValue, cbAddress, dwProcessId, iDataType);
				frmNewValueForm->ShowDialog();
			}
			else if (iColumnIndex == dgvMemoryListDescription->Index)
			{
				int iRowIndex = e->RowIndex;
				String^ strCurrentDescription = dgvAddressList->Rows[iRowIndex]->Cells[dgvMemoryListDescription->Index]->Value->ToString();

				NewDescriptionForm^ frmNewDescriptionForm = gcnew NewDescriptionForm(strCurrentDescription);
				frmNewDescriptionForm->ShowDialog();

				// Check if the dialog result is OK
				if (frmNewDescriptionForm->DialogResult == System::Windows::Forms::DialogResult::OK)
				{
					String^ strNewDescription = frmNewDescriptionForm->GetNewDescription();
					dgvAddressList->Rows[e->RowIndex]->Cells[dgvMemoryListDescription->Index]->Value = strNewDescription;
				}
			}
			else if (iColumnIndex == dgvMemoryListType->Index)
			{
				int iRowIndex = e->RowIndex;
				MemoryValue^ selectedMemoryValue = (MemoryValue^)dgvAddressList->Rows[iRowIndex]->Tag;

				ChangeTypeForm^ frmChangeTypeForm = gcnew ChangeTypeForm(selectedMemoryValue);
				frmChangeTypeForm->ShowDialog();

				// Check if the dialog result is OK
				if (frmChangeTypeForm->DialogResult == System::Windows::Forms::DialogResult::OK)
				{
					int iNewDataType = frmChangeTypeForm->GetNewDataType();
					selectedMemoryValue->Type = iNewDataType;
					dgvAddressList->Rows[iRowIndex]->Cells[dgvMemoryListType->Index]->Value = GetTypeString(iNewDataType);
				}
			}
		}
	}
	private: System::Void tsmiAddToAddressList_Click(System::Object^ sender, System::EventArgs^ e) {
		AddSelectedAddressesToAddressList();
	}

	private: System::Void tsmiCopySelectedAddresses_Click(System::Object^ sender, System::EventArgs^ e) {
		DataGridViewSelectedRowCollection^ dgvSelectedRows = dgvMemoryAddress->SelectedRows;

		String^ strClipboardText = "";

		for (int i = 0; i < dgvSelectedRows->Count; i++)
		{
			DataGridViewRow^ dgvSelectedRow = dgvSelectedRows[i];
			String^ strOriginalAddress = dgvSelectedRow->Cells["dgvMemoryAddressAddress"]->Value->ToString();

			strClipboardText += strOriginalAddress;

			if (i != dgvSelectedRows->Count - 1)
			{
				strClipboardText += "\n";
			}
		}

		CopyToClipboard(strClipboardText);
	}
	private: System::Void dgvMemoryAddress_RowsAdded(System::Object^ sender, System::Windows::Forms::DataGridViewRowsAddedEventArgs^ e) {
		tsmiAddToAddressList->Enabled = (dgvMemoryAddress->Rows->Count > 0);
		tsmiCopySelectedAddresses->Enabled = (dgvMemoryAddress->Rows->Count > 0);
		tsmiChangeValueOfSelectedAddresses->Enabled = (dgvMemoryAddress->Rows->Count > 0);
	}
	private: System::Void dgvMemoryAddress_RowsRemoved(System::Object^ sender, System::Windows::Forms::DataGridViewRowsRemovedEventArgs^ e) {
		tsmiAddToAddressList->Enabled = (dgvMemoryAddress->Rows->Count > 0);
		tsmiCopySelectedAddresses->Enabled = (dgvMemoryAddress->Rows->Count > 0);
		tsmiChangeValueOfSelectedAddresses->Enabled = (dgvMemoryAddress->Rows->Count > 0);
	}
	private: System::Void dgvAddressList_SelectionChanged(System::Object^ sender, System::EventArgs^ e) {
		DataGridViewSelectedRowCollection^ selectedRows = dgvAddressList->SelectedRows;

		tsmiDeleteThisRecord->Enabled = (selectedRows->Count > 0);
		tsmiChangeRecord->Enabled = (selectedRows->Count > 0);
	}
	private: System::Void tsmiDeleteThisRecord_Click(System::Object^ sender, System::EventArgs^ e) {
		// Check if the dialog result is Yes
		if (MessageBox::Show("Are you sure you want to delete the selected records?", L"Confirmation",
			MessageBoxButtons::YesNo, MessageBoxIcon::Question) == System::Windows::Forms::DialogResult::Yes)
		{
			DataGridViewSelectedRowCollection^ dgvSelectedRows = dgvAddressList->SelectedRows;

			for (int i = dgvSelectedRows->Count - 1; i >= 0; i--)
			{
				int iRowIndex = dgvSelectedRows[i]->Index;
				dgvAddressList->Rows->RemoveAt(iRowIndex);
			}
		}
	}
	private: System::Void tmrUpdateValues_Tick(System::Object^ sender, System::EventArgs^ e) {
		UpdateAddressListValues(dgvAddressList, dgvMemoryListValue->Index);
		UpdateMemoryAddressValues(dgvMemoryAddress, dgvMemoryAddressValue->Index);
	}
	private: System::Void tsmiChangeDescription_Click(System::Object^ sender, System::EventArgs^ e) {
		DataGridViewSelectedRowCollection^ dgvSelectedRows = dgvAddressList->SelectedRows;

		for (int i = 0; i < dgvSelectedRows->Count; i++)
		{
			int iRowIndex = dgvSelectedRows[i]->Index;
			String^ strCurrentDescription = dgvAddressList->Rows[iRowIndex]->Cells[dgvMemoryListDescription->Index]->Value->ToString();

			NewDescriptionForm^ frmNewDescriptionForm = gcnew NewDescriptionForm(strCurrentDescription);
			frmNewDescriptionForm->ShowDialog();

			// Check if the dialog result is OK
			if (frmNewDescriptionForm->DialogResult == System::Windows::Forms::DialogResult::OK)
			{
				String^ strNewDescription = frmNewDescriptionForm->GetNewDescription();
				dgvAddressList->Rows[dgvSelectedRows[i]->Index]->Cells[dgvMemoryListDescription->Index]->Value = strNewDescription;
			}
		}
	}
	private: System::Void tsmiChangeValue_Click(System::Object^ sender, System::EventArgs^ e) {
		DataGridViewSelectedRowCollection^ dgvSelectedRows = dgvAddressList->SelectedRows;

		for (int i = 0; i < dgvSelectedRows->Count; i++)
		{
			int iRowIndex = dgvSelectedRows[i]->Index;
			MemoryValue^ selectedMemoryValue = (MemoryValue^)dgvAddressList->Rows[iRowIndex]->Tag;
			String^ strHexAddress = dgvAddressList->Rows[iRowIndex]->Cells[dgvMemoryListAddress->Index]->Value->ToString();
			BYTE* cbAddress = ExtractHexAddress(strHexAddress);
			String^ strValue = dgvAddressList->Rows[iRowIndex]->Cells[dgvMemoryListValue->Index]->Value->ToString();
			int iDataType = selectedMemoryValue->Type;

			NewValueForm^ frmNewValueForm = gcnew NewValueForm(strValue, cbAddress, dwProcessId, iDataType);
			frmNewValueForm->ShowDialog();
		}
	}
	private: System::Void dgvAddressList_RowsRemoved(System::Object^ sender, System::Windows::Forms::DataGridViewRowsRemovedEventArgs^ e) {
		btnDeleteAddresses->Enabled = (dgvAddressList->Rows->Count > 0);
	}
	private: System::Void tsmiAbout_Click(System::Object^ sender, System::EventArgs^ e) {
		AboutForm^ frmAboutForm = gcnew AboutForm();
		frmAboutForm->ShowDialog();
	}
	private: System::Void tsmiViewREADME_Click(System::Object^ sender, System::EventArgs^ e) {
		System::Diagnostics::Process::Start("https://github.com/JulianOzelRose/Memory-Scanner/blob/master/README.md");
	}
	private: System::Void cbValueType_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {
		// Change scan options for Text
		if (cbValueType->SelectedIndex == 4)
		{
			lblValue->Text = "Text:";
			chkNot->Text = "Case sensitive";

			cbScanType->Items->Clear();
			cbScanType->Items->Add("Search for text");
			cbScanType->SelectedIndex = 0;
		}
		// Change scan options for Numbers
		else
		{
			lblValue->Text = "Value:";
			chkNot->Text = "Not";

			cbScanType->Items->Clear();
			cbScanType->Items->Add("Exact value");
			cbScanType->Items->Add("Bigger than...");
			cbScanType->Items->Add("Smaller than...");
			cbScanType->SelectedIndex = 0;
		}

		// Clear search query textbox
		txtValue->Clear();
	}
	private: System::Void btnAddAddressManually_Click(System::Object^ sender, System::EventArgs^ e) {
		AddAddressForm^ frmAddAddressForm = gcnew AddAddressForm(true, nullptr, dwProcessId, dwModuleBaseAddress, nullptr, 0);
		frmAddAddressForm->ShowDialog();

		// Check if the dialog result is OK
		if (frmAddAddressForm->DialogResult == System::Windows::Forms::DialogResult::OK)
		{
			MemoryValue^ newMemoryValue = frmAddAddressForm->GetNewMemoryObject();
			memoryValues->Add(newMemoryValue);

			// Associate MemoryValue with Tag
			int iNewRowIndex = dgvAddressList->Rows->Add();
			dgvAddressList->Rows[iNewRowIndex]->Tag = newMemoryValue;

			// Extract MemoryValue info
			String^ strDisplayAddress = newMemoryValue->DisplayAddress;

			// Get new address description
			String^ strDescription = frmAddAddressForm->GetNewDescription();

			// Set the value in the new DataGridView cell
			dgvAddressList->Rows[iNewRowIndex]->Cells["dgvMemoryListAddress"]->Value = strDisplayAddress;
			dgvAddressList->Rows[iNewRowIndex]->Cells["dgvMemoryListValue"]->Value = newMemoryValue->Value;
			dgvAddressList->Rows[iNewRowIndex]->Cells["dgvMemoryListDescription"]->Value = strDescription;
			dgvAddressList->Rows[iNewRowIndex]->Cells["dgvMemoryListType"]->Value = GetTypeString(newMemoryValue->Type);
		}

		btnDeleteAddresses->Enabled = true;
	}
	private: System::Void tsmiClearAddressList_Click(System::Object^ sender, System::EventArgs^ e) {
		ClearAddressList();
	}
	private: System::Void tsmiChangeAddress_Click(System::Object^ sender, System::EventArgs^ e) {
		DataGridViewSelectedRowCollection^ dgvSelectedRows = dgvAddressList->SelectedRows;

		for (int i = 0; i < dgvSelectedRows->Count; i++)
		{
			int iRowIndex = dgvSelectedRows[i]->Index;

			MemoryValue^ selectedMemoryValue = (MemoryValue^)dgvAddressList->Rows[iRowIndex]->Tag;
			int iDataType = selectedMemoryValue->Type;
			String^ strHexAddress = dgvAddressList->Rows[iRowIndex]->Cells[dgvMemoryListAddress->Index]->Value->ToString();
			String^ strPreviousDescription = dgvAddressList->Rows[iRowIndex]->Cells[dgvMemoryListDescription->Index]->Value->ToString();

			AddAddressForm^ frmAddAddressForm = gcnew AddAddressForm(false, strPreviousDescription, dwProcessId, dwModuleBaseAddress, strHexAddress, iDataType);
			frmAddAddressForm->ShowDialog();

			// Check if the dialog result is OK
			if (frmAddAddressForm->DialogResult == System::Windows::Forms::DialogResult::OK)
			{
				BYTE* cbNewAddress = frmAddAddressForm->GetNewAddress();
				System::Diagnostics::Debug::WriteLine("cbNewAddress: " + System::Convert::ToString(reinterpret_cast<int>(cbNewAddress), 16));
				int iNewDataType = frmAddAddressForm->GetNewDataType();
				String^ strNewDescription = frmAddAddressForm->GetNewDescription();

				selectedMemoryValue->Address = cbNewAddress;
				selectedMemoryValue->Type = iNewDataType;

				selectedMemoryValue->DisplayAddress = String::Format("0x{0:X}", IntPtr(cbNewAddress).ToInt64() + dwModuleBaseAddress);
				dgvAddressList->Rows[iRowIndex]->Cells["dgvMemoryListAddress"]->Value = selectedMemoryValue->DisplayAddress;
				dgvAddressList->Rows[iRowIndex]->Cells["dgvMemoryListType"]->Value = GetTypeString(iNewDataType);
				dgvAddressList->Rows[iRowIndex]->Cells["dgvMemoryListDescription"]->Value = strNewDescription;
			}
		}
	}
	private: System::Void tsmiChangeType_Click(System::Object^ sender, System::EventArgs^ e) {
		DataGridViewSelectedRowCollection^ dgvSelectedRows = dgvAddressList->SelectedRows;

		for (int i = 0; i < dgvSelectedRows->Count; i++)
		{
			int iRowIndex = dgvSelectedRows[i]->Index;
			MemoryValue^ selectedMemoryValue = (MemoryValue^)dgvAddressList->Rows[iRowIndex]->Tag;

			ChangeTypeForm^ frmChangeTypeForm = gcnew ChangeTypeForm(selectedMemoryValue);
			frmChangeTypeForm->ShowDialog();

			// Check if the dialog result is OK
			if (frmChangeTypeForm->DialogResult == System::Windows::Forms::DialogResult::OK)
			{
				int iNewDataType = frmChangeTypeForm->GetNewDataType();
				selectedMemoryValue->Type = iNewDataType;
				dgvAddressList->Rows[iRowIndex]->Cells[dgvMemoryListType->Index]->Value = GetTypeString(iNewDataType);
			}
		}
	}
	private: System::Void dgvMemoryAddress_UserDeletingRow(System::Object^ sender, System::Windows::Forms::DataGridViewRowCancelEventArgs^ e) {
		e->Cancel = true;
	}
	};
}