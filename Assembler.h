#pragma once

#include <cstdint>
#include <vector>

//some codes are copied from https://github.com/mattbierner/Template-Assembly (MIT)

template<int s, int i>
struct GPR
{
	static constexpr int size = s;
	static constexpr int index = i;
};

constexpr GPR<1, 0> al{};
constexpr GPR<1, 1> cl{};
constexpr GPR<1, 2> dl{};
constexpr GPR<1, 3> bl{};
constexpr GPR<1, 4> ah{};
constexpr GPR<1, 5> ch{};
constexpr GPR<1, 6> dh{};
constexpr GPR<1, 7> bh{};

constexpr GPR<2, 0> ax{};
constexpr GPR<2, 1> cx{};
constexpr GPR<2, 2> dx{};
constexpr GPR<2, 3> bx{};
constexpr GPR<2, 4> sp{};
constexpr GPR<2, 5> bp{};
constexpr GPR<2, 6> si{};
constexpr GPR<2, 7> di{};

constexpr GPR<4, 0> eax{};
constexpr GPR<4, 1> ecx{};
constexpr GPR<4, 2> edx{};
constexpr GPR<4, 3> ebx{};
constexpr GPR<4, 4> esp{};
constexpr GPR<4, 5> ebp{};
constexpr GPR<4, 6> esi{};
constexpr GPR<4, 7> edi{};

constexpr GPR<8, 0> rax{};
constexpr GPR<8, 1> rcx{};
constexpr GPR<8, 2> rdx{};
constexpr GPR<8, 3> rbx{};
constexpr GPR<8, 4> rsp{};
constexpr GPR<8, 5> rbp{};
constexpr GPR<8, 6> rsi{};
constexpr GPR<8, 7> rdi{};
constexpr auto r0 = rax;
constexpr auto r1 = rcx;
constexpr auto r2 = rdx;
constexpr auto r3 = rbx;
constexpr auto r4 = rsp;
constexpr auto r5 = rbp;
constexpr auto r6 = rsi;
constexpr auto r7 = rdi;
constexpr GPR<8, 8> r8{};
constexpr GPR<8, 9> r9{};
constexpr GPR<8, 10> r10{};
constexpr GPR<8, 11> r11{};
constexpr GPR<8, 12> r12{};
constexpr GPR<8, 13> r13{};
constexpr GPR<8, 14> r14{};
constexpr GPR<8, 15> r15{};

template<int size, typename reg1, typename reg2>
struct Memory
{
	const int displacement;
	const int scale;
	Memory(const int displacement_, const int scale_) : displacement(displacement_), scale(scale_) {}
};

template<int s, int i>
auto operator +(const GPR<s, i> &r, int displacement)
{
	return Memory<s, decltype(r), std::nullptr_t>{displacement, 0};
}

constexpr struct
{
	template<int size, typename reg1, typename reg2>
	constexpr auto operator[](const Memory<size, reg1, reg2> &mem) const
	{
		return mem;
	}

	template<int i, int s>
	constexpr auto operator[](const GPR<i, s> &) const
	{
		return Memory<0, GPR<i, s>, std::nullptr_t>{0, 0};
	}
	
	auto operator [](int displacement) const
	{
		return Memory<0, std::nullptr_t, std::nullptr_t>{displacement, 0};
	}
} _ {};

template<bool w, bool r, bool x, bool b>
struct EncodeREX
{
	static const uint8_t value = 0x40 | (8 * static_cast<int>(w)) | (4 * static_cast<int>(r)) | (2 * static_cast<int>(x)) | static_cast<int>(b);
};

template<int bit>
class Assembler
{
private:
	std::vector<uint8_t> buffer_;
	size_t baseAddress_;
	size_t currentAddress_;

	template<typename T>
	void insertBuffer(const T &value)
	{
		const uint8_t *temp = reinterpret_cast<const uint8_t *>(&value);
		buffer_.insert(buffer_.end(), temp, temp + sizeof(T));
	}
	
	template<typename T>
	void insertOpImm(uint8_t opcode, const T &value)
	{
		buffer_.push_back(opcode);
		insertBuffer(value);

		currentAddress_ += 1 + sizeof(T);
	}

	template<typename T>
	void insertOpImm(uint8_t opcode1, uint8_t opcode2, const T &value)
	{
		buffer_.push_back(opcode1);
		buffer_.push_back(opcode2);
		insertBuffer(value);

		currentAddress_ += 2 + sizeof(T);
	}

	template<typename T>
	void insertOpImm(uint8_t opcode1, uint8_t opcode2, uint8_t opcode3, uint8_t opcode4, const T &value)
	{
		buffer_.insert(buffer_.end(), {opcode1, opcode2, opcode3, opcode4});
		insertBuffer(value);

		currentAddress_ += 4 + sizeof(T);
	}
	
	void insertOp(uint8_t opcode)
	{
		buffer_.push_back(opcode);
		
		currentAddress_ += 1;
	}
	
	void insertOp(uint8_t opcode1, uint8_t opcode2)
	{
		buffer_.push_back(opcode1);
		buffer_.push_back(opcode2);
		
		currentAddress_ += 2;
	}

	void insertOp(uint8_t opcode1, uint8_t opcode2, uint8_t opcode3)
	{
		buffer_.push_back(opcode1);
		buffer_.push_back(opcode2);
		buffer_.push_back(opcode3);

		currentAddress_ += 3;
	}
public:
	Assembler(size_t baseAddress) : baseAddress_(baseAddress), currentAddress_(baseAddress) {}
	~Assembler() {}

	std::vector<uint8_t> &&finalize()
	{
		return std::move(buffer_);
	}

	Assembler &push(uint32_t operand)
	{
		insertOpImm(0x68, operand);

		return *this;
	}

	template<typename = std::enable_if<bit == 32>::type>
	Assembler &call(uint32_t targetAbs)
	{
		uint32_t addr = static_cast<uint32_t>(currentAddress_ + 5);
		insertOpImm(0xe8, static_cast<int32_t>(targetAbs - addr));

		return *this;
	}

	template<typename = std::enable_if<bit == 64>::type>
	Assembler &call(uint64_t targetAbs)
	{
		size_t addr = currentAddress_ + 5;
		int64_t diff = static_cast<int64_t>(targetAbs - addr);

		if(diff > 2147483647 || diff < -2147483647)
		{
			mov(rax, targetAbs);
			insertOp(0xff, 0xd0); //call rax  TODO change to generic version
		}
		else
			insertOpImm(0xe8, static_cast<int32_t>(targetAbs - addr));

		return *this;
	}

	Assembler &jmp(uint32_t targetAbs)
	{
		int32_t diff = targetAbs - (static_cast<int32_t>(currentAddress_) + 2);
		if(diff < 128 && diff >= -128)
			insertOpImm(0xeb, static_cast<int8_t>(targetAbs - (currentAddress_ + 2)));
		else
			insertOpImm(0xe9, static_cast<int32_t>(targetAbs - (currentAddress_ + 5)));
		
		return *this;
	}

	Assembler &ret(uint16_t stack = 0)
	{
		if(stack)
			insertOpImm(0xc2, stack);
		else
			insertOp(0xc3);

		return *this;
	}
	
	template<int size, typename reg1, typename reg2>
	Assembler &push(const Memory<size, reg1, reg2>&);
	template<int size, typename reg1, typename reg2, int s, int i>
	Assembler &mov(const Memory<size, reg1, reg2> &, GPR<s, i> operand);
	template<int s1, int i1, int s2, int i2>
	Assembler &mov(GPR<s1, i1>, GPR<s2, i2>);

	template<int size>
	Assembler &mov(const Memory<size, std::nullptr_t, std::nullptr_t> &m, GPR<4, 0>) //mov rax, disp
	{
		insertOpImm(0xa3, m.displacement);
		
		return *this;
	}

	template<int size>
	Assembler &mov(const Memory<size, std::nullptr_t, std::nullptr_t> &m, GPR<8, 0>) //mov [disp], rax
	{
		insertOpImm(EncodeREX<1, 0, 0, 0>::value, 0x89, 0x04, 0x25, m.displacement);

		return *this;
	}

	template<int size, int i, int s>
	Assembler &mov(const Memory<size, GPR<s, i>, std::nullptr_t> &m, GPR<8, 0>) //mov [GPR], rax
	{
		insertOp(EncodeREX<1, 0, 0, 0>::value, 0x89, 0x00 + i);

		return *this;
	}

	template<int size, int i, int s, typename reg2>
	Assembler &push(const Memory<size, const GPR<s, i> &, reg2> &m)
	{
		if(bit == 64 && s == 4)
			insertOp(0x67);
		if(m.displacement < 128 && m.displacement >= -128)
			insertOpImm(0xff, 0x70 + i, (int8_t)m.displacement);
		else
			insertOpImm(0xff, 0xb0 + i, (int32_t)m.displacement);
		return *this;
	}
	
	template<int s, int i, typename T>
	Assembler &mov(const GPR<s, i> &reg, T operand)
	{
		static_assert(bit / 8 == s, "Error");

		if(bit == 64 && s == 8)
			insertOpImm(EncodeREX<1, 0, 0, 0>::value, 0xb8 + i, (uint64_t)operand);
		else if(bit == 32 && s == 4)
			insertOpImm(0xb8 + i, (uint32_t)operand);
			
		
		return *this;
	}
	
	template<int i, int s>
	Assembler &mov(GPR<s, i>, GPR<s, 4>) //TODO modrm..
	{
		if(s == 8 && bit == 64)
			insertOp(EncodeREX<1, 0, 0, 0>::value);
		insertOp(0x89, 0xe0 + i);
		
		return *this;
	}

	template<int i>
	Assembler &mov(GPR<8, i>, GPR<8, 8>) //TODO modrm..
	{
		insertOp(EncodeREX<1, 1, 0, 0>::value);
		insertOp(0x89, 0xc0 + i);

		return *this;
	}

	template<int s, int i>
	Assembler &sub(GPR<s, i>, uint8_t value)
	{
		if(s == 8 && bit == 64)
			insertOp(EncodeREX<1, 0, 0, 0>::value);
		insertOpImm(0x83, 0xe8 + i, value);

		return *this;
	}

	template<int s, int i>
	Assembler &add(GPR<s, i>, uint8_t value)
	{
		if(s == 8 && bit == 64)
			insertOp(EncodeREX<1, 0, 0, 0>::value);
		insertOpImm(0x83, 0xc0 + i, value);

		return *this;
	}
	
	template<typename = std::enable_if<bit == 32>::type>
	Assembler &insertData(uint32_t data)
	{
		insertBuffer(data);
		currentAddress_ += 4;

		return *this;
	}

	template<typename = std::enable_if<bit == 64>::type>
	Assembler &insertData(uint64_t data)
	{
		insertBuffer(data);
		currentAddress_ += 8;

		return *this;
	}

	Assembler &insertData(const std::initializer_list<uint8_t> &data)
	{
		buffer_.insert(buffer_.end(), data.begin(), data.end());

		currentAddress_ += data.size();
		return *this;
	}
};
