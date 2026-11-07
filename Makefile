
# Host-side unit tests (compiled with the Mac's cc, not the ARM toolchain)
CC     = cc
CFLAGS = -std=c11 -Wall -Wextra -O0 -g -Ifirmware/include -Ifirmware/src/control

.PHONY: test clean

test: build/test_pid ; ./build/test_pid

build/test_pid: tests/test_pid.c firmware/src/control/pid.c ; mkdir -p build && $(CC) $(CFLAGS) $^ -lm -o $@

clean: ; rm -rf build




