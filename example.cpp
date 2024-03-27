#include "pch.h"
#include "japi.h"
#include <iostream>

namespace JAPI_EXAMPLE {
	void JAPI_CALL test(int32_t arg1);
}

using namespace JAPI_EXAMPLE;

// test函数的真正功能
// 该函数在jass端的调用方式是takes integer returns nothing，因此在内部参数必须保持一致，返回值无所谓
void JAPI_CALL JAPI_EXAMPLE::test(int32_t arg1) {
	// 打开控制台
	auto open_console = []() {
		HWND h = ::GetConsoleWindow();
		if (!h) {
			FILE* new_file;
			::AllocConsole();
			::SetConsoleOutputCP(CP_UTF8); // 设置控制台编码为UTF8
			freopen_s(&new_file, "CONIN$", "r", stdin);
			freopen_s(&new_file, "CONOUT$", "w", stdout);
			freopen_s(&new_file, "CONOUT$", "w", stderr);
		}
		::ShowWindow(::GetConsoleWindow(), SW_SHOW);
	};

	open_console();
	std::cout	<< "调用c++函数: test" << std::endl
				<< "	arg1为: " << arg1 << std::endl;	// 在控制台输出
}

void example_init() {
	japi::add_japi("test", test);
}