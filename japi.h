#pragma once
#include "pch.h"
#include "jass.h"
#include <unordered_map>

#define JAPI_CALL __cdecl

// 用于记录japi函数的信息
struct JapiFuncStruct {
	void*		func_addr;
	const char* func_name;
	uint32_t	ret_type;	// 返回值类型
};

namespace japi {
	/*
		2D000000
		JapiFuncStruct		-- call japi
		0D000000
		00000000			-- mov r00, r00
		27000000
		00000000			-- ret		
		04000000
		00000000			-- endfunction
	*/

	// 注册japi函数
	// @param func_name: jass接口函数在jass脚本的名字
	// @param func_addr: 实际调用的内部函数的地址
	template <class R, class...Args>
	bool add_japi(const char* func_name, R(JAPI_CALL* func_addr)(Args...)) {
		if (func_name && func_addr) {
			JassFuncStruct* jass_func = jass::get_main_thread()->get_jass_func(func_name);
			if (jass_func) {
				OPCodeStruct* func = jass_func->func_addr;
				JapiFuncStruct* japi_func = new JapiFuncStruct();
				japi_func->func_name = jass_func->func_name;
				japi_func->func_addr = func_addr;
				japi_func->ret_type = jass_func->ret_type;

				// 重载jass函数
				func[0].op = OPCODE_CALLJAPI;
				func[0].r1 = 0;
				func[0].r2 = 0;
				func[0].r3 = 0;
				func[0].arg = (uint32_t)japi_func;

				func[1].op = OPCODE_MOVRR;
				func[1].r1 = 0;
				func[1].r2 = 0;
				func[1].r3 = 0;
				func[1].arg = 0;

				func[2].op = OPCODE_RETURN;
				func[2].r1 = 0;
				func[2].r2 = 0;
				func[2].r3 = 0;
				func[2].arg = 0;

				func[3].op = OPCODE_ENDFUNCTION;
				func[3].r1 = 0;
				func[3].r2 = 0;
				func[3].r3 = 0;
				func[3].arg = 0;

				return true;
			}
		}
		return false;
	}
}

void japi_init();