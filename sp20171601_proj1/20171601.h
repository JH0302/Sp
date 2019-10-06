#define INPUT_SIZE 500
#define MEMORY_SIZE 0x100000

typedef struct _Node {
	char a[INPUT_SIZE];
	struct _Node *list;
}Node;

typedef struct _Op {
	int num;
	char inst[10];
	char type[5];
	struct _Op *list;
}Op;

void opcodelist();
void mnemonic(char *origin,char **command);
void create_opcode();
void insert(Op *node);
void check1(char *origin,char **command);
void check2(char *origin,char **command, int TOKEN_SIZE);
void quit();
void record(char *origin,char **command, int TOKEN_SIZE);
void help();
void history();
void dir();
void dump(unsigned int start, unsigned int end);
void edit(unsigned int origin, unsigned int new);
void fill(unsigned int start, unsigned int end, unsigned int new);
void reset();
