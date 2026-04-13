#include "analyz/analyz.h"
#include "context.h"
#include "type/virtual_block.h"
#include "utils/virtual_file.h"

#include <iostream>

int main()
{
	spdlog::set_level(spdlog::level::debug);
	// spdlog::set_default_logger(spdlog::basic_logger_mt("basic_logger", "log.txt"));

	util::file::VirtualFileBuilder current_file(FILE_NAME);

	LOG_INFO("File size -> {}", current_file.Size());
	LOG_INFO("Sections size -> {}", current_file.Sections().size());

	type::Context *context = type::NewContext(&current_file);
	{
		type::ControlFlowGraph *graph = analyz::AnalyzBinary(context);

		util::file::MapNewFile(&current_file, "deobuf_object.exe");

		LOG_INFO("Graph size -> {}", graph->Size());
	}
}
