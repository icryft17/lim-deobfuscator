#ifndef UTIL_ENCODER_DECODER_H
#define UTIL_ENCODER_DECODER_H

namespace util
{
	type::VirtualInstruction DecodeInstruction(ZydisMachineMode mode, std::uint8_t *object,
											   std::size_t runtime_address);

	type::VirtualInstruction DecodeWithNotNOPInstruction(ZydisMachineMode mode, std::uint8_t *object,
														 std::size_t runtime_address);

	bool EncoderInstruction(ZydisMachineMode mode, std::uint8_t *object, type::VirtualInstruction instruction);

	void DeleteInstruction(type::VirtualBlock* current_block, type::VirtualInstruction instruction);

	// void EncoderInstruction();

} // namespace util

#endif
