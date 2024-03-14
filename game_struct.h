#pragma once
#include "fp_call.h"

// jass字节码的结构
struct OPCodeStruct {
	byte_t				r3;
	byte_t				r2;
	byte_t				r1;
	byte_t				op;
	uint32_t			arg;


};

// jass寄存器/变量的结构
struct JassRegister {
	JassRegister*		next_frame;				// 0x0 只在栈元素中，链表结构
	unsigned char		ukn_0x4[0x14];
	uint32_t			type_0x18;				// 0x18 寄存器类型
	uint32_t			type_0x1C;				// 0x1C	字节码赋予的类型
	uint32_t			value;					// 0x20
	uint32_t			is_local;				// 0x24 是局部变量
};//sizeof = 0x28

// jass符号的结构
struct JassSymbol {
	uint32_t			string_hash;			// 0x0
	unsigned char		ukn_0x4[0x10];
	char*				name;					// 0x14
	uint32_t			index;					// 0x18
};

// jass符号数组
struct JassSymbolArray {
	unsigned char		ukn_0x0[0x8];			// 似乎是两个计数
	JassSymbol**		symbol_array;			// 0x8
};

// jass符号表
struct JassSymbolStruct {
	OPCodeStruct*		start_line;				// 0x0 jass脚本字节码起始位置
	unsigned char		ukn_0x4[0x4];			// 似乎是脚本长度
	JassSymbolArray*	table;					// 0x8 符号表	符号表 + 0x8->符号数组
};

// jass变量表的结构
struct ScriptDataTable {
	// wip...
};

// jass字符串表的结构
struct JassStringTable {
	unsigned char		ukn_0x0[0x4];
	uint32_t			max_size;
	uintptr_t			string_array;
};

// jass栈帧的结构
struct JassStackStruct {
	JassStackStruct*	next_stack;				// 0x0	下一个栈 双向链表
	JassStackStruct*	last_stack;				// 0x4	上一个栈
	uint32_t			top_0x8;				// 0x8	栈顶
	JassRegister*		frame[0x20];			// 0xC	栈元素
	uint32_t			top_0x8C;				// 0x8C 栈顶
	ScriptDataTable		local_table;			// 0x90 局部变量表
};// 0x8C后是ScriptDataTable

// jass虚拟机的结构
struct JassVMStruct {
	unsigned char		ukn_0x0[0x20];
	OPCodeStruct*		opcode_struct;			// 0x20
	char				ukn_0x24[0x10];
	uint32_t			has_sleep;				// 0x34
	unsigned char		ukn_0x38[0xC];
	int32_t				oplimit;				// 0x44
	unsigned char		ukn_0x48[0x8];
	JassRegister		reg[0x100];				// 0x50		256个寄存器
	uint32_t			index;					// 0x2850
	uint32_t			ukn3;
	JassSymbolStruct*	symbol_table;			// 0x2858
	ScriptDataTable*	global_table;			// 0x285C
	unsigned char		ukn_0x2860[0x8];
	JassStackStruct*	stack_struct;			// 0x2868
	unsigned char		ukn_0x286C[0x8];
	JassStringTable*	string_table;		    // 0x2874
	unsigned char		ukn_0x2878[0xC];
	uintptr_t			func_table;				// 0x2884
	uint32_t			code_table;				// 0x2888
	unsigned char		ukn_0x288C[0x14];
	void*				set_handle_ref;			// 0x28A0
	uintptr_t			map_table;				// 0x28A4	CGameState**
};

// native函数的结构
struct NativeFuncStruct {
	void**				vf_table;				// 0x0
	uint32_t			string_hash;			// 0x4
	unsigned char		ukn_0x8[0x10];
	char*				func_name;				// 0x18
	void*				func_addr;				// 0x1C
	uint32_t			param_count;			// 0x20
	const char*			param_info;				// 0x24
	unsigned char		ukn_0x28[0x14];
}; // sizeof = 0x3C

// jass函数的结构
struct JassFuncStruct {
	uint32_t			string_hash;			// 0x0
	unsigned char		ukn_0x4[0x10];
	char*				func_name;				// 0x14
	OPCodeStruct*		func_addr;				// 0x18
	uint32_t			ret_type;				// 0x1C
}; // sizeof = 0x20

// 字节码
enum OPCODE :byte_t {
	OPCODE_MINLIMIT		= 0x00,
	OPCODE_ENDPROGRAM	= 0x01,
	OPCODE_OLDJUMP		= 0x02,
	OPCODE_FUNCTION		= 0x03,
	OPCODE_ENDFUNCTION	= 0x04,
	OPCODE_LOCAL		= 0x05,
	OPCODE_GLOBAL		= 0x06,
	OPCODE_CONSTANT		= 0x07,
	OPCODE_FUNCARG		= 0x08,
	OPCODE_EXTENDS		= 0x09,
	OPCODE_TYPE			= 0x0A,
	OPCODE_POPN			= 0x0B,
	OPCODE_MOVRL		= 0x0C,		// mov 寄存器, 字面值
	OPCODE_MOVRR		= 0x0D,		// mov 寄存器, 寄存器
	OPCODE_MOVRV		= 0x0E,		// mov 寄存器, 变量
	OPCODE_MOVRCODE		= 0x0F,		// mov 寄存器, 函数
	OPCODE_MOVRA		= 0x10,		// mov 寄存器, 数组
	OPCODE_MOVVR		= 0x11,		// mov 变量,   寄存器
	OPCODE_MOVAR		= 0x12,		// mov 数组,   寄存器
	OPCODE_PUSH			= 0x13,
	OPCODE_POP			= 0x14,
	OPCODE_CALLNATIVE	= 0x15,
	OPCODE_CALLJASS		= 0x16,
	OPCODE_I2R			= 0x17,
	OPCODE_AND			= 0x18,
	OPCODE_OR			= 0x19,
	OPCODE_EQUAL		= 0x1A,
	OPCODE_NOTEQUAL		= 0x1B,
	OPCODE_LESSEREQUAL	= 0x1C,
	OPCODE_GREATEREQUAL = 0x1D,
	OPCODE_LESSER		= 0x1E,
	OPCODE_GREATER		= 0x1F,
	OPCODE_ADD			= 0x20,
	OPCODE_SUB			= 0x21,
	OPCODE_MUL			= 0x22,
	OPCODE_DIV			= 0x23,
	OPCODE_MOD			= 0x24,
	OPCODE_NEGATE		= 0x25,
	OPCODE_NOT			= 0x26,
	OPCODE_RETURN		= 0x27,
	OPCODE_LABEL		= 0x28,
	OPCODE_JUMPIFTRUE	= 0x29,
	OPCODE_JUMPIFFALSE	= 0x2A,
	OPCODE_JUMP			= 0x2B,
	OPCODE_MAXLIMIT		= 0x2C,
	OPCODE_CALLJAPI		= 0x2D,		// 新增字节码，用于与jass交互
};

// 变量类型
enum OPCODE_VARIABLE_TYPE :uint32_t {
	OPCODE_VTYPE_NOTHING		= 0x0,		// nothing
	OPCODE_VTYPE_UNKNOWN		= 0x1,		// unknown
	OPCODE_VTYPE_NULL			= 0x2,		// null
	OPCODE_VTYPE_CODE			= 0x3,		// code
	OPCODE_VTYPE_INTEGER		= 0x4,		// integer
	OPCODE_VTYPE_REAL			= 0x5,		// real
	OPCODE_VTYPE_STRING			= 0x6,		// string
	OPCODE_VTYPE_HANDLE			= 0x7,		// handle
	OPCODE_VTYPE_BOOLEAN		= 0x8,		// boolean
	OPCODE_VTYPE_INTEGER_ARRAY	= 0x9,		// integer array
	OPCODE_VTYPE_REAL_ARRAY		= 0xA,		// real array
	OPCODE_VTYPE_STRING_ARRAY	= 0xB,		// string array
	OPCODE_VTYPE_HANDLE_ARRAY	= 0xC,		// handle array
	OPCODE_VTYPE_BOOLEAN_ARRAY	= 0xD,		// boolean array
};