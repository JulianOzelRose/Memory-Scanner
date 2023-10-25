#pragma once
#include <Windows.h>
#include <msclr/marshal_cppstd.h>

namespace MemoryScanner {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Runtime::InteropServices;
	using namespace msclr::interop;

	public ref class NewValueForm : public System::Windows::Forms::Form
	{
	public:
		NewValueForm(String^ lastValue, BYTE* address, DWORD processId, int dataType)
		{
			InitializeComponent();

			cbAddress = address;
			dwProcessId = processId;
			iDataType = dataType;

			txtNewValue->Text = lastValue;
		}

		void WriteNewValueToMemory()
		{
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, dwProcessId);

			if (iDataType != 4)
			{
				UINT64 u64NewValue;

				// Parse value from text box
				if (System::UInt64::TryParse(txtNewValue->Text, u64NewValue))
				{
					if (iDataType == 0)
					{
						// Write BYTE number to memory
						WriteByteToMemory(hProcess, (BYTE*)cbAddress, (BYTE)u64NewValue);
					}
					else if (iDataType == 1)
					{
						// Write 2-byte number to memory
						WriteUInt16ToMemory(hProcess, (BYTE*)cbAddress, (uint16_t)u64NewValue);
					}
					else if (iDataType == 2)
					{
						// Write 4-byte number to memory
						WriteDWORDToMemory(hProcess, (BYTE*)cbAddress, (DWORD)u64NewValue);
					}
					else if (iDataType == 3)
					{
						// Write 8-byte number to memory
						WriteUInt64ToMemory(hProcess, (BYTE*)cbAddress, (uint64_t)u64NewValue);
					}
				}
				else
				{
					MessageBox::Show("Unable to parse value", "ERROR");
				}
			}
			else if (iDataType == 4)
			{
				String^ strNewValue = txtNewValue->Text;
				char* cNewValue = static_cast<char*>(Marshal::StringToHGlobalAnsi(strNewValue).ToPointer());

				// Write string to memory
				hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
				WriteStringToMemory(hProcess, (BYTE*)cbAddress, cNewValue);

				Marshal::FreeHGlobal(IntPtr(cNewValue));
			}

			CloseHandle(hProcess);
		}

		void WriteByteToMemory(HANDLE hProcess, BYTE* cbAddress, BYTE bValue)
		{
			if (!WriteProcessMemory(hProcess, (BYTE*)cbAddress, &bValue, sizeof(bValue), NULL))
			{
				DWORD dwError = GetLastError();
				MessageBox::Show("Failed to write to address, error code: " + dwError.ToString(), "ERROR");
			}
		}

		void WriteUInt16ToMemory(HANDLE hProcess, BYTE* cbAddress, uint16_t u16Value)
		{
			if (!WriteProcessMemory(hProcess, (BYTE*)cbAddress, &u16Value, sizeof(u16Value), NULL))
			{
				DWORD dwError = GetLastError();
				MessageBox::Show("Failed to write to address, error code: " + dwError.ToString(), "ERROR");
			}
		}

		void WriteDWORDToMemory(HANDLE hProcess, BYTE* cbAddress, DWORD dwValue)
		{
			if (!WriteProcessMemory(hProcess, (BYTE*)cbAddress, &dwValue, sizeof(dwValue), NULL))
			{
				DWORD dwError = GetLastError();
				MessageBox::Show("Failed to write to address, error code: " + dwError.ToString(), "ERROR");
			}
		}

		void WriteUInt64ToMemory(HANDLE hProcess, BYTE* cbAddress, uint64_t u64Value)
		{
			if (!WriteProcessMemory(hProcess, (BYTE*)cbAddress, &u64Value, sizeof(u64Value), NULL))
			{
				DWORD dwError = GetLastError();
				MessageBox::Show("Failed to write to address, error code: " + dwError.ToString(), "ERROR");
			}
		}

		void WriteStringToMemory(HANDLE hProcess, BYTE* cbAddress, const char* cNewString)
		{
			// Calculate the size of the string, including the null terminator
			size_t uDataSize = strlen(cNewString) + 1;

			if (!WriteProcessMemory(hProcess, (LPVOID)cbAddress, cNewString, uDataSize, 0))
			{
				DWORD dwError = GetLastError();
				MessageBox::Show("Failed to write to address, error code: " + dwError.ToString(), "ERROR");
			}
		}

	private: System::Windows::Forms::TextBox^ txtNewValue;
	public:

	private: System::Windows::Forms::Button^ btnCancel;
	private: System::Windows::Forms::Button^ btnOK;
	private: System::Windows::Forms::Label^ lblEnterNewValue;




	private:
		BYTE* cbAddress;
		DWORD dwProcessId;
		int iDataType;
	protected:
		~NewValueForm()
		{
			if (components)
			{
				delete components;
			}
		}

	private:
		System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->txtNewValue = (gcnew System::Windows::Forms::TextBox());
			this->btnCancel = (gcnew System::Windows::Forms::Button());
			this->btnOK = (gcnew System::Windows::Forms::Button());
			this->lblEnterNewValue = (gcnew System::Windows::Forms::Label());
			this->SuspendLayout();
			// 
			// txtNewValue
			// 
			this->txtNewValue->Location = System::Drawing::Point(12, 25);
			this->txtNewValue->Name = L"txtNewValue";
			this->txtNewValue->Size = System::Drawing::Size(468, 20);
			this->txtNewValue->TabIndex = 0;
			this->txtNewValue->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &NewValueForm::txtNewValue_KeyDown);
			// 
			// btnCancel
			// 
			this->btnCancel->Location = System::Drawing::Point(405, 51);
			this->btnCancel->Name = L"btnCancel";
			this->btnCancel->Size = System::Drawing::Size(75, 23);
			this->btnCancel->TabIndex = 1;
			this->btnCancel->Text = L"Cancel";
			this->btnCancel->UseVisualStyleBackColor = true;
			this->btnCancel->Click += gcnew System::EventHandler(this, &NewValueForm::btnCancel_Click);
			// 
			// btnOK
			// 
			this->btnOK->Location = System::Drawing::Point(324, 51);
			this->btnOK->Name = L"btnOK";
			this->btnOK->Size = System::Drawing::Size(75, 23);
			this->btnOK->TabIndex = 2;
			this->btnOK->Text = L"OK";
			this->btnOK->UseVisualStyleBackColor = true;
			this->btnOK->Click += gcnew System::EventHandler(this, &NewValueForm::btnOK_Click);
			// 
			// lblEnterNewValue
			// 
			this->lblEnterNewValue->AutoSize = true;
			this->lblEnterNewValue->Location = System::Drawing::Point(9, 7);
			this->lblEnterNewValue->Name = L"lblEnterNewValue";
			this->lblEnterNewValue->Size = System::Drawing::Size(87, 13);
			this->lblEnterNewValue->TabIndex = 3;
			this->lblEnterNewValue->Text = L"Enter new value:";
			// 
			// NewValueForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(489, 82);
			this->Controls->Add(this->lblEnterNewValue);
			this->Controls->Add(this->btnOK);
			this->Controls->Add(this->btnCancel);
			this->Controls->Add(this->txtNewValue);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"NewValueForm";
			this->ShowIcon = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Change Value";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion


	private: System::Void btnCancel_Click(System::Object^ sender, System::EventArgs^ e) {
		this->DialogResult = System::Windows::Forms::DialogResult::Cancel;
		this->Close();
	}
	private: System::Void btnOK_Click(System::Object^ sender, System::EventArgs^ e) {
		WriteNewValueToMemory();

		this->DialogResult = System::Windows::Forms::DialogResult::OK;
		this->Close();
	}
	private: System::Void txtNewValue_KeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e) {
		if (e->KeyCode == Keys::Enter)
		{
			WriteNewValueToMemory();

			e->SuppressKeyPress = true;

			this->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->Close();
		}
		else if (e->KeyCode == Keys::Escape)
		{
			this->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->Close();
		}
	}
	};
}
