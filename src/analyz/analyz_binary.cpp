#include "analyz.h"

namespace analyz
{
	type::ControlFlowGraph *AnalyzBinary(type::Context *ctx)
	{
		bool is_read = true;
		ZydisMachineMode mode = ctx->current_file->Is64() ? ZYDIS_MACHINE_MODE_LONG_64 : ZYDIS_MACHINE_MODE_LEGACY_32;

		IMAGE_NT_HEADERS *nt_header = ctx->current_file->GetPEHeader();
		IMAGE_SECTION_HEADER *section = ctx->current_file->GetSectionByRVA(ctx->current_file->GetRVAEntryPoint());

		std::size_t runtime_address = nt_header->OptionalHeader.ImageBase + ctx->current_file->GetRVAEntryPoint();
		std::size_t buffer_size = ctx->current_file->Size();

		std::uint8_t *buffer = &ctx->current_file->Buffer()[ctx->current_file->GetRAWEntryPoint()];

		type::VirtualBlock *current_block = type::NewBasicBlock(type::EntryBlock, buffer, runtime_address);

		type::ControlFlowGraph *graph = type::NewControlFlowGraph(current_block);

		type::PatternContext *pattern_ctx = type::NewPatternContext(ctx, current_block);

		std::queue<type::VirtualBlock *> list_block;
		std::queue<type::VirtualEdge> list_edge;

		ZydisDisassembledInstruction current_instruction{};

		while (ZYAN_SUCCESS(ZydisDisassembleIntel(mode, runtime_address, buffer, buffer_size, &current_instruction)))
		{
#ifdef DEBUG_SECTION
			if (runtime_address >= (section->VirtualAddress + section->Misc.VirtualSize))
			{
				break;
			}
#endif

			switch (current_instruction.info.meta.category)
			{
			case ZYDIS_CATEGORY_RET:
			case ZYDIS_CATEGORY_UNCOND_BR:
			case ZYDIS_CATEGORY_COND_BR:
			{
				is_read = false;
			}
			}

			if (current_instruction.info.mnemonic != ZYDIS_MNEMONIC_NOP)
			{

#ifdef SHOW_INSTRUCTION
				LOG_DEBUG("{}", current_instruction.text);
#endif
				type::VirtualInstruction virtual_instruction = type::NewInstruction(current_instruction, buffer);
				current_block->Insert(type::NewInstruction(current_instruction, buffer));

				if (pattern::Go(virtual_instruction, pattern_ctx))
				{
					is_read = true;

					buffer = current_block->PhysicalAddress();
					runtime_address = current_block->RuntimeAddress();

					current_block->Clear();

					continue;
				}
			}

			if (is_read)
			{
				runtime_address += current_instruction.info.length;
				buffer += current_instruction.info.length;

				continue;
			}

			if (!is_read)
			{
				if (current_instruction.info.meta.category != ZYDIS_CATEGORY_RET)
				{
					type::VirtualInstruction last_instruction = current_block->LastInstruction();
					if (!last_instruction.IsCatagory(ZYDIS_CATEGORY_COND_BR) &&
						!last_instruction.IsCatagory(ZYDIS_CATEGORY_UNCOND_BR))
					{
						is_read = true;

						buffer = current_block->PhysicalAddress();
						runtime_address = current_block->RuntimeAddress();

						current_block->Clear();

						continue;
					}

					if (current_instruction.info.meta.category == ZYDIS_CATEGORY_COND_BR)
					{
						std::size_t runtime_address_next = runtime_address + current_instruction.info.length;
						std::uint8_t *buffer_next = buffer + current_instruction.info.length;

						if (!graph->Exists(runtime_address_next))
						{
							list_block.push(type::NewBasicBlock(type::DefaultBlock, buffer_next, runtime_address_next));

							list_edge.push(type::NewEdge(type::NegativeEdge, current_block->RuntimeAddress(),
														 runtime_address_next));
						}
					}

					// OPERAND_ADDRESS
					std::size_t address_to = (current_instruction.operands->imm.value.u +
											  current_instruction.runtime_address + current_instruction.info.length) -
											 nt_header->OptionalHeader.ImageBase;

					std::uint8_t *buffer_to = &ctx->current_file->Buffer()[ctx->current_file->GetRAWValue(address_to)];

					if (!graph->Exists(address_to))
					{
						list_block.push(type::NewBasicBlock(type::DefaultBlock, buffer_to,
															address_to + nt_header->OptionalHeader.ImageBase));

						list_edge.push(type::NewEdge(type::PositiveEdge, current_block->RuntimeAddress(),
													 address_to + +nt_header->OptionalHeader.ImageBase));
					}
				}

				graph->InsertNode(current_block);

				if (!list_block.empty())
				{

#ifdef SHOW_INSTRUCTION
					LOG_DEBUG("---- End analyz block ------- \n\n\n\n\n");
#endif

					current_block = list_block.front();
					list_block.pop();

					buffer = current_block->PhysicalAddress();
					runtime_address = current_block->RuntimeAddress();

					is_read = true;

					pattern_ctx->SetVirtualBlock(current_block);

#ifdef SHOW_INSTRUCTION
					LOG_DEBUG("---- Start analyz block ------- \n\n\n\n\n");
#endif

					continue;
				}

				LOG_DEBUG("Successfull scaning binary", "");

				break;
			}
		}

		// INIT EDGES
		while (!list_edge.empty())
		{
			type::VirtualEdge edge = list_edge.front();
			list_edge.pop();

			graph->InsertEdge(edge);
		}

		return graph;
	}
} // namespace analyz
