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
	
	auto operator [](int displacement) const
	{
		return Memory<0, std::nullptr_t, std::nullptr_t>{displacement, 0};
	}
} _ {};


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

	Assembler &call(uint32_t targetAbs)
	{
		size_t addr = currentAddress_ + 5;
		insertOpImm(0xe8, static_cast<int32_t>(targetAbs - addr));

		return *this;
	}

	Assembler &ret(uint16_t stack)
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
	template<int s, int i>
	Assembler &mov(GPR<s, i>, uint32_t);
	template<int s1, int i1, int s2, int i2>
	Assembler &mov(GPR<s1, i1>, GPR<s2, i2>);

	template<int size>
	Assembler &mov(Memory<size, std::nullptr_t, std::nullptr_t> m, GPR<4, 0>)
	{
		insertOpImm(0xa3, m.displacement);
		
		return *this;
	}
	
	template<int size, int i, typename reg2>
	Assembler &push(const Memory<size, const GPR<4, i> &, reg2> &m)
	{
		insertOpImm(0xff, 0xb0 + i, (int32_t)m.displacement); //there's ff 70 for disp8, but using only 32bit makes code cleaner.. and we have a lot of memory..
		return *this;
	}
	
	template<int i>
	Assembler &mov(GPR<4, i> reg, uint32_t operand)
	{
		insertOpImm(0xb8 + i, operand);
		
		return *this;
	}
	
	template<int i1>
	Assembler &mov(GPR<4, i1>, GPR<4, 4>) //TODO modrm..
	{
		insertOp(0x89, 0xe0 + i1);
		
		return *this;
	}
	
	Assembler &insertData(uint32_t data)
	{
		insertBuffer(data);
		currentAddress_ += 4;

		return *this;
	}

	Assembler &insertData(const std::initializer_list<uint8_t> &data)
	{
		buffer_.insert(buffer_.end(), data.begin(), data.end());

		currentAddress_ += data.size();
		return *this;
	}
};
