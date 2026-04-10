#ifndef CONTEXT_H
#define CONTEXT_H

#include "type/virtual_block.h"
#include "utils/virtual_file.h"
#include "utils/encoder_decoder.h"

namespace type
{
	struct Context
	{
		util::file::VirtualFileBuilder *current_file = nullptr;
	};

	class PatternContext
	{
	  public:
		PatternContext() = default;
		PatternContext(Context *ctx, type::VirtualBlock *virtual_block);

	  public:
		void SetContext(Context *ctx) noexcept;
		void SetVirtualBlock(type::VirtualBlock *virtual_block) noexcept;

		Context *GetContext() noexcept;

		type::VirtualBlock *GetBlock() noexcept;
		// geters
	  private:
		Context *ctx = nullptr;
		type::VirtualBlock *virtual_block = nullptr;
	};

	Context *NewContext(util::file::VirtualFileBuilder *current_file);

	PatternContext *NewPatternContext(Context *ctx, type::VirtualBlock *virtual_block);
} // namespace type
#endif
