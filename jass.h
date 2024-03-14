#pragma once
#include "game_struct.h"

extern void* pGameDLL;

// 包装一个jass虚拟机类来更方便地操作
class JassVM {
public:
	JassVMStruct jvm;

	// 获取jass函数结构
	JassFuncStruct* get_jass_func(const char* func_name);
};

namespace jass {
	// 获取jass线程
	JassVM* get_jass_thread(uint32_t index);
	// 获取jass主线程
	JassVM* get_main_thread();
}

void jass_init();