#include "stdafx.h"
#include "included.h"




// Ӧ�ó������

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE /* hPrevInstance */, LPWSTR /* lpCmdLine */, int nCmdShow){
    if (SUCCEEDED(CoInitialize(nullptr)))
    {
        AllocConsole();
        FILE*m_new_stdout_file = nullptr;
        freopen_s(&m_new_stdout_file, "CONOUT$", "w+t", stdout);
        {
            ThisApp app;
            system("pause");
            app.Exit();
        }
        CoUninitialize();
        fclose(m_new_stdout_file);
        FreeConsole();
    }
	return 0;
}



// �﷨�ļ�
WCHAR* ThisApp::s_GrammarFileName = L"Grammar.xml";