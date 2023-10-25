#include "MainForm.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThreadAttribute]

void main()
{
	Application::SetCompatibleTextRenderingDefault(false);
	Application::EnableVisualStyles();

	MemoryScanner::MainForm frmMainForm;
	Application::Run(% frmMainForm);
}