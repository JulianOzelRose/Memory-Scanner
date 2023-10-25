#pragma once
#include "MemoryValue.h"

namespace MemoryScanner {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	public ref class ChangeTypeForm : public System::Windows::Forms::Form
	{
	public:
		ChangeTypeForm(MemoryValue^ memoryValue)
		{
			InitializeComponent();

			cbValueType->SelectedIndex = memoryValue->Type;
		}

		int GetNewDataType()
		{
			return cbValueType->SelectedIndex;
		}

	protected:
		~ChangeTypeForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::ComboBox^ cbValueType;
	private: System::Windows::Forms::Label^ lblSelectType;
	protected:

	protected:

	private: System::Windows::Forms::Button^ btnCancel;
	private: System::Windows::Forms::Button^ btnOK;



	private:
		System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->cbValueType = (gcnew System::Windows::Forms::ComboBox());
			this->lblSelectType = (gcnew System::Windows::Forms::Label());
			this->btnCancel = (gcnew System::Windows::Forms::Button());
			this->btnOK = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			// 
			// cbValueType
			// 
			this->cbValueType->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cbValueType->FormattingEnabled = true;
			this->cbValueType->Items->AddRange(gcnew cli::array< System::Object^  >(5) {
				L"Byte", L"2 Bytes", L"4 Bytes", L"8 Bytes",
					L"String"
			});
			this->cbValueType->Location = System::Drawing::Point(12, 24);
			this->cbValueType->Name = L"cbValueType";
			this->cbValueType->Size = System::Drawing::Size(198, 21);
			this->cbValueType->TabIndex = 0;
			// 
			// lblSelectType
			// 
			this->lblSelectType->AutoSize = true;
			this->lblSelectType->Location = System::Drawing::Point(10, 8);
			this->lblSelectType->Name = L"lblSelectType";
			this->lblSelectType->Size = System::Drawing::Size(104, 13);
			this->lblSelectType->TabIndex = 1;
			this->lblSelectType->Text = L"Select the new type:";
			// 
			// btnCancel
			// 
			this->btnCancel->Location = System::Drawing::Point(137, 51);
			this->btnCancel->Name = L"btnCancel";
			this->btnCancel->Size = System::Drawing::Size(75, 23);
			this->btnCancel->TabIndex = 2;
			this->btnCancel->Text = L"Cancel";
			this->btnCancel->UseVisualStyleBackColor = true;
			this->btnCancel->Click += gcnew System::EventHandler(this, &ChangeTypeForm::btnCancel_Click);
			// 
			// btnOK
			// 
			this->btnOK->Location = System::Drawing::Point(56, 51);
			this->btnOK->Name = L"btnOK";
			this->btnOK->Size = System::Drawing::Size(75, 23);
			this->btnOK->TabIndex = 3;
			this->btnOK->Text = L"OK";
			this->btnOK->UseVisualStyleBackColor = true;
			this->btnOK->Click += gcnew System::EventHandler(this, &ChangeTypeForm::btnOK_Click);
			// 
			// ChangeTypeForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(224, 84);
			this->Controls->Add(this->btnOK);
			this->Controls->Add(this->btnCancel);
			this->Controls->Add(this->lblSelectType);
			this->Controls->Add(this->cbValueType);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"ChangeTypeForm";
			this->ShowIcon = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Type";
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
	};
}
