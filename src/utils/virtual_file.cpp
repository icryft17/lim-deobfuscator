#include "virtual_file.h"
//
//

namespace util
{
	std::string GetRandomSystemFile()
	{
		std::vector<std::string> files;
		WIN32_FIND_DATAA data{};
		HANDLE first_file = FindFirstFileA("C:\\Windows\\System32\\*", &data);
		do
		{
			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				continue;
			}

			if (strstr((const char *)data.cFileName, ".exe"))
			{
				files.push_back((const char *)data.cFileName);
			}
		} while (FindNextFileA(first_file, &data));

		return std::string("C:\\Windows\\System32\\") + files[rand() % (files.size() - 1)];
	}
	namespace file
	{
		VirtualSection::VirtualSection(std::string name, std::size_t virtual_size)
			: name(name), virtual_size(virtual_size)
		{
		}

		VirtualSection NewSection(std::string name, std::size_t virtual_size)
		{
			return VirtualSection(name, virtual_size);
		}
		VirtualFileBuilder::VirtualFileBuilder(const std::uint8_t *buffer)
		{
			IMAGE_DOS_HEADER *dos_header = (IMAGE_DOS_HEADER *)(buffer);
			IMAGE_NT_HEADERS *pe_header = (IMAGE_NT_HEADERS *)(buffer + dos_header->e_lfanew);

			this->size_ = pe_header->OptionalHeader.SizeOfImage;
			this->capacity_size_ = BUFFER_CAPACITY;
			this->buffer_.resize(this->capacity_size_);

			std::memcpy(buffer_.data(), buffer, this->size_);

			IMAGE_NT_HEADERS *header = GetPEHeader();
			IMAGE_SECTION_HEADER *current_section =
				reinterpret_cast<IMAGE_SECTION_HEADER *>((std::uint8_t *)header + sizeof(IMAGE_NT_HEADERS));
			for (std::size_t i = 0; i < header->FileHeader.NumberOfSections; i++)
			{
				section_list_.push_back(current_section);
				current_section++;
			}
		}

		VirtualFileBuilder::VirtualFileBuilder(const std::string_view name, bool is_padding)
		{
			OFSTRUCT ofstruct{};
			this->handle_ = reinterpret_cast<HANDLE>(OpenFile(name.data(), &ofstruct, OF_READ));
			this->size_ = GetFileSize(this->handle_, NULL);
			this->capacity_size_ = BUFFER_CAPACITY;
			this->buffer_.resize(this->capacity_size_);

			if (!ReadFile(this->handle_, this->buffer_.data(), this->size_, NULL, NULL))
			{
				return;
			}

			IMAGE_NT_HEADERS *header = GetPEHeader();
			IMAGE_DOS_HEADER *dos_header = GetDosHeader();

			if (is_padding)
			{
				{
					std::size_t padding = ALIGNUP(250, header->OptionalHeader.FileAlignment);

					std::uint8_t *buffer = (std::uint8_t *)malloc(Size() + padding);

					std::memcpy(buffer, &Buffer()[dos_header->e_lfanew], Size());

					std::memset(&Buffer()[dos_header->e_lfanew], 0x0, padding);
					std::memcpy(&Buffer()[dos_header->e_lfanew + padding], buffer, Size());

					dos_header->e_lfanew += padding;
					header = GetPEHeader();

					IMAGE_SECTION_HEADER *current_section =
						reinterpret_cast<IMAGE_SECTION_HEADER *>((std::uint8_t *)header + sizeof(IMAGE_NT_HEADERS));
					for (std::size_t i = 0; i < header->FileHeader.NumberOfSections; i++)
					{

						current_section->PointerToRawData += padding;

						section_list_.push_back(current_section);

						current_section++;
					}
					size_ += padding;

					free(buffer);
				}
				IMAGE_SECTION_HEADER *last_section = section_list_[section_list_.size() - 1];

				std::size_t padding_sections =
					ALIGNUP(MAX_SIZE_SECTIONS - section_list_.size(), header->OptionalHeader.FileAlignment);

				std::uint8_t *buffer = (std::uint8_t *)malloc(size_ + padding_sections);

				std::memcpy(buffer, (last_section + 1), size_);
				std::memset((last_section + 1), 0x0, size_);

				std::uint8_t *dst = ((std::uint8_t *)(last_section + 1)) + padding_sections;

				std::memcpy(dst, buffer, size_);
				for (IMAGE_SECTION_HEADER *section : section_list_)
				{
					section->PointerToRawData += padding_sections;
				}
				size_ += padding_sections;

				free(buffer);
			}
			else
			{

				IMAGE_SECTION_HEADER *current_section =
					reinterpret_cast<IMAGE_SECTION_HEADER *>((std::uint8_t *)header + sizeof(IMAGE_NT_HEADERS));
				for (std::size_t i = 0; i < header->FileHeader.NumberOfSections; i++)
				{
					section_list_.push_back(current_section);
					current_section++;
				}
			}

			IMAGE_DATA_DIRECTORY import_directory = header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
			if (import_directory.VirtualAddress && import_directory.Size)
			{
				IMAGE_IMPORT_DESCRIPTOR *import_descriptor =
					(IMAGE_IMPORT_DESCRIPTOR *)&buffer_[GetRAWValue(import_directory.VirtualAddress)];
				while (import_descriptor->FirstThunk != 0 || import_descriptor->OriginalFirstThunk != 0)
				{

					Import current_import{};

					current_import.descriptor = import_descriptor;
					current_import.name = (const char *)&buffer_[GetRAWValue(import_descriptor->Name)];

					IMAGE_THUNK_DATA *current_import_function =
						(IMAGE_THUNK_DATA *)&buffer_[GetRAWValue(import_descriptor->OriginalFirstThunk)];
					while (current_import_function->u1.AddressOfData != 0)
					{
						current_import.original_functions.push_back(current_import_function);
						current_import_function++;
					}

					current_import_function = (IMAGE_THUNK_DATA *)&buffer_[GetRAWValue(import_descriptor->FirstThunk)];

					while (current_import_function->u1.AddressOfData != 0)
					{
						current_import.first_functions.push_back(current_import_function);

						current_import_function++;
					}
					imports_.push_back(current_import);

					import_descriptor++;

					import_directory.Size -= sizeof(IMAGE_IMPORT_DESCRIPTOR);
				}
			}
		}

		std::vector<Import> VirtualFileBuilder::GetImports() { return imports_; }

		VirtualFileBuilder::~VirtualFileBuilder()
		{
			if (handle_ != nullptr)
			{
				CloseHandle(handle_);
				handle_ = nullptr;
			}
		}
		std::size_t VirtualFileBuilder::Size() noexcept { return size_; }

		IMAGE_SECTION_HEADER *VirtualFileBuilder::GetSectionByName(std::string_view name)
		{
			for (IMAGE_SECTION_HEADER *section : section_list_)
			{
				if (std::strstr(name.data(), (const char *)section->Name))
				{
					return section;
				}
			}
			return nullptr;
		}

		IMAGE_SECTION_HEADER *VirtualFileBuilder::GetSectionByRVA(std::size_t rva)
		{
			for (IMAGE_SECTION_HEADER *section : section_list_)
			{
				if (rva >= section->VirtualAddress && rva < (section->VirtualAddress + section->Misc.VirtualSize))
				{
					return section;
				}
			}
			return nullptr;
		}

		std::size_t VirtualFileBuilder::GetRAWValue(const std::size_t value)
		{
			for (IMAGE_SECTION_HEADER *section : section_list_)
			{
				if (value >= section->VirtualAddress && value < (section->VirtualAddress + section->Misc.VirtualSize))
				{
					return (section->PointerToRawData + (value - section->VirtualAddress));
				}
			}
			return FILE_ERROR;
		}

		std::size_t VirtualFileBuilder::GetRVAEntryPoint() { return GetPEHeader()->OptionalHeader.AddressOfEntryPoint; }

		std::size_t VirtualFileBuilder::GetRAWEntryPoint() { return GetRAWValue(GetRVAEntryPoint()); }

		IMAGE_DOS_HEADER *VirtualFileBuilder::GetDosHeader()
		{
			return reinterpret_cast<IMAGE_DOS_HEADER *>(buffer_.data());
		}

		IMAGE_NT_HEADERS *VirtualFileBuilder::GetPEHeader()
		{
			return reinterpret_cast<IMAGE_NT_HEADERS *>((buffer_.data() + GetDosHeader()->e_lfanew));
		}
		void VirtualFileBuilder::SetSize(std::size_t size)
		{
			if (!size)
				return;
			if (size > buffer_.capacity())
				buffer_.resize(size * 2);
			size_ = size;
		}

		bool VirtualFileBuilder::IsPacked()
		{
			if (Empty())
				return false;

			std::unordered_map<std::uint8_t, std::size_t> bytes_map;
			for (std::size_t i = 0; i < size_; i++)
				bytes_map[buffer_[i]]++;

			std::float_t entropy = 0;
			for (auto &v : bytes_map)
			{
				std::float_t x = (std::float_t)v.first / size_;
				if (x > 0)
				{
					entropy -= x * log2f(x);
				}
			}
			entropy /= 8.0f;

			return entropy >= 7;
		}

		std::uint8_t *VirtualFileBuilder::Buffer() { return buffer_.data(); }

		bool VirtualFileBuilder::Is64() { return GetPEHeader()->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64; }

		std::vector<std::uint8_t> &VirtualFileBuilder::BufferVector() { return buffer_; }

		std::vector<IMAGE_SECTION_HEADER *> &VirtualFileBuilder::Sections() { return section_list_; }

		IMAGE_SECTION_HEADER *VirtualFileBuilder::AppendSection(VirtualSection virtual_section)
		{
			if (virtual_section.name.size() > IMAGE_SIZEOF_SHORT_NAME)
			{
				return {};
			}

			IMAGE_NT_HEADERS *header = this->GetPEHeader();
			IMAGE_SECTION_HEADER *first_section =
				(IMAGE_SECTION_HEADER *)((std::uint8_t *)header + sizeof(IMAGE_NT_HEADERS));
			IMAGE_SECTION_HEADER *last_section = first_section + header->FileHeader.NumberOfSections - 1;

			IMAGE_SECTION_HEADER *section = first_section + header->FileHeader.NumberOfSections;

			memcpy(section->Name, virtual_section.name.data(), virtual_section.name.size() + 1);

			section->Misc.VirtualSize = virtual_section.virtual_size;
			section->PointerToRawData = size_;
			section->VirtualAddress = ALIGNUP(last_section->VirtualAddress + last_section->Misc.VirtualSize,
											  header->OptionalHeader.SectionAlignment);
			section->SizeOfRawData = ALIGNUP(virtual_section.virtual_size, header->OptionalHeader.FileAlignment);
			section->Characteristics = IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE |
									   IMAGE_SCN_CNT_CODE | IMAGE_SCN_CNT_INITIALIZED_DATA;
			*(last_section + 1) = *section;

			header->FileHeader.NumberOfSections = header->FileHeader.NumberOfSections + 1;
			header->OptionalHeader.SizeOfImage =
				ALIGNUP((section->VirtualAddress + section->Misc.VirtualSize), header->OptionalHeader.SectionAlignment);

			size_ = section->PointerToRawData + section->SizeOfRawData;

			section_list_.push_back(section);

			return section;
		}

		bool MapNewFile(VirtualFileBuilder *file, const std::string_view name)
		{
			if (file->Empty())
			{
				return false;
			}
			OFSTRUCT ofstruct{};

			HANDLE handle = (HANDLE)OpenFile(name.data(), &ofstruct, OF_CREATE);

			const bool isWrite = WriteFile(handle, file->Buffer(), file->Size(), NULL, NULL);

			CloseHandle(handle);

			return isWrite;
		}
	} // namespace file
} // namespace util
