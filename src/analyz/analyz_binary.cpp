#include "analyz.h"

namespace analyz
{

	type::VirtualBlock *AnalyzBlock(type::PatternContext *ctx, ZydisMachineMode mode, type::VirtualBlock *block)
	{
		bool is_read = true;

		std::size_t runtime_address = block->RuntimeAddress();
		std::uint8_t *physical_address = block->PhysicalAddress();

		ZydisDisassembledInstruction current_instruction{};

		while (ZYAN_SUCCESS(ZydisDisassembleIntel(mode, runtime_address, physical_address, ZYDIS_MAX_INSTRUCTION_LENGTH,
												  &current_instruction)))
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
				type::VirtualInstruction virtual_instruction =
					type::NewInstruction(current_instruction, physical_address);
				block->Insert(type::NewInstruction(current_instruction, physical_address));

				if (pattern::Go(virtual_instruction, ctx))
				{
					is_read = true;

					physical_address = block->PhysicalAddress();
					runtime_address = block->RuntimeAddress();

					block->Clear();

					continue;
				}
			}

			if (is_read)
			{
				runtime_address += current_instruction.info.length;
				physical_address += current_instruction.info.length;

				continue;
			}
			break;
		}

		return block;
	}

	type::ControlFlowGraph *AnalyzBinary(type::Context *ctx)
	{
		ZydisMachineMode mode = ctx->current_file->Is64() ? ZYDIS_MACHINE_MODE_LONG_64 : ZYDIS_MACHINE_MODE_LEGACY_32;

		IMAGE_NT_HEADERS *nt_header = ctx->current_file->GetPEHeader();
		IMAGE_SECTION_HEADER *section = ctx->current_file->GetSectionByRVA(ctx->current_file->GetRVAEntryPoint());

		std::size_t runtime_address = nt_header->OptionalHeader.ImageBase + ctx->current_file->GetRVAEntryPoint();

		std::uint8_t *buffer = &ctx->current_file->Buffer()[ctx->current_file->GetRAWEntryPoint()];

		type::VirtualBlock *current_block = type::NewBasicBlock(type::EntryBlock, buffer, runtime_address);

		type::ControlFlowGraph *graph = type::NewControlFlowGraph(current_block);

		type::PatternContext *pattern_ctx = type::NewPatternContext(ctx, current_block);

		std::queue<type::VirtualBlock *> list_block;
		std::queue<type::VirtualEdge> list_edge;
		tsl::robin_set<std::size_t> work_block;

		while (true)
		{
			current_block = AnalyzBlock(pattern_ctx, mode, current_block);

			if (!current_block->Empty())
			{
				type::VirtualInstruction last_instruction = current_block->LastInstruction();
				ZydisDisassembledInstruction current_instruction = last_instruction.Get();

				runtime_address = last_instruction.RuntimeAddress();
				buffer = last_instruction.PhysicalAddress();

				if (!current_block->IsPacked())
				{

					if (!last_instruction.IsCatagory(ZYDIS_CATEGORY_RET))
					{
						if (last_instruction.IsCatagory(ZYDIS_CATEGORY_COND_BR))
						{
							std::size_t runtime_address_next = runtime_address + last_instruction.Size();
							std::uint8_t *buffer_next = buffer + last_instruction.Size();

							if (!graph->Exists(runtime_address_next) &&
								(work_block.find(runtime_address_next) == work_block.end()))
							{
								list_block.push(
									type::NewBasicBlock(type::DefaultBlock, buffer_next, runtime_address_next));

								list_edge.push(type::NewEdge(type::NegativeEdge, current_block->RuntimeAddress(),
															 runtime_address_next));

								work_block.insert(runtime_address_next);
							}
						}

						std::size_t address_to =
							(current_instruction.operands->imm.value.u + current_instruction.runtime_address +
							 current_instruction.info.length) -
							nt_header->OptionalHeader.ImageBase;

						std::uint8_t *buffer_to =
							&ctx->current_file->Buffer()[ctx->current_file->GetRAWValue(address_to)];

						if (!graph->Exists(address_to) &&
							(work_block.find(address_to + nt_header->OptionalHeader.ImageBase) == work_block.end()))
						{
							list_block.push(type::NewBasicBlock(type::DefaultBlock, buffer_to,
																address_to + nt_header->OptionalHeader.ImageBase));

							list_edge.push(type::NewEdge(type::PositiveEdge, current_block->RuntimeAddress(),
														 address_to + +nt_header->OptionalHeader.ImageBase));

							work_block.insert(address_to + nt_header->OptionalHeader.ImageBase);
						}
					}

					graph->InsertNode(current_block);
				}
				else
				{
					LOG_WARN("Block is packed");
				}
			}

#ifdef SHOW_INSTRUCTION
			LOG_DEBUG("---- End analyz block ------- \n\n\n\n\n");
#endif

			if (!list_block.empty())
			{

				current_block = list_block.front();
				list_block.pop();

				buffer = current_block->PhysicalAddress();
				runtime_address = current_block->RuntimeAddress();

				pattern_ctx->SetVirtualBlock(current_block);

#ifdef SHOW_INSTRUCTION
				LOG_DEBUG("---- Start analyz block ------- \n\n\n\n\n");
#endif

				continue;
			}

			LOG_DEBUG("Successfull scaning binary", "");

			break;
		}
		return graph;
	}
} // namespace analyz
