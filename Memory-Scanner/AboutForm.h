#pragma once

namespace MemoryScanner {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	public ref class AboutForm : public System::Windows::Forms::Form
	{
	public:
		AboutForm(void)
		{
			InitializeComponent();
		}

	protected:
		~AboutForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::PictureBox^ picMemoryScanner;
	protected:
	private: System::Windows::Forms::Label^ lblMemoryScanner;
	private: System::Windows::Forms::Label^ lblSeparator;
	private: System::Windows::Forms::Button^ btnOK;
	private: System::Windows::Forms::Label^ lblAboutLine1;
	private: System::Windows::Forms::Label^ lblAboutLine2;

	private: System::Windows::Forms::LinkLabel^ llbGitHub;


	private:
		System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(AboutForm::typeid));
			this->picMemoryScanner = (gcnew System::Windows::Forms::PictureBox());
			this->lblMemoryScanner = (gcnew System::Windows::Forms::Label());
			this->lblSeparator = (gcnew System::Windows::Forms::Label());
			this->btnOK = (gcnew System::Windows::Forms::Button());
			this->lblAboutLine1 = (gcnew System::Windows::Forms::Label());
			this->lblAboutLine2 = (gcnew System::Windows::Forms::Label());
			this->llbGitHub = (gcnew System::Windows::Forms::LinkLabel());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->picMemoryScanner))->BeginInit();
			this->SuspendLayout();
			// 
			// picMemoryScanner
			// 
			this->picMemoryScanner->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"picMemoryScanner.Image")));
			this->picMemoryScanner->Location = System::Drawing::Point(26, 12);
			this->picMemoryScanner->Name = L"picMemoryScanner";
			this->picMemoryScanner->Size = System::Drawing::Size(134, 136);
			this->picMemoryScanner->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
			this->picMemoryScanner->TabIndex = 0;
			this->picMemoryScanner->TabStop = false;
			// 
			// lblMemoryScanner
			// 
			this->lblMemoryScanner->AutoSize = true;
			this->lblMemoryScanner->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 27.75F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->lblMemoryScanner->ForeColor = System::Drawing::Color::ForestGreen;
			this->lblMemoryScanner->Location = System::Drawing::Point(186, 62);
			this->lblMemoryScanner->Name = L"lblMemoryScanner";
			this->lblMemoryScanner->Size = System::Drawing::Size(302, 42);
			this->lblMemoryScanner->TabIndex = 1;
			this->lblMemoryScanner->Text = L"Memory Scanner";
			// 
			// lblSeparator
			// 
			this->lblSeparator->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->lblSeparator->Location = System::Drawing::Point(8, 159);
			this->lblSeparator->Name = L"lblSeparator";
			this->lblSeparator->Size = System::Drawing::Size(500, 2);
			this->lblSeparator->TabIndex = 2;
			// 
			// btnOK
			// 
			this->btnOK->Location = System::Drawing::Point(430, 332);
			this->btnOK->Name = L"btnOK";
			this->btnOK->Size = System::Drawing::Size(75, 23);
			this->btnOK->TabIndex = 3;
			this->btnOK->Text = L"OK";
			this->btnOK->UseVisualStyleBackColor = true;
			this->btnOK->Click += gcnew System::EventHandler(this, &AboutForm::btnOK_Click);
			// 
			// lblAboutLine1
			// 
			this->lblAboutLine1->AutoSize = true;
			this->lblAboutLine1->Location = System::Drawing::Point(41, 183);
			this->lblAboutLine1->Name = L"lblAboutLine1";
			this->lblAboutLine1->Size = System::Drawing::Size(366, 13);
			this->lblAboutLine1->TabIndex = 4;
			this->lblAboutLine1->Text = L"This is a memory scanner/editor program. It works with x86 and x64 binaries.";
			// 
			// lblAboutLine2
			// 
			this->lblAboutLine2->AutoSize = true;
			this->lblAboutLine2->Location = System::Drawing::Point(41, 212);
			this->lblAboutLine2->Name = L"lblAboutLine2";
			this->lblAboutLine2->Size = System::Drawing::Size(312, 13);
			this->lblAboutLine2->TabIndex = 5;
			this->lblAboutLine2->Text = L"Written by Julian O. Rose. For more of my projects, check out my";
			// 
			// llbGitHub
			// 
			this->llbGitHub->AutoSize = true;
			this->llbGitHub->Location = System::Drawing::Point(350, 212);
			this->llbGitHub->Name = L"llbGitHub";
			this->llbGitHub->Size = System::Drawing::Size(40, 13);
			this->llbGitHub->TabIndex = 6;
			this->llbGitHub->TabStop = true;
			this->llbGitHub->Text = L"GitHub";
			this->llbGitHub->LinkClicked += gcnew System::Windows::Forms::LinkLabelLinkClickedEventHandler(this, &AboutForm::llbGitHub_LinkClicked);
			// 
			// AboutForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(517, 367);
			this->Controls->Add(this->llbGitHub);
			this->Controls->Add(this->lblAboutLine2);
			this->Controls->Add(this->lblAboutLine1);
			this->Controls->Add(this->btnOK);
			this->Controls->Add(this->lblSeparator);
			this->Controls->Add(this->lblMemoryScanner);
			this->Controls->Add(this->picMemoryScanner);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->KeyPreview = true;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"AboutForm";
			this->ShowIcon = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"About Memory Scanner";
			this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &AboutForm::AboutForm_KeyDown);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->picMemoryScanner))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void btnOK_Click(System::Object^ sender, System::EventArgs^ e) {
		this->Close();
	}
	private: System::Void AboutForm_KeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e) {
		if (e->KeyCode == Keys::Enter || e->KeyCode == Keys::Escape)
		{
			this->Close();
		}
	}
	private: System::Void llbGitHub_LinkClicked(System::Object^ sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^ e) {
		System::Diagnostics::Process::Start("https://github.com/JulianOzelRose");
	}
	};
}
