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

typedef struct _Ob{
	int first, mid, end, location;
	struct _Ob *list;
	int format, enter_flag;
	char var[255];
}Ob;

typedef struct _Sym{
	int location;
	char name[10];
	struct _Sym *list;
}Sym;

void symbol(Sym **symhead, int flag);
Sym *allocate(Sym *data);
int check_mnemonic(char *mnemonic, char *format, int *opcode, Op **ophead);
int build_symtab(Sym **symhead, Sym *node, int line_num);
int find_symbol(Sym **symhead, char *symbol);
void location_counter(char *format, unsigned int *location, char *mnemonic);
void objectcode_insert(int format, int first, int mid, int end, int location, Ob** head, Ob** tail, char *var, int enter_flag);
int reg_num(char *reg);
void type(char *origin, char **command);
void assemble(char *origin,char **command, Op **ophead, Sym ***output, int *symbol_print_flag);
void symbol();
void opcodelist();
void mnemonic(char *origin,char **command);
void create_opcode();
void insert(Op *node);
void check1(char *origin,char **command,Sym ***symhead, int *symbol_print_flag);
void check2(char *origin,char **command, int TOKEN_SIZE, Sym ***symhead, int *symbol_print_flag);
void quit();
void record(char *origin,char **command, int TOKEN_SIZE);
void help();
void history();
void dir();
void dump(unsigned int start, unsigned int end);
void edit(unsigned int origin, unsigned int new);
void fill(unsigned int start, unsigned int end, unsigned int new);
void reset();
