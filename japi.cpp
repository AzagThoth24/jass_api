#include "pch.h"
#include "japi.h"

namespace japi {
	
	// 解释器hook
	namespace parser_hook {
		static uintptr_t jmpInstruction;	// 跳转指令的地址
		static uintptr_t cmpInstruction;	// 比较指令的地址
		static uintptr_t incOpcodeStep;		// 增长字节码的地址

		// 参数缓冲区，最大支持16个参数
		static uint32_t param_buffer[0x10];

		// 跳转表 长度为最后的那个字节码 - 1
		static void* opcodeSwitchTable[OPCODE_CALLJAPI - 1];

		// 调用器
		// @param japi_func: japi函数的地址
		static uint32_t __fastcall caller(JapiFuncStruct* japi_func);

		// 跳板
		static void board();

		// handler
		// 通过跳板来调用，注意调用约定和参数顺序需与跳板中的调用方式一致
		// @param japi_func: japi函数的地址
		// @param opcode: 当前jass语句地址
		static void __fastcall handler(JassVM* jvm, JapiFuncStruct* japi_func, OPCodeStruct* opcode);

		// 初始化hook
		static void init();
	}

}

using namespace japi;

// 解释器hook

// 调用器
// 不确定参数个数，因此使用naked函数配合内联汇编来写这个函数
uint32_t __declspec(naked) __fastcall parser_hook::caller(JapiFuncStruct* japi_func) {
	__asm {
		push ebp;
		mov ebp, esp;
		push dword ptr ds : [param_buffer + 0x3C];
		push dword ptr ds : [param_buffer + 0x38];
		push dword ptr ds : [param_buffer + 0x34];
		push dword ptr ds : [param_buffer + 0x30];
		push dword ptr ds : [param_buffer + 0x2C];
		push dword ptr ds : [param_buffer + 0x28];
		push dword ptr ds : [param_buffer + 0x24];
		push dword ptr ds : [param_buffer + 0x20];
		push dword ptr ds : [param_buffer + 0x1C];
		push dword ptr ds : [param_buffer + 0x18];
		push dword ptr ds : [param_buffer + 0x14];
		push dword ptr ds : [param_buffer + 0x10];
		push dword ptr ds : [param_buffer + 0xC];
		push dword ptr ds : [param_buffer + 0x8];
		push dword ptr ds : [param_buffer + 0x4];
		push dword ptr ds : [param_buffer + 0x0];
		call dword ptr ds : [ecx] ;	// 函数地址是JapiFuncStruct中第一个成员变量
		add esp, 0x40;
		mov esp, ebp;
		pop ebp;
		ret;
	}
}

// 跳板
// 跳转时: eax = op - 2; ebx = jvm; edx = param; edi = opcode
// 因此使用naked函数配合内联汇编来写这个函数
// 注意1.27a与其他版本的寄存器有区别，如果想支持其他版本可以多写几个跳板
void __declspec(naked) parser_hook::board() {
	__asm {
		pushad;
		push edi;
		mov ecx, ebx;
		call handler;
		popad;
		jmp dword ptr ds : [eax * 0x4 + opcodeSwitchTable] ;
	}
}

// handler
// 通过跳板来调用
void __fastcall parser_hook::handler(JassVM* jvm, JapiFuncStruct* japi_func, OPCodeStruct* opcode) {
	if (!japi_func || opcode->op != OPCODE_CALLJAPI) return;	// 安全性判定，同时只处理OPCODE_CALLJAPI

	// 将jass函数栈中的元素dump到缓冲区
	auto dump_param = [=]() {
		JassStackStruct* stack = jvm->jvm.stack_struct;

		// 无栈帧时这个位置的值是一个地址的按位取反，因此需要判定非负数
		if ((intptr_t)stack > 0) {
			stack = stack->last_stack;		// jass函数被调用的时候，参数存放在last_stack中

			// 栈中元素的个数。jass函数被调用时栈中会固定存放一个返回点，因此top等于1时是无参数的，大于1才是有参数的
			int32_t top = stack->top_0x8C - 1; // 

			while (top > 0)
			{
				param_buffer[top - 1] = stack->frame[top - 1]->value;
				--top;
			}
		}
	};

	dump_param();
	uint32_t res = caller(japi_func);	// 调用japi函数，并记录其返回值

	// 如果函数有返回值，则设置r00的值，因为r00寄存器固定存放返回值
	uint32_t ret_type = japi_func->ret_type;
	if (ret_type) {
		JassRegister& r00 = jvm->jvm.reg[0];
		r00.value = res;
		r00.type_0x18 = ret_type;
		r00.type_0x1C = ret_type;
	}
}

/*
	-0x3 83C0	FE		: add eax, 0xFFFFFFFE
	+0x0 83F8	29		: cmp eax, 0x29				<---- cmpInstruction
	+0x3 0F87	DWORD	: ja incOpcodeStep
	+0x9 FF2485 DWORD	: jmp dword ptr ds:[eax * 4 + opcodeSwitchTable]	<---- jmpInstruction
*/

// 初始化hook
void parser_hook::init() {
	DWORD old_prot;
	VirtualProtect((void*)cmpInstruction, 0x20, 0x40, &old_prot);

	// 更改跳转条件
	*(byte_t*)(cmpInstruction + 0x2) = 0x2B;		// 0x2B = (CALL_JAPI - 2)

	// copy 字节码跳转表
	void** switchTable = *(void***)(jmpInstruction + 0x3);

	for (uintptr_t index = 0; index < OPCODE_CALLJAPI - 1; ++index)
	{
		if (index > 0x2A) {
			// CALLJAPI已经在handler中处理完毕，因此这里只需为其指定增长字节码的地址即可
			// 所有其他字节码运行完毕后都会跳转到这个增长字节码的位置来
			opcodeSwitchTable[index] = (void*)incOpcodeStep;
		}
		else
		{
			opcodeSwitchTable[index] = switchTable[index];
		}
	}

	*(byte_t*)jmpInstruction = 0xE9;
	*(uintptr_t*)(jmpInstruction + 0x1) = (uintptr_t)board - jmpInstruction - 0x5;	// hook字节码跳转表
	*(WORD*)(jmpInstruction + 0x5) = 0x9090;
	VirtualProtect((void*)cmpInstruction, 0x20, old_prot, &old_prot);
}

void japi_init() {
	parser_hook::cmpInstruction = (uintptr_t)pGameDLL + 0x7F1A9E;
	parser_hook::incOpcodeStep	= (uintptr_t)pGameDLL + 0x7F1C3D;
	parser_hook::jmpInstruction = (uintptr_t)pGameDLL + 0x7F1AA7;

	parser_hook::init();
}