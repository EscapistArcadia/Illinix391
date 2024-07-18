#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "term.h"
#include "rtc.h"
#include "filesys.h"
#include "syscall.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

int exp0_test() {
	TEST_HEADER;
	int a = 0xECE391, b = 0;
	a = a / b;
	return PASS;
}

int syscall_test() {
	TEST_HEADER;
	asm("int $0x80");
	return PASS;
}

int paging_test(uint8_t *addr) {
	TEST_HEADER;
	*addr = 0x78;
	return PASS;
}

/* Checkpoint 2 tests */
int terminal_test() {
	TEST_HEADER;
	uint8_t buf[128];
	int32_t count;
	while ((count = terminal_read(0, buf, 128)) != -1) {
		terminal_write(0, buf, count);	/* print what we got from user input */
	}
	return 0;
}


int rtc_test(char ch) {
	TEST_HEADER;
	rtc_open(0);

	int32_t freq, i, j, max_count = 10;
	char seq[RTC_MAX_RATE + 1] = {ch, '\0'};

	for (freq = RTC_MIN_FREQ, i = 1; freq <= RTC_MAX_FREQ; freq = (freq << 1), ++i, max_count = max_count << 1) {
		clear();
		seq[i] = ++ch;
		rtc_write(NULL, &freq, sizeof(int32_t));

		for (j = 0; j < max_count; ++j) {
			printf("Frequency: %d #%d ", freq, j);
			printf(seq);
			printf("\n");
			rtc_read(NULL, NULL, NULL);
		}
	}

	rtc_close(0);
	return PASS;
}

int file_system_test() {
	int ret;
	/* should return -1, since file name exceeds the maximum length */
	if ((ret = file_open((const uint8_t *)"verylargetextwithverylongname.txt")) != -1) {
		return FAIL;	
	}
	/* should return 0, since the file exists */
	if ((ret = file_open((const uint8_t *)"fish")) == -1) {
		return FAIL;
	}
	
	char content[65536];
	int size = file_read(ret, content, 65536);
	printf("%d %d\n", size, file_size(ret));			/* test file size inquery */
	terminal_write(0, (const uint8_t *)content, size);
	return PASS;
}

int list_file_test() {
	int fd = dir_open((const uint8_t *)".");
	char file_name[64];
	
	while (dir_read(fd, file_name, 64) != 0) {
		printf("Name: ");
		terminal_write(fd, file_name, FS_MAX_LEN);
		
		printf("Size: %d\n", file_size(file_open((const uint8_t *)file_name)));
	}
	return PASS;
}
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("exp0_test", exp0_test());
	// TEST_OUTPUT("syscall_test", syscall_test());
	// TEST_OUTPUT("paging_test", paging_test((uint8_t *)0x12345));
	// TEST_OUTPUT("paging_test", paging_test((uint8_t *)0xB8765));
	// TEST_OUTPUT("paging_test", paging_test((uint8_t *)0x456789));

	// TEST_OUTPUT("rtc_test", rtc_test('A'));
	// TEST_OUTPUT("file_system_test", file_system_test());
	// TEST_OUTPUT("list_file_test", list_file_test());
	// TEST_OUTPUT("terminal_test", terminal_test());
	
	execute((const uint8_t *)"               shell    ");

	// launch your tests here
}
