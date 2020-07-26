#include "DebugConsole.h"

void DebugConsole::Init() {
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	Redirect();
	std::cout << "Welcome to the debug console!" << std::endl << std::endl;
}

void DebugConsole::Redirect() {
	FILE* commandLine;
	freopen_s(&commandLine, "CON", "w", stdout);
}
