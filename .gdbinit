target remote :1234
watch *(unsigned long long*)0x10000 == 0xDEADBEEF
continue
set $base = *(unsigned long long*)0x10008
add-symbol-file build/breakout.debug -o $base
