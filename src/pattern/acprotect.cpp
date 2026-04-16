#include "acprotect.h"

namespace pattern
{
	namespace ac
	{
		bool JB_JAE_JNP_JB_JL_JGE_JMP_MEM(type::PatternContext *pattern_ctx)
		{
			type::Context *ctx = pattern_ctx->GetContext();
			IMAGE_NT_HEADERS *header = ctx->current_file->GetPEHeader();

			type::VirtualBlock *current_block = pattern_ctx->GetBlock();
			type::VirtualInstruction branch_instruction = current_block->LastInstruction();
			ZydisDisassembledInstruction disassembly_instruction = branch_instruction.Get();
			if (branch_instruction.IsCatagory(ZYDIS_CATEGORY_RET))
				return false;

			std::size_t operand_address =
				ABSOLUTE_ADDRESS(disassembly_instruction.runtime_address, disassembly_instruction.operands->imm.value.u,
								 disassembly_instruction.info.length);
			std::size_t next_address = branch_instruction.RuntimeAddress() + branch_instruction.Size();
			if (operand_address == (next_address + 1))
			{
				std::uint8_t *phy_instruction = branch_instruction.PhysicalAddress() + branch_instruction.Size();
				if ((*phy_instruction) == 0x90)
					return false;

				std::memset((branch_instruction.PhysicalAddress() + branch_instruction.Size()), 0x90, 1);

				LOG_DEBUG("pattern one -> JB_JAE_JNP_JB_JL_JGE_MEM -> {}", disassembly_instruction.text);

				return true;
			}

			return false;
		}

		bool CLD_CLD_STC_STC_CLC_STC(type::PatternContext *pattern_ctx)
		{
			type::VirtualBlock *current_block = pattern_ctx->GetBlock();
			type::VirtualInstruction last_instruction = current_block->LastInstruction();

			ZydisDisassembledInstruction disassm_instruction = last_instruction.Get();

			if (!last_instruction.Is(ZYDIS_MNEMONIC_CLD) && !last_instruction.Is(ZYDIS_MNEMONIC_STC) &&
				!last_instruction.Is(ZYDIS_MNEMONIC_CLC))
				return false;

			type::VirtualInstruction next_instruction = util::DecodeWithNotNOPInstruction(
				disassm_instruction.info.machine_mode, last_instruction.PhysicalAddress() + last_instruction.Size(),
				last_instruction.RuntimeAddress() + last_instruction.Size());

			const bool is_clc_stc = last_instruction.Is(ZYDIS_MNEMONIC_CLC) && next_instruction.Is(ZYDIS_MNEMONIC_STC);
			if (is_clc_stc)
			{
				util::DeleteInstruction(current_block, last_instruction);

				std::memset(next_instruction.PhysicalAddress(), 0x90, next_instruction.Size());

				LOG_DEBUG("pattern -> CLD_CLD_STC_STC_CLC_STC -> {} {}", disassm_instruction.text,
						  next_instruction.Get().text);

				return true;
			}

			const bool is_cld = last_instruction.Is(ZYDIS_MNEMONIC_CLD) && next_instruction.Is(ZYDIS_MNEMONIC_CLD);
			const bool is_stc = last_instruction.Is(ZYDIS_MNEMONIC_STC) && next_instruction.Is(ZYDIS_MNEMONIC_STC);

			if (is_cld || is_stc)
			{
				std::memset(next_instruction.PhysicalAddress(), 0x90, next_instruction.Size());

				return true;
			}

			return false;
		}

		bool INC_DEC(type::PatternContext *pattern_ctx)
		{
			type::VirtualBlock *current_block = pattern_ctx->GetBlock();
			type::List<type::VirtualInstruction> list = current_block->List();

			if (list.Size() < 2)
				return false;

			type::VirtualInstruction last_instruction = current_block->LastInstruction();
			type::VirtualInstruction prev_instruction = list.Last()->Prev()->Value();

			const bool inc_dec = prev_instruction.Is(ZYDIS_MNEMONIC_INC) && last_instruction.Is(ZYDIS_MNEMONIC_DEC);
			const bool dec_inc = prev_instruction.Is(ZYDIS_MNEMONIC_DEC) && last_instruction.Is(ZYDIS_MNEMONIC_INC);
			if (inc_dec || dec_inc)
			{
				util::DeleteInstruction(current_block, last_instruction);
				util::DeleteInstruction(current_block, prev_instruction);

				LOG_DEBUG("pattern INC_DEC", "");

				return true;
			}

			return false;
		}

		/*
			push deobuf_object.46B193
			test ecx,edx
			pop eax

			--------

			push deobuf_object.46B193
			pop eax

		*/

		bool PUSH_POP(type::PatternContext *pattern_ctx)
		{
			type::VirtualBlock *current_block = pattern_ctx->GetBlock();
			type::ControlFlowGraph *graph = pattern_ctx->Graph();

			type::VirtualInstruction last_instruction = current_block->LastInstruction();
			type::List<type::VirtualInstruction> list = current_block->List();

			ZydisDisassembledInstruction disassm_instruction = last_instruction.Get();

			if (pattern_ctx->IsPred())
			{
				if (list.Size() < 2)
					return false;

				type::VirtualInstruction push_instruction = list.Last()->Prev()->Value();

				if (push_instruction.Is(ZYDIS_MNEMONIC_PUSH))
				{
					tsl::robin_map<std::size_t, type::VirtualEdge> edges = current_block->Edges();
					if (!edges.empty())
					{
						for (auto edge : edges)
						{
							type::VirtualEdge virtual_edge = edge.second;
							type::VirtualBlock *virtual_block = graph->Get(edge.first);
							if (!virtual_block)
								continue;

							type::VirtualInstruction first_instruction = virtual_block->FirstInstruction();

							if (first_instruction.Is(ZYDIS_MNEMONIC_POP))
							{
								ZydisDisassembledInstruction one_disassm_instruction = push_instruction.Get();
								ZydisDisassembledInstruction two_disassm_instruction = first_instruction.Get();

								ZydisDecodedOperand &src = one_disassm_instruction.operands[0];
								ZydisDecodedOperand &dst = two_disassm_instruction.operands[0];

								// if (src.type == ZYDIS_OPERAND_TYPE_REGISTER && dst.type ==
								// ZYDIS_OPERAND_TYPE_MEMORY)
								//{
								ZydisEncoderRequest req{};

								req.machine_mode = disassm_instruction.info.machine_mode;
								req.operand_count = 2;

								req.mnemonic = ZYDIS_MNEMONIC_MOV;

								req.operands[0] = util::OperandDecodedToEncoded(dst);
								req.operands[1] = util::OperandDecodedToEncoded(src);

								std::size_t lenght = sizeof(req);

								std::uint8_t buffer_instruction[ZYDIS_MAX_INSTRUCTION_LENGTH];
								if (ZYAN_SUCCESS(ZydisEncoderEncodeInstruction(&req, buffer_instruction, &lenght)))
								{
									std::size_t total_lenght = push_instruction.Size();
									if (lenght <= total_lenght)
									{

										util::DeleteInstruction(current_block, push_instruction);
										util::DeleteInstruction(virtual_block, first_instruction);

										std::memcpy(push_instruction.PhysicalAddress(), buffer_instruction, lenght);

										LOG_DEBUG("pattern push pop block {} {}", push_instruction.Get().text,
												  first_instruction.Get().text);

										return true;
									}
								}
							}
						}
					}
				}
			}

			if (!last_instruction.IsCatagory(ZYDIS_CATEGORY_PUSH))
			{
				if (last_instruction.IsCatagory(ZYDIS_CATEGORY_POP))
				{
					if (list.Size() < 3)
						return false;

					type::VirtualInstruction push_unstruction = list.Last()->Prev()->Prev()->Value();
					if (!push_unstruction.Is(ZYDIS_MNEMONIC_PUSH))
						return false;

					ZydisDisassembledInstruction one_disassm_instruction = push_unstruction.Get();
					ZydisDisassembledInstruction two_disassm_instruction = last_instruction.Get();

					ZydisDecodedOperand &src = one_disassm_instruction.operands[0];
					ZydisDecodedOperand &dst = two_disassm_instruction.operands[0];

					ZydisEncoderRequest req{};

					req.machine_mode = disassm_instruction.info.machine_mode;
					req.operand_count = 2;

					req.mnemonic = ZYDIS_MNEMONIC_MOV;

					req.operands[0] = util::OperandDecodedToEncoded(dst);
					req.operands[1] = util::OperandDecodedToEncoded(src);

					std::size_t lenght = sizeof(req);

					std::uint8_t buffer_instruction[ZYDIS_MAX_INSTRUCTION_LENGTH];
					if (ZYAN_SUCCESS(ZydisEncoderEncodeInstruction(&req, buffer_instruction, &lenght)))
					{
						std::size_t total_lenght = push_unstruction.Size();
						if (lenght <= total_lenght)
						{
							util::DeleteInstruction(current_block, push_unstruction);
							util::DeleteInstruction(current_block, last_instruction);

							std::memcpy(push_unstruction.PhysicalAddress(), buffer_instruction, lenght);

							return true;
						}
					}
					//}
				}
				return false;
			}

			type::VirtualInstruction next_instruction = util::DecodeWithNotNOPInstruction(
				disassm_instruction.info.machine_mode, last_instruction.PhysicalAddress() + last_instruction.Size(),
				last_instruction.RuntimeAddress() + last_instruction.Size());

			if (next_instruction.Is(ZYDIS_MNEMONIC_POPAD) && last_instruction.Is(ZYDIS_MNEMONIC_PUSHAD))
			{
				util::DeleteInstruction(current_block, last_instruction);

				// std::memset(last_instruction.PhysicalAddress(), 0x90, last_instruction.Size());
				std::memset(next_instruction.PhysicalAddress(), 0x90, next_instruction.Size());

				LOG_DEBUG("pattern pushad popad -> {} {}", disassm_instruction.text, next_instruction.Get().text);

				return true;
			}

			if (last_instruction.Is(ZYDIS_MNEMONIC_PUSH) && next_instruction.Is(ZYDIS_MNEMONIC_POP))
			{
				ZydisDisassembledInstruction one_disassm_instruction = last_instruction.Get();
				ZydisDisassembledInstruction two_disassm_instruction = next_instruction.Get();

				ZydisDecodedOperand &src = one_disassm_instruction.operands[0];
				ZydisDecodedOperand &dst = two_disassm_instruction.operands[0];

				if (util::IsOpernadToOpernad(src, dst))
				{
					util::DeleteInstruction(current_block, last_instruction);
					std::memset(next_instruction.PhysicalAddress(), 0x90, next_instruction.Size());

					LOG_DEBUG("pattern push pop -> {} {} -> {} {}", last_instruction.Get().text,
							  next_instruction.Get().text, last_instruction.RuntimeAddress(),
							  next_instruction.RuntimeAddress());

					return true;
				}

				const bool reg_memory =
					src.type == ZYDIS_OPERAND_TYPE_REGISTER && dst.type == ZYDIS_OPERAND_TYPE_MEMORY;
				const bool imm_reg =
					src.type == ZYDIS_OPERAND_TYPE_IMMEDIATE && dst.type == ZYDIS_OPERAND_TYPE_REGISTER;

				if (reg_memory || imm_reg)
				{
					ZydisEncoderRequest req{};

					req.machine_mode = disassm_instruction.info.machine_mode;
					req.operand_count = 2;

					req.mnemonic = ZYDIS_MNEMONIC_MOV;

					req.operands[0] = util::OperandDecodedToEncoded(dst);
					req.operands[1] = util::OperandDecodedToEncoded(src);

					std::size_t lenght = sizeof(req);

					std::uint8_t buffer_instruction[ZYDIS_MAX_INSTRUCTION_LENGTH];
					if (ZYAN_SUCCESS(ZydisEncoderEncodeInstruction(&req, buffer_instruction, &lenght)))
					{
						std::size_t total_lenght = last_instruction.Size() + next_instruction.Size();
						if (lenght < total_lenght)
						{
							util::DeleteInstruction(current_block, last_instruction);
							// std::memset(last_instruction.PhysicalAddress(), 0x90, last_instruction.Size());
							std::memset(next_instruction.PhysicalAddress(), 0x90, next_instruction.Size());

							std::memcpy(last_instruction.PhysicalAddress(), buffer_instruction, lenght);

							return true;
						}
					}
				}
			}

			return false;
		}

		bool MOV_XOR_SUB(type::PatternContext *pattern_ctx)
		{
			type::VirtualBlock *current_block = pattern_ctx->GetBlock();
			type::VirtualInstruction last_instruction = current_block->LastInstruction();
			type::ControlFlowGraph *graph = pattern_ctx->Graph();

			type::List<type::VirtualInstruction> instructions = current_block->List();

			if (pattern_ctx->IsPred())
			{
				if (instructions.Size() < 2)
					return false;

				type::VirtualInstruction mov_instruction = instructions.Last()->Prev()->Value();
				if (mov_instruction.Is(ZYDIS_MNEMONIC_MOV))
				{
					tsl::robin_map<std::size_t, type::VirtualEdge> edges = current_block->Edges();
					if (!edges.empty())
					{
						for (auto edge : edges)
						{
							type::VirtualEdge virtual_edge = edge.second;

							if (virtual_edge.Type() != type::PositiveEdge)
								continue;

							type::VirtualBlock *virtual_block = graph->Get(virtual_edge.To());
							type::VirtualInstruction first_instruction = virtual_block->FirstInstruction();

							ZydisDisassembledInstruction disassm_mov = mov_instruction.Get();
							ZydisDisassembledInstruction disassm_instructuion = first_instruction.Get();

							ZydisDecodedOperand &one_opernad = disassm_mov.operands[0];
							ZydisDecodedOperand &two_opernad = disassm_instructuion.operands[0];

							if (util::IsOpernadToOpernad(one_opernad, two_opernad))
							{
								if (disassm_mov.operands[1].type == ZYDIS_OPERAND_TYPE_IMMEDIATE &&
									disassm_instructuion.operands[1].type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
								{
									std::size_t instruction_value = disassm_instructuion.operands[1].imm.value.s;
									std::size_t mov_value = disassm_mov.operands[1].imm.value.s;

									switch (disassm_instructuion.info.mnemonic)
									{
									case ZYDIS_MNEMONIC_SUB:
									{
										mov_value = mov_value - instruction_value;
										break;
									}

									case ZYDIS_MNEMONIC_XOR:
									{
										mov_value = mov_value ^ instruction_value;
										break;
									}

									case ZYDIS_MNEMONIC_ADD:
									{
										mov_value = mov_value + instruction_value;
										break;
									}
									}

									disassm_mov.operands[1].imm.value.s = mov_value;

									mov_instruction.SetOperand(1, disassm_mov.operands[1]);

									virtual_block->DeleteInstruction(virtual_block->List().Begin());
								}
							}
						}
					}
				}

				return false;
			}

			if (!last_instruction.Is(ZYDIS_MNEMONIC_XOR) && !last_instruction.Is(ZYDIS_MNEMONIC_SUB) &&
				last_instruction.Is(ZYDIS_MNEMONIC_ADD))
				return false;

			type::List<type::VirtualInstruction> list_instruction = current_block->List();
			if (list_instruction.Size() < 3)
				return false;

			type::VirtualInstruction mov_instruction = list_instruction.Last()->Prev()->Prev()->Value();
			if (mov_instruction.Is(ZYDIS_MNEMONIC_MOV))
			{
				ZydisDisassembledInstruction disassm_mov = mov_instruction.Get();
				ZydisDisassembledInstruction disassm_instructuion = last_instruction.Get();

				ZydisDecodedOperand &one_opernad = disassm_mov.operands[0];
				ZydisDecodedOperand &two_opernad = disassm_instructuion.operands[0];

				if (util::IsOpernadToOpernad(one_opernad, two_opernad))
				{
					if (disassm_mov.operands[1].type == ZYDIS_OPERAND_TYPE_IMMEDIATE &&
						disassm_instructuion.operands[1].type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
					{
						std::size_t instruction_value = disassm_instructuion.operands[1].imm.value.s;
						std::size_t mov_value = disassm_mov.operands[1].imm.value.s;

						switch (disassm_instructuion.info.mnemonic)
						{
						case ZYDIS_MNEMONIC_SUB:
						{
							mov_value = mov_value - instruction_value;
							break;
						}

						case ZYDIS_MNEMONIC_XOR:
						{
							mov_value = mov_value ^ instruction_value;
							break;
						}

						case ZYDIS_MNEMONIC_ADD:
						{
							mov_value = mov_value + instruction_value;
							break;
						}
						}

						disassm_mov.operands[1].imm.value.s = mov_value;

						mov_instruction.SetOperand(1, disassm_mov.operands[1]);

						current_block->DeleteInstruction(list_instruction.Last());

						return true;
					}
				}
			}

			return false;
		}

		bool CALL_MEM(type::PatternContext *pattern_ctx)
		{
			type::Context *ctx = pattern_ctx->GetContext();
			IMAGE_NT_HEADERS *header = ctx->current_file->GetPEHeader();

			type::VirtualBlock *current_block = pattern_ctx->GetBlock();
			type::VirtualInstruction call_instruction = current_block->LastInstruction(); // bad construction
			ZydisDisassembledInstruction disassembly_instruction = call_instruction.Get();

			type::VirtualInstruction last_instruction = util::DecodeInstruction(
				disassembly_instruction.info.machine_mode, call_instruction.PhysicalAddress() + call_instruction.Size(),
				call_instruction.RuntimeAddress() + call_instruction.Size());

			if (!call_instruction.IsCatagory(ZYDIS_CATEGORY_CALL))
				return false;

			std::size_t call_address =
				ABSOLUTE_ADDRESS(call_instruction.RuntimeAddress(), disassembly_instruction.operands->imm.value.u,
								 call_instruction.Size());

			if (call_address == (last_instruction.RuntimeAddress() + 1))
			{
				LOG_DEBUG("pattern -> CALL_MEM -> {}", disassembly_instruction.text);

				type::VirtualInstruction under_instruction = util::DecodeInstruction(
					disassembly_instruction.info.machine_mode,
					&ctx->current_file
						 ->Buffer()[ctx->current_file->GetRAWValue(call_address - header->OptionalHeader.ImageBase)],
					call_address);

				ZydisDisassembledInstruction under_disassembly_instruction = under_instruction.Get();

				if (under_instruction.Is(ZYDIS_MNEMONIC_ADD))
				{
					switch (under_disassembly_instruction.operands[1].imm.value.u)
					{

					case 0x6:
					{
						std::size_t new_jump =
							ABSOLUTE_ADDRESS(call_instruction.RuntimeAddress(), 0x6, call_instruction.Size());
						ZydisEncoderRequest req{};
						req.mnemonic = ZYDIS_MNEMONIC_JMP;
						req.operands->imm.u = new_jump - (call_instruction.RuntimeAddress() + 0x2);
						req.operands->type = ZYDIS_OPERAND_TYPE_IMMEDIATE;
						req.machine_mode = ZYDIS_MACHINE_MODE_LEGACY_32;
						req.operand_count = 1;
						std::size_t lenght = sizeof(req);

						if (ZYAN_SUCCESS(
								ZydisEncoderEncodeInstruction(&req, call_instruction.PhysicalAddress(), &lenght)))
						{
							std::memset(call_instruction.PhysicalAddress() + lenght, 0x90, req.operands->imm.u);

							LOG_DEBUG("pattern 0x6 -> CALL_MEM -> reset {}", disassembly_instruction.text);

							current_block->DeleteInstruction(current_block->List().Last());

							return true;
						}

						return false;
					}

					case 0x4:
					{
						std::memset(call_instruction.PhysicalAddress(), 0x90,
									(call_instruction.Size() + under_instruction.Size() + 1));

						LOG_DEBUG("pattern 0x4 -> CALL_MEM -> reset {}", disassembly_instruction.text);

						current_block->DeleteInstruction(current_block->List().Last());

						return true;
					}
					default:
						LOG_WARN("pattern -> CALL_MEM -> NOT FOUND IMM VALUE");
					}
				}
			}

			return false;
		}
	} // namespace ac
} // namespace pattern
