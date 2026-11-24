CC      ?= gcc
CFLAGS  ?= -std=c99 -Wall -Wextra -Wconversion -Werror -O2

CASES = case-studies/01-j1939-decoder/j1939 \
        case-studies/02-e2e-protection/e2e \
        case-studies/03-bms-overvoltage/ov_monitor

build/test_drills: src/drills.c tests/test_drills.c include/drills.h
	@mkdir -p build
	$(CC) $(CFLAGS) -Iinclude src/drills.c tests/test_drills.c -o $@

test: build/test_drills
	./build/test_drills
	@echo "case studies"
	@set -e; for c in $(CASES); do \
		d=$$(dirname $$c); n=$$(basename $$c); \
		$(CC) $(CFLAGS) -I$$d $$d/$$n.c $$d/test_$$n.c -o build/test_$$n; \
		./build/test_$$n; \
	done

clean:
	rm -rf build

.PHONY: test clean
