#pragma once
#include <string>
#include <windows.h>
#include <tlhelp32.h>

namespace MemoryScanner {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Diagnostics;

	public ref class ProcessListForm : public System::Windows::Forms::Form
	{
	public:
		ProcessListForm(void)
		{
			InitializeComponent();
		}

	public: ref class ProcessListItem
	{
	public:
		Process^ ProcessObj;
		String^ DisplayText;

		ProcessListItem(Process^ process, String^ displayText)
		{
			ProcessObj = process;
			DisplayText = displayText;
		}
	};

		  event System::EventHandler^ DataSent;

		  void OnDataSent(System::Object^ sender, System::EventArgs^ e)
		  {
			  DataSent(sender, e);
		  }

		  DWORD GetSelectedProcessId()
		  {
			  DWORD dwProcessId = -1;

			  // Check if a process is selected
			  if (lvwProcesses->SelectedItems->Count > 0)
			  {
				  // Get the selected process
				  ListViewItem^ lvwSelectedItem = lvwProcesses->SelectedItems[0];

				  // Retrieve the process from the ListViewItem's Tag
				  ProcessListItem^ processListItem = safe_cast<ProcessListItem^>(lvwSelectedItem->Tag);

				  if (processListItem != nullptr)
				  {
					  dwProcessId = processListItem->ProcessObj->Id;
				  }
			  }

			  return dwProcessId;
		  }

		  String^ GetSelectedProcessName()
		  {
			  String^ strProcessName;

			  // Check if a process is selected
			  if (lvwProcesses->SelectedItems->Count > 0)
			  {
				  // Get the selected process
				  ListViewItem^ lvwSelectedItem = lvwProcesses->SelectedItems[0];

				  // Retrieve the process from the ListViewItem's Tag
				  ProcessListItem^ processListItem = safe_cast<ProcessListItem^>(lvwSelectedItem->Tag);

				  if (processListItem != nullptr)
				  {
					  strProcessName = processListItem->ProcessObj->ProcessName;
				  }
			  }

			  return strProcessName;
		  }

	protected:
		~ProcessListForm()
		{
			if (components)
			{
				delete components;
			}
		}

	protected:
	private: System::Windows::Forms::Button^ btnOpen;
	private: System::Windows::Forms::Button^ btnCancel;
	private: System::Windows::Forms::ListView^ lvwProcesses;

	private: System::Windows::Forms::ColumnHeader^ columnHeader1;
	private:
		System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(ProcessListForm::typeid));
			this->btnOpen = (gcnew System::Windows::Forms::Button());
			this->btnCancel = (gcnew System::Windows::Forms::Button());
			this->lvwProcesses = (gcnew System::Windows::Forms::ListView());
			this->columnHeader1 = (gcnew System::Windows::Forms::ColumnHeader());
			this->SuspendLayout();
			// 
			// btnOpen
			// 
			this->btnOpen->Enabled = false;
			this->btnOpen->Location = System::Drawing::Point(83, 444);
			this->btnOpen->Name = L"btnOpen";
			this->btnOpen->Size = System::Drawing::Size(75, 23);
			this->btnOpen->TabIndex = 1;
			this->btnOpen->Text = L"Open";
			this->btnOpen->UseVisualStyleBackColor = true;
			this->btnOpen->Click += gcnew System::EventHandler(this, &ProcessListForm::btnOpen_Click);
			// 
			// btnCancel
			// 
			this->btnCancel->Location = System::Drawing::Point(164, 444);
			this->btnCancel->Name = L"btnCancel";
			this->btnCancel->Size = System::Drawing::Size(75, 23);
			this->btnCancel->TabIndex = 2;
			this->btnCancel->Text = L"Cancel";
			this->btnCancel->UseVisualStyleBackColor = true;
			this->btnCancel->Click += gcnew System::EventHandler(this, &ProcessListForm::btnCancel_Click);
			// 
			// lvwProcesses
			// 
			this->lvwProcesses->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(1) { this->columnHeader1 });
			this->lvwProcesses->HideSelection = false;
			this->lvwProcesses->Location = System::Drawing::Point(12, 12);
			this->lvwProcesses->Name = L"lvwProcesses";
			this->lvwProcesses->Size = System::Drawing::Size(308, 426);
			this->lvwProcesses->TabIndex = 3;
			this->lvwProcesses->UseCompatibleStateImageBehavior = false;
			this->lvwProcesses->View = System::Windows::Forms::View::Details;
			this->lvwProcesses->SelectedIndexChanged += gcnew System::EventHandler(this, &ProcessListForm::lvwProcesses_SelectedIndexChanged);
			this->lvwProcesses->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &ProcessListForm::lvwProcesses_KeyDown);
			// 
			// columnHeader1
			// 
			this->columnHeader1->Text = L"Process";
			this->columnHeader1->Width = 304;
			// 
			// ProcessListForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(332, 477);
			this->Controls->Add(this->lvwProcesses);
			this->Controls->Add(this->btnCancel);
			this->Controls->Add(this->btnOpen);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->MaximizeBox = false;
			this->Name = L"ProcessListForm";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Process List";
			this->Load += gcnew System::EventHandler(this, &ProcessListForm::ProcessListForm_Load);
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void btnCancel_Click(System::Object^ sender, System::EventArgs^ e) {
		DialogResult = System::Windows::Forms::DialogResult::Cancel;
		this->Close();
	}
	private: System::Void ProcessListForm_Load(System::Object^ sender, System::EventArgs^ e) {
		// Get a list of running processes
		array<Process^>^ processArray = Process::GetProcesses();

		// Populate the existing ListView with ProcessListItem objects
		for (int i = 0; i < processArray->Length; i++)
		{
			Process^ process = processArray[i];
			String^ strDisplayText = String::Format("{0} - {1}", process->ProcessName, process->Id);
			ProcessListItem^ processListItem = gcnew ProcessListItem(process, strDisplayText);

			ListViewItem^ lvwItem = gcnew ListViewItem(strDisplayText);

			// Store the ProcessListItem in the ListViewItem's Tag
			lvwItem->Tag = processListItem;

			lvwProcesses->Items->Add(lvwItem);
		}
	}
	private: System::Void btnOpen_Click(System::Object^ sender, System::EventArgs^ e) {
		DialogResult = System::Windows::Forms::DialogResult::OK;
		this->Close();
	}
	private: System::Void lvwProcesses_KeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e) {
		if (e->KeyCode == Keys::Enter)
		{
			if (lvwProcesses->SelectedItems->Count > 0)
			{
				DialogResult = System::Windows::Forms::DialogResult::OK;
			}
			else
			{
				DialogResult = System::Windows::Forms::DialogResult::Cancel;
			}

			this->Close();
		}

		if (e->KeyCode == Keys::Escape)
		{
			DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->Close();
		}
	}
	private: System::Void lvwProcesses_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {
		btnOpen->Enabled = true;
	}
	};
}
