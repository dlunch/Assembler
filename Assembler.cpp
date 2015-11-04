#include "Assembler.h"

Assembler::Assembler(size_t baseAddress) : baseAddress_(baseAddress), currentAddress_(baseAddress)
{
	
}

Assembler::~Assembler()
{
	
}

std::vector<uint8_t> &&Assembler::finalize()
{
	return std::move(buffer_);
}


Assembler &Assembler::push(uint32_t operand)
{
	insertOpImm(0x68, operand);

	return *this;
}

Assembler &Assembler::call(uint32_t targetAbs)
{
	size_t addr = currentAddress_ + 5;
	insertOpImm(0xe8, static_cast<int32_t>(targetAbs - addr));

	return *this;
}

Assembler &Assembler::ret(uint16_t stack)
{
	if(stack)
		insertOpImm(0xc2, stack);
	else
		insertOp(0xc3);
	
	return *this;
}

Assembler &Assembler::insertData(uint32_t data)
{
	insertBuffer(data);
	currentAddress_ += 4;

	return *this;
}

Assembler &Assembler::insertData(const std::initializer_list<uint8_t> &data)
{
	buffer_.insert(buffer_.end(), data.begin(), data.end());

	currentAddress_ += data.size();
	return *this;
}