#include "stdafx.h"
#include "included.h"


// Ӧ�ó������
int main(){
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	if (SUCCEEDED(CoInitialize(NULL)))
	{
		{
			ThisApp app;
            system("pause");
            app.Exit();
		}
		CoUninitialize();
	}
	return 0;
}