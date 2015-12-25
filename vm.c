#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned short u16;
u16 memory[32767 + 8]; // +8 for the registers

int OFFSET;

void op_halt(void);
void op_set(void);
void op_push(void);
void op_pop(void);
void op_eq(void);
void op_gt(void);
void op_jmp(void);
void op_jt(void);
void op_jf(void);
void op_add(void);
void op_mult(void);
void op_mod(void);
void op_and(void);
void op_or(void);
void op_not(void);
void op_rmem(void);
void op_wmem(void);
void op_call(void);
void op_ret(void);
void op_out(void);
void op_in(void);
void op_noop(void);

typedef struct node {
	int value;
	struct node *next;
} node;

node *head;

void stack_init(void) {
	head = (node *)malloc(sizeof(node));
	head->value = -1;
	head->next = NULL;
}

void stack_print(void) {
	node *cur = head;
	while (cur) {
		printf("%i, ", cur->value);
		cur = cur->next;
	}
	printf("\n");
}

void push(u16 value) {
	node *cur = head;
	while (cur->next != NULL)
		cur = cur->next;
	
	cur->next = (node *)malloc(sizeof(node));
	cur->next->value = value;
	cur->next->next = NULL;
}

u16 pop(void) {
	node *cur = head;
	node *prev;
	u16 r;

	while (cur->next != NULL) {
		prev = cur;
		cur = cur->next;
	}

	if (cur == head)
		return -1;

	r = cur->value;
	free(cur);
	prev->next = NULL;
	return r;
}

u16 getmem(u16 address) {
	return memory[address];
}

u16 getrarg(int n) {
	return getmem(OFFSET + n);
}

u16 getarg(int n) {
	u16 v = getrarg(n);
	if (v > 32767)
		return getmem(v);
	return v;
}

void setmem(u16 address, u16 value) {
	memory[address] = value;
}

void load_file_to_memory(FILE *binary) {
	int o = 0;
	u16 n;
	while (!feof(binary)) {
		fread(&n, sizeof(u16), 1, binary);
		setmem(o++, n);
	}
}

void (*runop(u16 opcode))(void) {
	switch (opcode) {
		case 0:
			return &op_halt;
		case 1:
			return &op_set;
		case 2:
			return &op_push;
		case 3:
			return &op_pop;
		case 4:
			return &op_eq;
		case 5:
			return &op_gt;
		case 6:
			return &op_jmp;
		case 7:
			return &op_jt;
		case 8:
			return &op_jf;
		case 9:
			return &op_add;
		case 10:
			return &op_mult;
		case 11:
			return &op_mod;
		case 12:
			return &op_and;
		case 13:
			return &op_or;
		case 14:
			return &op_not;
		case 15:
			return &op_rmem;
		case 16:
			return &op_wmem;
		case 17:
			return &op_call;
		case 18:
			return &op_ret;
		case 19:
			return &op_out;
		case 20:
			return &op_in;
		case 21:
		default:
			return &op_noop;
	}
}

void execute(void) {
	u16 opcode;

	for (OFFSET = 0;;) {
		opcode = getmem(OFFSET);
		runop(opcode)();
	}
}

void dumpmem(int s, int e) {
	int i;

	for (i = s; i <= s + e; i++) {
		printf("[%i] %u\n", i, getmem(i));
	}
}

int main(int argc, char *argv[]) {
	FILE *fp;

	if (argc < 2) {
		printf("Usage: vm <binary>\n");
		return 1;
	}

	memset(memory, 0, sizeof(memory));

	fp = fopen(argv[1], "rb");
	load_file_to_memory(fp);
	fclose(fp);

	//dumpmem(592, 20);
	stack_init();
	execute();

	return 0;
}

void op_halt(void) {
	exit(0);
}

void op_set(void) {
	setmem(getrarg(1), getarg(2));
	OFFSET += 3;
}

void op_push(void) {
	push(getarg(1));
	OFFSET += 2;
}

void op_pop(void) {
	setmem(getrarg(1), pop());
	OFFSET += 2;
}

void op_eq(void) {
	setmem(getrarg(1), (getarg(2) == getarg(3)));
	OFFSET += 4;
}

void op_gt(void) {
	setmem(getrarg(1), (getarg(2) > getarg(3)));
	OFFSET += 4;
}

void op_jmp(void) {
	OFFSET = getarg(1);
}

void op_jt(void) {
	OFFSET = (getarg(1) ? getarg(2) : OFFSET + 3);
}

void op_jf(void) {
	OFFSET = (getarg(1) ? OFFSET + 3: getarg(2));
}

void op_add(void) {
	setmem(getrarg(1), (getarg(2) + getarg(3)) % 32768);
	OFFSET += 4;
}

void op_mult(void) {
	setmem(getrarg(1), (getarg(2) * getarg(3)) % 32768);
	OFFSET += 4;
}

void op_mod(void) {
	setmem(getrarg(1), getarg(2) % getarg(3));
	OFFSET += 4;
}

void op_and(void) {
	setmem(getrarg(1), (getarg(2) & getarg(3)));
	OFFSET += 4;
}

void op_or(void) {
	setmem(getrarg(1), (getarg(2) | getarg(3)));
	OFFSET += 4;
}

void op_not(void) {
	setmem(getrarg(1), getarg(2) ^ 0x7FFF);
	OFFSET += 3;
}

void op_rmem(void) {
	setmem(getrarg(1), getmem(getarg(2)));
	OFFSET += 3;
}

void op_wmem(void) {
	setmem(getarg(1), getarg(2));
	OFFSET += 3;
}

void op_call(void) {
	push(OFFSET + 2);
	OFFSET = getarg(1);
}

void op_ret(void) {
	u16 nxop = pop();
	if (nxop < 0) op_halt();
	OFFSET = nxop;
}

void op_out(void) {
	putchar(getarg(1));
	OFFSET += 2;
}

void op_in(void) {
	setmem(getarg(1), getchar());
	OFFSET += 2;
}

void op_noop(void) {
	OFFSET += 1;
}