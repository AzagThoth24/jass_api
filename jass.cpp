#include "pch.h"
#include "jass.h"

void* pGameDLL;

namespace jass {
	static uintptr_t getJassVM;
	static uintptr_t getCode;
}

using namespace jass;


// 获取jass函数结构
JassFuncStruct* JassVM::get_jass_func(const char* func_name) {
	return (this && func_name) ? this_call<JassFuncStruct*>(getCode, jvm.func_table, func_name) : nullptr;
}

// 获取jass线程
JassVM* jass::get_jass_thread(uint32_t index) {
	return fast_call<JassVM*>(getJassVM, index);
}

// 获取jass主线程
JassVM* jass::get_main_thread() {
	return get_jass_thread(1);
}

// 初始化
void jass_init() {
	pGameDLL = GetModuleHandleA("game.dll");

	// 内部函数地址
	getJassVM	= (uintptr_t)pGameDLL + 0x7E1100;
	getCode		= (uintptr_t)pGameDLL + 0x7EFBB0;
}