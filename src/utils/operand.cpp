#include "operand.h"

namespace util
{
	bool IsOpernadToOpernad(ZydisDecodedOperand src, ZydisDecodedOperand dst)
	{
		if (src.type != dst.type)
			return false;

		switch (dst.type)
		{
		case ZYDIS_OPERAND_TYPE_MEMORY:
		{
			if (src.mem.segment != dst.mem.segment)
				return false;
			if (src.mem.disp.value != dst.mem.disp.value)
				return false;
			break;
		}
		case ZYDIS_OPERAND_TYPE_POINTER:
		{
			if (src.ptr.segment != dst.ptr.segment)
				return false;

			if (src.ptr.offset != src.ptr.offset)
				return false;
			break;
		}
		case ZYDIS_OPERAND_TYPE_REGISTER:
		{
			if (src.reg.value != dst.reg.value)
				return false;
			break;
		}
		default:
			return false;
		}
		return true;
	}

	ZydisEncoderOperand OperandDecodedToEncoded(ZydisDecodedOperand operand)
	{
		ZydisEncoderOperand encoded{};
		encoded.type = operand.type;

		encoded.reg.value = operand.reg.value;
		encoded.reg.is4 = ZYAN_FALSE;

		encoded.mem.base = operand.mem.base;
		encoded.mem.index = operand.mem.index;
		encoded.mem.scale = operand.mem.scale;
		encoded.mem.displacement = operand.mem.disp.value;
		encoded.mem.size = operand.size / 8;

		encoded.ptr.segment = operand.ptr.segment;
		encoded.ptr.offset = operand.ptr.offset;

		encoded.imm.u = operand.imm.value.u;
		encoded.imm.s = operand.imm.value.s;

		return encoded;
	}
} // namespace util
