# Simple embeddable x86 assembler for C++

## Usage

```
Assembler a(0);
	a.push(_[ebp + 4]);
	a.mov(ebp, esp);
	a.mov(ecx, 1000);
	a.push(1234);
	a.mov(_[1234], eax);
	a.call(0x12341234);
	a.mov(_[i], eax);
	a.mov(ecx, i);
	a.ret(0x12);
	
	std::vector<uint8_t> temp = std::move(a.finalize());
```

You can write temp to somewhere and execute generated code. 
