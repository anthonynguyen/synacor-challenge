#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint16_t memory[32767 + 8]; // +8 for the registers

int OFFSET;

typedef struct node {
	int value;
	struct node *next;
} node;

node *head;

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

void stack_init(void) {
	head = (node *)malloc(sizeof(node));
	head->value = -1;
	head->next = NULL;
}

void push(uint16_t value) {
	node *cur = head;
	while (cur->next != NULL)
		cur = cur->next;
	
	cur->next = (node *)malloc(sizeof(node));
	cur->next->value = value;
	cur->next->next = NULL;
}

uint16_t pop(void) {
	node *cur = head;
	node *prev;
	uint16_t r;

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

void setmem(uint16_t address, uint16_t value) {
	memory[address] = value;
}

uint16_t getmem(uint16_t address) {
	return memory[address];
}

uint16_t getrarg(int n) {
	return getmem(OFFSET + n);
}

uint16_t getarg(int n) {
	uint16_t v = getrarg(n);
	if (v > 32767)
		return getmem(v);
	return v;
}

void (*runop(uint16_t opcode))(void) {
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
	uint16_t opcode;

	for (OFFSET = 0;;) {
		opcode = getmem(OFFSET);
		runop(opcode)();
	}
}

int main(int argc, char *argv[]) {
	FILE *fp;
	int o = 0;
	uint16_t n;

	if (argc < 2) {
		printf("Usage: vm <binary>\n");
		return 1;
	}

	memset(memory, 0, sizeof(memory));

	fp = fopen(argv[1], "rb");
	while (!feof(fp)) {
		fread(&n, sizeof(uint16_t), 1, fp);
		setmem(o++, n);
	}

	fclose(fp);

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
	setmem(getrarg(1), getarg(2) ^ 32767);
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
	uint16_t nxop = pop();
	if (nxop < 0) op_halt();
	OFFSET = nxop;
}

void op_out(void) {
	putchar(getarg(1));
	OFFSET += 2;
}

void op_in(void) {
	setmem(getrarg(1), getchar());
	OFFSET += 2;
}

void op_noop(void) {
	OFFSET += 1;
}