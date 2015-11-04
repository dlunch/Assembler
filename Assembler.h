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

constexpr auto al = GPR<1, 0>{};
constexpr auto cl = GPR<1, 1>{};
constexpr auto dl = GPR<1, 2>{};
constexpr auto bl = GPR<1, 3>{};
constexpr auto ah = GPR<1, 4>{};
constexpr auto ch = GPR<1, 5>{};
constexpr auto dh = GPR<1, 6>{};
constexpr auto bh = GPR<1, 7>{};

constexpr auto ax = GPR<2, 0>{};
constexpr auto cx = GPR<2, 1>{};
constexpr auto dx = GPR<2, 2>{};
constexpr auto bx = GPR<2, 3>{};
constexpr auto sp = GPR<2, 4>{};
constexpr auto bp = GPR<2, 5>{};
constexpr auto si = GPR<2, 6>{};
constexpr auto di = GPR<2, 7>{};

constexpr auto eax = GPR<4, 0>{};
constexpr auto ecx = GPR<4, 1>{};
constexpr auto edx = GPR<4, 2>{};
constexpr auto ebx = GPR<4, 3>{};
constexpr auto esp = GPR<4, 4>{};
constexpr auto ebp = GPR<4, 5>{};
constexpr auto esi = GPR<4, 6>{};
constexpr auto edi = GPR<4, 7>{};

constexpr auto rax = GPR<8, 0>{};
constexpr auto rcx = GPR<8, 1>{};
constexpr auto rdx = GPR<8, 2>{};
constexpr auto rbx = GPR<8, 3>{};
constexpr auto rsp = GPR<8, 4>{};
constexpr auto rbp = GPR<8, 5>{};
constexpr auto rsi = GPR<8, 6>{};
constexpr auto rdi = GPR<8, 7>{};
constexpr auto r0 = rax;
constexpr auto r1 = rcx;
constexpr auto r2 = rdx;
constexpr auto r3 = rbx;
constexpr auto r4 = rsp;
constexpr auto r5 = rbp;
constexpr auto r6 = rsi;
constexpr auto r7 = rdi;
constexpr auto r8 = GPR<8, 8>{};
constexpr auto r9 = GPR<8, 9>{};
constexpr auto r10 = GPR<8, 10>{};
constexpr auto r11 = GPR<8, 11>{};
constexpr auto r12 = GPR<8, 12>{};
constexpr auto r13 = GPR<8, 13>{};
constexpr auto r14 = GPR<8, 14>{};
constexpr auto r15 = GPR<8, 15>{};

template<int size, typename reg1, typename reg2>
struct Memory
{
	int displacement;
	int scale;
	Memory(int displacement_, int scale_) : displacement(displacement_) {}
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
	Assembler(size_t baseAddress);
	~Assembler();

	std::vector<uint8_t> &&finalize();

	Assembler &push(uint32_t operand);
	Assembler &call(uint32_t targetAbs);
	Assembler &ret(uint16_t stack);
	
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
	
	Assembler &insertData(uint32_t data);
	Assembler &insertData(const std::initializer_list<uint8_t> &data);
};
