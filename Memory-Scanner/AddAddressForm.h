#pragma once
#include "MemoryValue.h"
#include <msclr/marshal.h>

namespace MemoryScanner {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	public ref class AddAddressForm : public System::Windows::Forms::Form
	{
	public:
		AddAddressForm(bool bIsNewAddress, String^ previousDescription, DWORD processId, DWORD moduleBaseAddress, String^ hexAddress, int dataType)
		{
			InitializeComponent();

			dwProcessId = processId;
			dwModuleBaseAddress = moduleBaseAddress;

			if (bIsNewAddress)
			{
				cbValueType->SelectedIndex = 2;
				this->Text = "Add Address";
				chkAddModuleBaseAddress->Checked = true;
			}
			else
			{
				cbValueType->SelectedIndex = dataType;
				txtDescription->Text = previousDescription;
				this->Text = "Change Address";

				chkAddModuleBaseAddress->Checked = false;
				txtAddress->Text = hexAddress;
				ReadValue();
			}
		}

		BYTE* GetNewAddress()
		{
			BYTE* cbAddress = 0;

			if (chkAddModuleBaseAddress->Checked)
			{
				cbAddress = (BYTE*)(ExtractHexAddress(txtAddress->Text));
			}
			else
			{
				cbAddress = (BYTE*)(ExtractHexAddress(txtAddress->Text) - dwModuleBaseAddress);
			}

			return cbAddress;
		}

		int GetNewDataType()
		{
			return cbValueType->SelectedIndex;
		}

		String^ GetNewDescription()
		{
			return txtDescription->Text;
		}

		void ReadValue()
		{
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, dwProcessId);
			BYTE* cbAddress;
			DWORD dwValue = 0;
			String^ strValue = " = ";

			if (chkAddModuleBaseAddress->Checked)
			{
				cbAddress = (BYTE*)(dwModuleBaseAddress + ExtractHexAddress(txtAddress->Text));
			}
			else
			{
				cbAddress = (BYTE*)ExtractHexAddress(txtAddress->Text);
			}

			if (cbValueType->SelectedIndex == 0)
			{
				// Read BYTE number from memory
				strValue += ReadByteFromMemory(hProcess, cbAddress);
			}
			else if (cbValueType->SelectedIndex == 1)
			{
				// Read 2-byte number from memory
				strValue += ReadUInt16FromMemory(hProcess, cbAddress);
			}
			else if (cbValueType->SelectedIndex == 2)
			{
				// Read 4-byte number from memory
				strValue += ReadDWORDFromMemory(hProcess, cbAddress);
			}
			else if (cbValueType->SelectedIndex == 3)
			{
				// Read 8-byte number from memory
				strValue += ReadUInt64FromMemory(hProcess, cbAddress);
			}
			else if (cbValueType->SelectedIndex == 4)
			{
				// Reset process handle
				hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

				// Read string from memory
				strValue += ReadStringFromMemory(cbAddress, hProcess);
			}

			lblValue->Text = strValue;
		}

		String^ ReadByteFromMemory(HANDLE hProcess, BYTE* cbAddress)
		{
			BYTE bValue = 0;

			if (!ReadProcessMemory(hProcess, reinterpret_cast<LPVOID>(cbAddress), &bValue, sizeof(bValue), nullptr))
			{
				return "???";
			}

			return bValue.ToString();
		}

		String^ ReadUInt16FromMemory(HANDLE hProcess, BYTE* cbAddress)
		{
			uint16_t u16Value = 0;

			if (!ReadProcessMemory(hProcess, reinterpret_cast<LPVOID>(cbAddress), &u16Value, sizeof(u16Value), nullptr))
			{
				return "???";
			}

			return u16Value.ToString();
		}

		String^ ReadDWORDFromMemory(HANDLE hProcess, BYTE* cbAddress)
		{
			DWORD dwValue = 0;

			if (!ReadProcessMemory(hProcess, reinterpret_cast<LPVOID>(cbAddress), &dwValue, sizeof(dwValue), nullptr))
			{
				return "???";
			}

			return dwValue.ToString();
		}

		String^ ReadUInt64FromMemory(HANDLE hProcess, BYTE* cbAddress)
		{
			uint64_t u64Value = 0;

			if (!ReadProcessMemory(hProcess, reinterpret_cast<LPVOID>(cbAddress), &u64Value, sizeof(u64Value), nullptr))
			{
				return "???";
			}

			return u64Value.ToString();
		}

		MemoryValue^ GetNewMemoryObject()
		{
			MemoryValue^ newMemoryValue = gcnew MemoryValue();
			BYTE* cbAddress;

			if (chkAddModuleBaseAddress->Checked)
			{
				cbAddress = (BYTE*)(ExtractHexAddress(txtAddress->Text));
			}
			else
			{
				cbAddress = (BYTE*)(ExtractHexAddress(txtAddress->Text) - dwModuleBaseAddress);
			}

			// Set MemoryValue object's values
			newMemoryValue->DisplayAddress = String::Format("0x{0:X}", IntPtr(cbAddress).ToInt64() + dwModuleBaseAddress);
			newMemoryValue->Address = cbAddress;
			newMemoryValue->Type = cbValueType->SelectedIndex;
			newMemoryValue->Value = (lblValue->Text->Replace("= ", ""))->Trim();
			newMemoryValue->IsModuleBaseIncluded = (!chkAddModuleBaseAddress->Checked);

			return newMemoryValue;
		}

		String^ ReadStringFromMemory(BYTE* cbAddress, HANDLE hProcess)
		{
			const int iMaxStringLength = 1024;
			array<BYTE>^ cbBufferArray = gcnew array<BYTE>(iMaxStringLength);
			pin_ptr<BYTE> cbptrBuffer = &cbBufferArray[0];

			if (ReadProcessMemory(hProcess, (LPCVOID)cbAddress, cbptrBuffer, iMaxStringLength, nullptr))
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

			return nullptr;
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
				BYTE* cbValue = reinterpret_cast<BYTE*>(uParsedValue);

				return cbValue;
			}
			catch (FormatException^)
			{
				return nullptr;
			}
		}

	private: DWORD dwProcessId;
	private: DWORD dwModuleBaseAddress;

	protected:
		~AddAddressForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Label^ lblAddress;
	protected:
	private: System::Windows::Forms::Label^ lblDescription;
	private: System::Windows::Forms::Label^ lblType;
	private: System::Windows::Forms::TextBox^ txtAddress;

	private: System::Windows::Forms::TextBox^ txtDescription;

	private: System::Windows::Forms::ComboBox^ cbValueType;
	private: System::Windows::Forms::Button^ btnCancel;
	private: System::Windows::Forms::Button^ btnOK;
	private: System::Windows::Forms::Label^ lblValue;
	private: System::Windows::Forms::CheckBox^ chkAddModuleBaseAddress;


	private:
		System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(AddAddressForm::typeid));
			this->lblAddress = (gcnew System::Windows::Forms::Label());
			this->lblDescription = (gcnew System::Windows::Forms::Label());
			this->lblType = (gcnew System::Windows::Forms::Label());
			this->txtAddress = (gcnew System::Windows::Forms::TextBox());
			this->txtDescription = (gcnew System::Windows::Forms::TextBox());
			this->cbValueType = (gcnew System::Windows::Forms::ComboBox());
			this->btnCancel = (gcnew System::Windows::Forms::Button());
			this->btnOK = (gcnew System::Windows::Forms::Button());
			this->lblValue = (gcnew System::Windows::Forms::Label());
			this->chkAddModuleBaseAddress = (gcnew System::Windows::Forms::CheckBox());
			this->SuspendLayout();
			// 
			// lblAddress
			// 
			this->lblAddress->AutoSize = true;
			this->lblAddress->Location = System::Drawing::Point(6, 9);
			this->lblAddress->Name = L"lblAddress";
			this->lblAddress->Size = System::Drawing::Size(48, 13);
			this->lblAddress->TabIndex = 0;
			this->lblAddress->Text = L"Address:";
			// 
			// lblDescription
			// 
			this->lblDescription->AutoSize = true;
			this->lblDescription->Location = System::Drawing::Point(6, 78);
			this->lblDescription->Name = L"lblDescription";
			this->lblDescription->Size = System::Drawing::Size(63, 13);
			this->lblDescription->TabIndex = 1;
			this->lblDescription->Text = L"Description:";
			// 
			// lblType
			// 
			this->lblType->AutoSize = true;
			this->lblType->Location = System::Drawing::Point(6, 127);
			this->lblType->Name = L"lblType";
			this->lblType->Size = System::Drawing::Size(34, 13);
			this->lblType->TabIndex = 2;
			this->lblType->Text = L"Type:";
			// 
			// txtAddress
			// 
			this->txtAddress->Location = System::Drawing::Point(9, 25);
			this->txtAddress->Name = L"txtAddress";
			this->txtAddress->Size = System::Drawing::Size(168, 20);
			this->txtAddress->TabIndex = 3;
			this->txtAddress->TextChanged += gcnew System::EventHandler(this, &AddAddressForm::txtAddress_TextChanged);
			this->txtAddress->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &AddAddressForm::txtAddress_KeyDown);
			// 
			// txtDescription
			// 
			this->txtDescription->Location = System::Drawing::Point(9, 94);
			this->txtDescription->Name = L"txtDescription";
			this->txtDescription->Size = System::Drawing::Size(255, 20);
			this->txtDescription->TabIndex = 4;
			this->txtDescription->Text = L"No description";
			this->txtDescription->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &AddAddressForm::txtDescription_KeyDown);
			// 
			// cbValueType
			// 
			this->cbValueType->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cbValueType->FormattingEnabled = true;
			this->cbValueType->Items->AddRange(gcnew cli::array< System::Object^  >(5) {
				L"Byte", L"2 Bytes", L"4 Bytes", L"8 Bytes",
					L"String"
			});
			this->cbValueType->Location = System::Drawing::Point(9, 143);
			this->cbValueType->Name = L"cbValueType";
			this->cbValueType->Size = System::Drawing::Size(168, 21);
			this->cbValueType->TabIndex = 5;
			this->cbValueType->SelectedIndexChanged += gcnew System::EventHandler(this, &AddAddressForm::cbValueType_SelectedIndexChanged);
			// 
			// btnCancel
			// 
			this->btnCancel->Location = System::Drawing::Point(319, 188);
			this->btnCancel->Name = L"btnCancel";
			this->btnCancel->Size = System::Drawing::Size(75, 23);
			this->btnCancel->TabIndex = 6;
			this->btnCancel->Text = L"Cancel";
			this->btnCancel->UseVisualStyleBackColor = true;
			this->btnCancel->Click += gcnew System::EventHandler(this, &AddAddressForm::btnCancel_Click);
			// 
			// btnOK
			// 
			this->btnOK->Location = System::Drawing::Point(238, 188);
			this->btnOK->Name = L"btnOK";
			this->btnOK->Size = System::Drawing::Size(75, 23);
			this->btnOK->TabIndex = 7;
			this->btnOK->Text = L"OK";
			this->btnOK->UseVisualStyleBackColor = true;
			this->btnOK->Click += gcnew System::EventHandler(this, &AddAddressForm::btnOK_Click);
			// 
			// lblValue
			// 
			this->lblValue->AutoSize = true;
			this->lblValue->Location = System::Drawing::Point(183, 28);
			this->lblValue->Name = L"lblValue";
			this->lblValue->Size = System::Drawing::Size(34, 13);
			this->lblValue->TabIndex = 8;
			this->lblValue->Text = L"= \?\?\?";
			// 
			// chkAddModuleBaseAddress
			// 
			this->chkAddModuleBaseAddress->AutoSize = true;
			this->chkAddModuleBaseAddress->Checked = true;
			this->chkAddModuleBaseAddress->CheckState = System::Windows::Forms::CheckState::Checked;
			this->chkAddModuleBaseAddress->Location = System::Drawing::Point(9, 51);
			this->chkAddModuleBaseAddress->Name = L"chkAddModuleBaseAddress";
			this->chkAddModuleBaseAddress->Size = System::Drawing::Size(148, 17);
			this->chkAddModuleBaseAddress->TabIndex = 9;
			this->chkAddModuleBaseAddress->Text = L"Add module base address";
			this->chkAddModuleBaseAddress->UseVisualStyleBackColor = true;
			this->chkAddModuleBaseAddress->CheckedChanged += gcnew System::EventHandler(this, &AddAddressForm::chkAddModuleBaseAddress_CheckedChanged);
			// 
			// AddAddressForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(406, 223);
			this->Controls->Add(this->chkAddModuleBaseAddress);
			this->Controls->Add(this->lblValue);
			this->Controls->Add(this->btnOK);
			this->Controls->Add(this->btnCancel);
			this->Controls->Add(this->cbValueType);
			this->Controls->Add(this->txtDescription);
			this->Controls->Add(this->txtAddress);
			this->Controls->Add(this->lblType);
			this->Controls->Add(this->lblDescription);
			this->Controls->Add(this->lblAddress);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"AddAddressForm";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Add Address";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void btnCancel_Click(System::Object^ sender, System::EventArgs^ e) {
		DialogResult = System::Windows::Forms::DialogResult::Cancel;
		this->Close();
	}
	private: System::Void btnOK_Click(System::Object^ sender, System::EventArgs^ e) {
		DialogResult = System::Windows::Forms::DialogResult::OK;
		this->Close();
	}
	private: System::Void txtAddress_TextChanged(System::Object^ sender, System::EventArgs^ e) {
		ReadValue();
	}
	private: System::Void chkAddModuleBaseAddress_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {
		ReadValue();
	}
	private: System::Void cbValueType_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {
		ReadValue();
	}
	private: System::Void txtDescription_KeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e) {
		if (e->KeyCode == Keys::Enter)
		{
			DialogResult = System::Windows::Forms::DialogResult::OK;
			this->Close();
		}
		else if (e->KeyCode == Keys::Escape)
		{
			DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->Close();
		}
	}
	private: System::Void txtAddress_KeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e) {
		if (e->KeyCode == Keys::Enter)
		{
			DialogResult = System::Windows::Forms::DialogResult::OK;
			this->Close();
		}
		else if (e->KeyCode == Keys::Escape)
		{
			DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->Close();
		}
	}
	};
}
