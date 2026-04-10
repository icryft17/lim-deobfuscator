#ifndef UTIL_OPERNAD_H
#define UTIL_OPERNAD_H

namespace util
{
	bool IsOpernadToOpernad(ZydisDecodedOperand src, ZydisDecodedOperand dst);

	ZydisEncoderOperand OperandDecodedToEncoded(ZydisDecodedOperand operand);
} // namespace util
#endif
