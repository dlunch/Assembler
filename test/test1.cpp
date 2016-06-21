#include <iostream>

#include "../Assembler.h"

#include <iomanip>

int main()
{
	int i = 1;
	Assembler<64> a(0);
	a.push(_[rbp + 4]);
	a.mov(rbp, rsp);
	a.mov(rcx, 1000);
	a.push(1234);
	a.mov(_[1234], rax);
	a.call(0x12341234);
	a.mov(_[i], rax);
	a.mov(rcx, i);
	a.ret(0x12);
	
	std::vector<uint8_t> temp = a.finalize();
	
	for(auto i : temp)
	{
		std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)i << " ";
	}

	std::cout << "\n";

	Assembler<32> b(0);
	b.push(_[ebp + 4]);
	b.mov(ebp, esp);
	b.mov(ecx, 1000);
	b.push(1234);
	b.mov(_[1234], eax);
	b.call(0x12341234);
	b.mov(_[i], eax);
	b.mov(ecx, i);
	b.ret(0x12);

	temp = b.finalize();

	for(auto i : temp)
	{
		std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)i << " ";
	}
}