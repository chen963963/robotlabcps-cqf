#include <locale>
#include <iostream>
#include "UIApplication.h"

int main(int, char**)
{
	// ����locale��֧������
	setlocale(LC_ALL, "LC_CTYPE=.utf8");

	//UIApplication app;
	try
	{
		g_app.Run();
	}
	catch (const std::runtime_error& theError)
	{
		std::cerr << theError.what() << std::endl;
		return EXIT_FAILURE;
	}
	return 0;
}
