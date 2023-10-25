#pragma once

namespace MemoryScanner {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	public ref class NewDescriptionForm : public System::Windows::Forms::Form
	{
	public:
		NewDescriptionForm(String^ strCurrentDescription)
		{
			InitializeComponent();

			txtNewDescription->Text = strCurrentDescription;
		}

	public: String^ GetNewDescription()
	{
		return txtNewDescription->Text;
	}

	protected:
		~NewDescriptionForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::TextBox^ txtNewDescription;
	private: System::Windows::Forms::Label^ lblEnterNewDescription;
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
			this->txtNewDescription = (gcnew System::Windows::Forms::TextBox());
			this->lblEnterNewDescription = (gcnew System::Windows::Forms::Label());
			this->btnCancel = (gcnew System::Windows::Forms::Button());
			this->btnOK = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			// 
			// txtNewDescription
			// 
			this->txtNewDescription->Location = System::Drawing::Point(12, 26);
			this->txtNewDescription->Name = L"txtNewDescription";
			this->txtNewDescription->Size = System::Drawing::Size(468, 20);
			this->txtNewDescription->TabIndex = 0;
			this->txtNewDescription->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &NewDescriptionForm::txtNewDescription_KeyDown);
			// 
			// lblEnterNewDescription
			// 
			this->lblEnterNewDescription->AutoSize = true;
			this->lblEnterNewDescription->Location = System::Drawing::Point(9, 9);
			this->lblEnterNewDescription->Name = L"lblEnterNewDescription";
			this->lblEnterNewDescription->Size = System::Drawing::Size(130, 13);
			this->lblEnterNewDescription->TabIndex = 1;
			this->lblEnterNewDescription->Text = L"Enter the new description:";
			// 
			// btnCancel
			// 
			this->btnCancel->Location = System::Drawing::Point(405, 53);
			this->btnCancel->Name = L"btnCancel";
			this->btnCancel->Size = System::Drawing::Size(75, 23);
			this->btnCancel->TabIndex = 2;
			this->btnCancel->Text = L"Cancel";
			this->btnCancel->UseVisualStyleBackColor = true;
			this->btnCancel->Click += gcnew System::EventHandler(this, &NewDescriptionForm::btnCancel_Click);
			// 
			// btnOK
			// 
			this->btnOK->Location = System::Drawing::Point(324, 53);
			this->btnOK->Name = L"btnOK";
			this->btnOK->Size = System::Drawing::Size(75, 23);
			this->btnOK->TabIndex = 3;
			this->btnOK->Text = L"OK";
			this->btnOK->UseVisualStyleBackColor = true;
			this->btnOK->Click += gcnew System::EventHandler(this, &NewDescriptionForm::btnOK_Click);
			// 
			// NewDescriptionForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(492, 88);
			this->Controls->Add(this->btnOK);
			this->Controls->Add(this->btnCancel);
			this->Controls->Add(this->lblEnterNewDescription);
			this->Controls->Add(this->txtNewDescription);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"NewDescriptionForm";
			this->ShowIcon = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Change Description";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void btnCancel_Click(System::Object^ sender, System::EventArgs^ e) {
		this->DialogResult = System::Windows::Forms::DialogResult::Cancel;
	}
	private: System::Void btnOK_Click(System::Object^ sender, System::EventArgs^ e) {
		this->DialogResult = System::Windows::Forms::DialogResult::OK;
	}
	private: System::Void txtNewDescription_KeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e) {
		if (e->KeyCode == Keys::Enter)
		{
			// Handle Enter key as OK
			btnOK->PerformClick();
		}
		else if (e->KeyCode == Keys::Escape)
		{
			// Handle Esc key as Cancel
			btnCancel->PerformClick();
		}
	}
	protected: virtual bool ProcessCmdKey(System::Windows::Forms::Message% msg, Keys keyData) override {
		if (keyData == Keys::Enter)
		{
			// Handle Enter key as OK
			btnOK->PerformClick();
			return true;
		}
		else if (keyData == Keys::Escape)
		{
			// Handle Esc key as Cancel
			btnCancel->PerformClick();
			return true;
		}

		return __super::ProcessCmdKey(msg, keyData);
	}
	};
}
