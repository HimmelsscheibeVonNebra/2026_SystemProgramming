CC = cc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
.PHONY: all test test-student clean
all: file_copy read_chunks test_write_all
file_copy: file_copy.c io_helpers.h
	$(CC) $(CFLAGS) file_copy.c -o $@
read_chunks: read_chunks.c io_helpers.h
	$(CC) $(CFLAGS) read_chunks.c -o $@
test_write_all: test_write_all.c io_helpers.h
	$(CC) $(CFLAGS) test_write_all.c -o $@
file_copy_student: file_copy.c student/io_helpers.h
	$(CC) $(CFLAGS) -DSTUDENT file_copy.c -o $@
test_write_all_student: test_write_all.c student/io_helpers.h
	$(CC) $(CFLAGS) -DSTUDENT test_write_all.c -o $@
test: all
	./test_write_all
	python3 test_copy.py
test-student: test_write_all_student
	./test_write_all_student
clean:
	rm -f file_copy read_chunks test_write_all file_copy_student test_write_all_student
