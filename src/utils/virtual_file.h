#ifndef VIRTUAL_FILE_H
#define VIRTUAL_FILE_H

#define NOMINMAX

#include <Windows.h>
#include <cstddef>
#include <cstdint>
#include <string>

#include <unordered_map>

#define ALIGNDOWN(x, align) ((x) & -(align))
#define ALIGNUP(x, align) (((x) + (align) - 1) & ~((align) - 1))

#define CALLBACK_MAX_SIZE 0x20

#ifdef _WIN64
#define BUFFER_CAPACITY 1'000'000'000
#else
#define BUFFER_CAPACITY 5'000'000
#endif
#define TLS_SIZE_BLOCK sizeof(IMAGE_TLS_DIRECTORY) + (sizeof(std::size_t) * CALLBACK_MAX_SIZE) + sizeof(std::size_t)
#define RESERVE_SIZE 0x1000
/*
	CALLBAKC
*/
#define MAX_SIZE_SECTIONS 20
#define FILE_ERROR INT64_MAX - 1
namespace util
{
	std::string GetRandomSystemFile();

	namespace file
	{

		struct Import
		{
			IMAGE_IMPORT_DESCRIPTOR *descriptor{};

			std::string name;

			std::vector<IMAGE_THUNK_DATA *> first_functions;
			std::vector<IMAGE_THUNK_DATA *> original_functions;
		};

		struct VirtualSection
		{
			VirtualSection() = default;
			VirtualSection(std::string name, std::size_t virtual_size);
			std::string name;
			std::size_t virtual_size = 0;
		};

		VirtualSection NewSection(std::string name, std::size_t virtual_size);

		class VirtualFileBuilder
		{
		  public:
			VirtualFileBuilder(const std::string_view name, bool is_padding = false);

			VirtualFileBuilder(const std::uint8_t *buffer);

			~VirtualFileBuilder();

		  public:
			bool Is64();

			bool Empty() noexcept { return size_ == 0; }

			std::size_t Size() noexcept;

			std::size_t GetRVAEntryPoint();

			std::size_t GetRAWEntryPoint();

			std::size_t GetRAWValue(const std::size_t value);

			IMAGE_DOS_HEADER *GetDosHeader();

			IMAGE_NT_HEADERS *GetPEHeader();

			IMAGE_SECTION_HEADER *GetSectionByName(std::string_view name);

			IMAGE_SECTION_HEADER *GetSectionByRVA(std::size_t rva);

			bool IsPacked();

			std::uint8_t *Buffer();

			std::vector<IMAGE_SECTION_HEADER *> &Sections();

			std::vector<Import> GetImports();

			std::vector<std::uint8_t> &BufferVector();

			IMAGE_SECTION_HEADER *AppendSection(VirtualSection section);

			void SetSize(std::size_t size);

		  private:
			std::size_t size_ = 0x0;
			std::size_t capacity_size_ = 0x0;
			std::vector<std::uint8_t> buffer_;

			std::vector<Import> imports_;

			std::vector<IMAGE_SECTION_HEADER *> section_list_;
			HANDLE handle_ = nullptr;
		};
		bool MapNewFile(VirtualFileBuilder *file, const std::string_view name);
	} // namespace file
} // namespace util
#endif // ! VIRTUAL_FILE
