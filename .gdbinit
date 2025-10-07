file zig-out/bin/kernel
target remote :1234
break _start
layout asm
continue