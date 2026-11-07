CC = cc
CFLAGS = -std=c11 -Wall -Wextra -O0 -g -Ifirmware/include -Ifirmware/src/control -Ifirmware/src/estimation

TESTS = build/test_pid build/test_odometry

test: $(TESTS)
	./build/test_pid
	./build/test_odometry

build/test_pid: tests/test_pid.c firmware/src/control/pid.c
	mkdir -p build
	$(CC) $(CFLAGS) $^ -lm -o $@

build/test_odometry: tests/test_odometry.c firmware/src/estimation/odometry.c
	mkdir -p build
	$(CC) $(CFLAGS) $^ -lm -o $@

clean:
	rm -rf build

.PHONY: test clean
