#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "20171601.h"

Node *head; // history record
Op *ophead[20]; // opcode record
unsigned char MEMORY[MEMORY_SIZE]; // Memory 지정
unsigned int address;	// dump 주소값

void create_opcode()	// opcode.txt를 읽어와 hash table 만들기
{
	FILE *fp = fopen("opcode.txt", "r"); // 파일 읽어오기
	int i;
	Op *node;

	if (!fp) printf("There's no Opcode!\n"); // 파일에 아무것도 없을 시

	node = (Op*)malloc(sizeof(Op));	// node에 메모리 할당

	for (i = 0; i<20; i++)
		ophead[i] = NULL;	// head를 NULL로 초기화

	while (!feof(fp))
	{
		fscanf(fp, "%x\t%s\t\t%s\n", &node->num, node->inst, node->type); // 파일 내용 읽기
		insert(node);	// linked list에 삽입
	}
	fclose(fp); //file close
}
void insert(Op *node) // hash table을 linked list 형태로 삽입
{
	int i, key, num, sum = 0;
	Op *new;

	num = strlen(node->inst);
	
	// hash function의 key값 생성
	for (i = 0; i < num; i++)
		sum += node->inst[i]; // 모든 아스키 값 더하기

	key = sum % 20;	// key값 정의

	//head에 아무 값도 없을 경우
	if (ophead[key] == NULL) 
	{
		ophead[key] = (Op *)malloc(sizeof(Op));

		ophead[key]->num = node->num;
		strcpy(ophead[key]->inst, node->inst);
		strcpy(ophead[key]->type, node->type);

		ophead[key]->list = NULL;
	}
	//이미 값이 존재하는 경우 : 중간에 원소 삽입
	else
	{
		new = (Op *)malloc(sizeof(Op));
		
		new->num = node->num;
		strcpy(new->inst, node->inst);
		strcpy(new->type, node->type);

		new->list = ophead[key] -> list;
		ophead[key]->list = new;
	}
}
void main()
{
	create_opcode(); // opcode 생성

	char command[INPUT_SIZE] = { "\0" }; // 입력받는 명령어
	char command_temp[INPUT_SIZE] = { "\0" }; // 명렁어를 나누기 위한 임의의 배열

	char **token; // 나눈 명령들 저장
	char *ptr; // 나눈 덩어리 저장을 위한 볏누
	Sym **symhead = NULL;
	int flag = 0; // flag = 1일 경우 종료
	int TOKEN_SIZE; // token 덩어리의 갯수
	int i, symbol_print_flag = 0;

	while (!flag)
	{
		TOKEN_SIZE = 1;
		printf("sicsim> ");

		fgets(command, sizeof(command), stdin);
		command[strlen(command) - 1] = '\0';
		strcpy(command_temp, command);

		//TOKEN의 갯수 세기
		ptr = strtok(command_temp, " \t\n");

		if (!ptr) continue;
		
		while (1)
		{
			ptr = strtok(NULL, " \t\n");
			if (!ptr) break;
			TOKEN_SIZE++;
		}
		token = (char **)malloc(sizeof(char *)*(TOKEN_SIZE + 1));

		//TOKEN 분리
		strcpy(command_temp, command);

		ptr = strtok(command_temp, " \t\n");

		if (!ptr) continue;

		token[0] = (char *)malloc(sizeof(char) * (INPUT_SIZE));
		strcpy(token[0], ptr);
		
		for (int i = 1; i < TOKEN_SIZE; i++) {
			ptr = strtok(NULL, " \t\n");
			if (ptr == NULL) break;
			token[i] = (char *)malloc(sizeof(char) * (INPUT_SIZE));
			strcpy(token[i], ptr);
		}

		if (strcmp(*token, "quit") == 0 || strcmp(*token, "q") == 0) // command가 quit일 경우
		{
			quit(); // 메모리 해제
			flag = 1; // 종료
		}
		else
		{
			if (TOKEN_SIZE == 1) // token의 갯수가 1개라면
				check1(command,token,&symhead,&symbol_print_flag); // check1() 함수로 이동
			else if (TOKEN_SIZE >= 2) // token의 갯수가 2개 이상이라면
				check2(command,token, TOKEN_SIZE, &symhead,&symbol_print_flag); // check2() 함수로 이동
		}

		//메모리 해제
		for (i = 0; i < TOKEN_SIZE; i++)
		{
			free(token[i]);
			token[i] = NULL;
		}
	}
}
void quit()
{
	Node *ptr1;
	Op *ptr2;
	int i;

	ptr1 = head;
	//History 메모리 해제
	while(head)
	{
		ptr1 = head;
		head = head->list;
		free(ptr1);
	}
	//Opcode 메모리 해제
	for(i = 0; i < 20; i++)
	{
		ptr2 = ophead[i];

		while(ophead[i])
		{
			ptr2 = ophead[i];
			ophead[i] = ophead[i]->list;
			free(ptr2);
		}
	}
}
//Token의 갯수가 1개인 경우
void check1(char *origin,char **command, Sym ***symhead, int *symbol_print_flag)
{
	unsigned int start, end = 0;

 	// command가 help인 경우
	if (strcmp(*command, "help") == 0 || strcmp(*command, "h") == 0)
	{
		record(origin,command, 1); // history에 기록
		help();
	}
	// command가 dir인 경우
	else if (strcmp(*command, "dir") == 0 || strcmp(*command, "d") == 0)
	{
		record(origin,command, 1);
		dir();
	}
	//command가 history인 경우
	else if (strcmp(*command, "history") == 0 || strcmp(*command, "hi") == 0)
	{
		record(origin,command, 1);
		history();
	}
	//dump : case 1(no argumennt)
	else if (strcmp(*command, "dump") == 0 || strcmp(*command, "du") == 0)
	{
		record(origin,command, 1);
		//(last address + 1)+159가 범위를 초과할 경우
		if ((address + 160) > 0xFFFFF)
		{
			//last address=를 0xfffff로 제한 & 다음 주소는 0x00000로지정 
			start = address;
			end = 0xFFFFF;
			address = 0;
		}
		// 범위를 초가하지 않는 경우 start, end address 지정
		else
		{
			start = address;
			end = address + 159;
			address = end + 1;
		}

		dump(start, end);
	}
	//command가 reset인 경우
	else if (strcmp(*command, "reset") == 0)
	{
		record(origin,command, 1);
		reset();
	}
	// command가 opcodelist인 경우
	else if (strcmp(*command, "opcodelist") == 0)
	{
		record(origin,command, 1);
		opcodelist();
	}
	else if (strcmp(*command, "symbol") == 0)
	{
		record(origin, command, 1);
		symbol(*symhead, *symbol_print_flag);
	}
	// 그 외긔 명령어가 들어오면 에러 출력
	else printf("Error : Invalid Parameter!!\n");
}
void opcodelist()
{
	int i, count = 0;
	Op *tmp;

	for (i = 0; i < 20; i++)
	{
		tmp = ophead[i];

		printf("%d : ", count);
		if (tmp == NULL)
			printf(" Empty!");
		
		while (tmp != NULL)
		{	
			printf("[%s, %02X]",tmp->inst, tmp->num);

			if (tmp->list != NULL)
				printf(" -> ");

			tmp = tmp->list;
		}
		count++;
		printf("\n");
	}
}
void mnemonic(char *origin,char **command)
{
	int i;
	Op *tmp;

	for (i = 0; i < 20; i++)
	{
		tmp = ophead[i];

		while (tmp != NULL)
		{
			if (strcmp(tmp->inst, command[1]) == 0) // opcode를 발견하면
			{
				printf("opcode is %02X.\n", tmp->num); // 내용을 출력하고
				record(origin,command, 2);
				return; // 종료
			}
			tmp = tmp->list; // 발견하지 못하면 list 이동
		}
	}
	printf("Error : There's no %s opcode\n", command[1]); // 발견하지 못하면 메세지 출력
}
void reset()
{
	int i;

	for (i = 0; i<MEMORY_SIZE; i++)
		MEMORY[i] = 0;		// 메모리에 0값 지정
}
//command token이 2개 이상인 경우
void check2(char *origin,char **command, int TOKEN_SIZE, Sym ***symhead,int *symbol_print_flag)
{
	unsigned int start, end, addr, new = 0,trash = 0;
	int i, count = 0;

	if ((strcmp(*command, "dump") == 0 || strcmp(*command, "du") == 0) && (TOKEN_SIZE <=4))
	{
		// dump : case 1(1 argument)
		if ((TOKEN_SIZE == 2) && (strchr(command[1],',') == NULL))
		{
		 	// start parameter 조건
			for(i= 0; i < strlen(command[1]); i++)
			{
				if((command[1][i] >= '0' && command[1][i] <= '9') ||(command[1][i] >= 'a' && command[1][i] <= 'f') ||
				   (command[1][i] >= 'A' && command[1][i] <= 'F') ||command[1][i] == ','|| (command[1][0] == '0' && strtol(command[1],NULL,16) >= 0x00000 && strtol(command[1],NULL,16) <= 0xfffff))
					continue;
				printf("Error : Invalid Parameter!!\n");	// dump XYZ
				return;
			}
			//조건 만족하면 변수에 저장
			start = strtol(command[1],NULL,16);
			// Boundary check
			if(start <= 0xfffff && start >= 0x00000)
			{
				if((start + 160) > 0xfffff)
				{
					end = 0xfffff;
					address = 0;
				}
				else
				{
					end = start + 159;
					address = end +1;
				}
				dump(start, end);
				record(origin,command, 2);
			}
			else printf("Error : Invalid Parameter!!\n");
		}
		// token이 2개 이상일 경우
		else
		{
			for(i = 1; i < TOKEN_SIZE - 1; i++)
			{
				strcat(command[1], " ");
				strcat(command[1], command[i + 1]);
			}
			for(i = 0; i < strlen(command[1]); i++)
				if(command[1][i] == ',') count++;
			//컴마의 갯수가 2개 이상이면 에러메세지 출력
			if(count >= 2) printf("Error : Invalid Parameter!!\n");
			else
			{
				i = sscanf(command[1], "%x , %x %x", &start, &end,&trash);
				// start와 end를 읽었다면
				if(i == 2)
				{	
					// Parameter Check
					for(i = 0; i < strlen(command[1]); i++)
					{
						if((command[1][i] >= '0' && command[1][i] <= '9') || (command[1][i] >= 'a' && command[1][i] <= 'f') ||
						   (command[1][i] >= 'A' && command[1][i] <= 'F') ||command[1][i] == ',' || command[1][i] == ' '||(start>=0x00000 &&start<=0xfffff && end<=0xfffff && end >= 0x00000))
							continue;
						printf("Error : Invalid Parameter!!\n"); 
						return;
					}
					// Boundary Check
					if(start <= end)
					{
						if((start >= 0 &&start <= 0xfffff) && (end >= 0 && end <= 0xfffff))
						{
							dump(start, end);
							record(origin,command, 2);
							if(end == 0xfffff) address = 0;
							else address = end + 1;
						}
						else printf("Error : Invalid Parameter!!\n");
					}
					else printf("Error : Invalid Parameter!!\n");
				}
				else printf("Error : Invalid Parameter!!\n");
			}
		}
	}
	// command가 edit인 경우
	else if (strcmp(*command, "edit") == 0 || strcmp(*command, "e") == 0)
	{
		//Parameter Check
		if (TOKEN_SIZE == 2 && strchr(command[1],',') == NULL)
                {
                	printf("Error : Invalid Parameter!!\n");
			return;       
                }
		else
                {
                        for(i = 1; i < TOKEN_SIZE - 1; i++)
                        {
                                strcat(command[1], " ");
                                strcat(command[1], command[i + 1]);
                        }
                        for(i = 0; i < strlen(command[1]); i++)
                                if(command[1][i] == ',') count++;
			// 컴마 갯수 Check
                        if(count >= 2) printf("Error : Invalid Parameter!!\n");
                        else
                        {
                                i = sscanf(command[1], "%x , %x %x", &addr, &new,&trash);

                                if(i == 2)
                                {
					//Parameter Chech
                                        for(i = 0; i < strlen(command[1]); i++)
                                        {
                                                if((command[1][i] >= '0' && command[1][i] <= '9') || (command[1][i] >= 'a' && command[1][i] <= 'f') ||
                                                   (command[1][i] >= 'A' && command[1][i] <= 'F') ||command[1][i] == ',' || command[1][i] == ' ' || (addr >= 0x00000 && addr<=0xfffff))
                                                        continue;
                                                printf("Error : Invalid Parameter!!\n");
                                                return;
                                        }
					// Boundary Check
                                        if(addr >= 0x00000 && addr <= 0xFFFFF)
                                        {
                                                if(new >= 0x00 && new <= 0xFF)
                                                {
                                                        edit(addr, new);
                                                        record(origin,command, TOKEN_SIZE);
                                                }
                                                else printf("Error : Invalid Parameter!!\n");
                                        }
                                        else printf("Error : Invalid Parameter!!\n");
                                }
                                else printf("Error : Invalid Parameter!!\n");
                        }
                }
	}
	// command가 fill인 경우
	else if (strcmp(*command, "fill") == 0 || strcmp(*command, "f") == 0)
	{	
		for(i = 1; i < TOKEN_SIZE - 1; i++)
               	{
			 strcat(command[1], " ");
                         strcat(command[1], command[i + 1]);
                }
                for(i = 0; i < strlen(command[1]); i++)
                	if(command[1][i] == ',') count++;

                if(count != 2) printf("Error : Invalid Parameter!!\n");
                else
		{
			i = sscanf(command[1], "%x , %x , %x %x", &start, &end,&new,&trash);

                        if(i == 3)
       	                {
				for(i = 0; i < strlen(command[1]); i++)
                                {
					if((command[1][i] >= '0' && command[1][i] <= '9') || (command[1][i] >= 'a' && command[1][i] <= 'f') ||
                                           (command[1][i] >= 'A' && command[1][i] <= 'F') ||command[1][i] == ',' || command[1][i] == ' ' || (start >= 0x00000 && start <= 0xfffff && end >=0x00000 && end <= 0xfffff))
						continue;
                                        printf("Error : Invalid Parameter!!\n");
                                        return;
                                }
                                if((start <= end)&&(start >= 0x00000 && start <= 0xFFFFF)&&((end >= 0x00000 && end <= 0xFFFFF)))
                                {	
					if(new >= 0x00 && new <= 0xFF)
                                        {
						fill(start, end, new);
                                                record(origin,command, TOKEN_SIZE);
                                        }
                                        else printf("Error : Invalid Parameter!!\n");
                                }
                                else printf("Error : Invalid Parameter!!\n");
                        }
                        else printf("Error : Invalid Parameter!!\n");
                 }
	}
	else if (strcmp(*command, "opcode") == 0)
		mnemonic(origin,command);
	else if (strcmp(*command, "type") == 0)
	{
		if(TOKEN_SIZE == 2)
			type(origin, command);
		else
		{
			printf("Error : Invalid Parameter!!\n");
			return;
		}
	}
	else if (strcmp(*command, "assemble") == 0)
	{
		if(TOKEN_SIZE == 2)
			assemble(origin, command, ophead, symhead, symbol_print_flag);
		else
		{
			printf("Error : Invalid Parameter!!\n");
			return;
		}
	}
	//그 외 명령어가 들어올 경우 에러메세지 출력
	else printf("Error : Invalid Parameter!!\n");
}
void symbol(Sym **symhead, int flag)
{
    	Sym *node = NULL, *list = NULL, *ptr = NULL, *follow = NULL, *temp;
    	int i;
    	if(!flag)
	    { 
        	printf("SYMTAB IS EMPTY\n");
	        return;
	    }
	//assemble한 적이 없거나 최근에 assemble한 file이 실패한 file일 경우
	//symtab이 존재하지 않으므로 비어있다고 알림
	
	//symtab이 존재하는 경우 내림차순으로 symbol 정렬
    	for(i = 0; i < 100; i++)
    	{
		node = symhead[i];
		//내림차순이기 때문에 ASCII값을 비교하는 반복문
		while(node)
		{
			ptr = follow = list;
        	    	while(ptr && strcmp(ptr->name, node->name) > 0)
			{
               		//strcmp값이 양수이면 다음값과 비교
                		follow = ptr;
	                	ptr = ptr->list;
        	   	}
			 //strcmp값이 음수이면 그 전 노드에 연결
	           	temp = allocate(node);
        	    	temp->list = ptr;
            		if(ptr == list)
			{
                		temp = allocate(node);
                		temp->list = ptr;
                		list = temp;
           		}
            		else    
				follow->list = temp;
        		node = node->list;
		}
		ptr = list;
    	}	
	
	//내림차순으로 정렬된 SYMBOL 출력
    	while(ptr)
    	{
        	printf("\t%s\t%04X", ptr->name, ptr->location);
	        ptr = ptr->list;
        	printf("\n");
    	}
}
// "symbol" command 수행 시 내림차순으로 정렬할 때 새로운 node에 메모리를 할당하는 함수
Sym* allocate(Sym* data)
{
    	Sym *node = (Sym*)malloc(sizeof(Sym));

 	node->location = data->location;
	strcpy(node->name, data->name);
    	node->list = data->list;

    	return node;
}

void assemble(char *origin,char **command, Op **ophead, Sym ***output, int *symbol_print_flag)
{
	FILE *fp = fopen(command[1], "rt");
    	FILE *lst, *obj, *mid;
	char temp[INPUT_SIZE];
    	char *lstname = (char*)malloc(sizeof(char) * strlen(command[1]));
    	char *objname = (char*)malloc(sizeof(char) * strlen(command[1]));
    	char operator[10], operand1[255], operand2[10], format[5], optemp[10], name[10];
    	int i, opcode, reg1, reg2, num, disp, pc = 0, base = 0, base_flag = 0, var;
    	int first_data = 0, mid_data = 0, symbol_flag = 0, end_flag = 0, start_flag = 0, error_flag = 0;
    	unsigned int line_num = 5, location = 0, length;
    	Sym **symhead = (Sym**)malloc(sizeof(Sym*) * 100), *node = (Sym*)malloc(sizeof(Sym));
    	Ob *head = NULL, *tail = NULL, *ptr1, *ptr2;
	
	if(!fp)
	{
        	printf("There is no %s file\n", command[1]);
        	return;
    	}
	//없는 파일 load시 ERROR

    	if(strcmp(command[1] + strlen(command[1]) - 4, ".asm"))
	{
        	printf("%s file is not assemble file\n", command[1]);
        	return;
    	}
	//.asm 파일 아닌 파일 load시 ERROR
	
	for(int i=0;i<100;i++)	
		symhead[i] = NULL;

	mid = fopen("mid.txt","wt");

	*symbol_print_flag = 0;

	while(fgets(temp, sizeof(temp),fp))
	{
		if(error_flag) return;

		strcpy(operand1,"");
		strcpy(operand2,"");
		strcpy(operator,"");
		strcpy(format,"");
		strcpy(node->name,"");
		strcpy(optemp,"");

		if(temp[0] == '.')	
		{
			fprintf(mid, "=%4d\t%s",line_num, temp);
			line_num += 5;
			continue;
		}
		else if(temp[0] == '\n') continue;

		else if(temp[0] == ' ' || temp[0] == '\t')
		{
			num = sscanf(temp, "%s %s %s",operator, operand1, operand2);
			if(num == 2 && strchr(operand1,','))
			{
				strcpy(operand2, strchr(operand1, ',')+1);
				for(i=0; i<strlen(operand1);i++)
				{
					if(operand1[i] == ',')
					{
						operand1[i+1]='\0';
						break;
					}
					num = 3;
				}
			}
		}

		//SYMBOL 존재O
        	else
		{
            		num = sscanf(temp, "%s %s %s %s", node->name , operator, operand1, operand2);
	    		if(!strcmp(operator, "BYTE") && operand1[0] == 'C' && num == 4)
	    		{
	    			sscanf(temp, "%s %s %[^\n]s", node->name, operator, operand1);
				num = 3;
	    		}
            		if(num == 3 && strchr(operand1, ','))
	    		{
                		strcpy(operand2, strchr(operand1, ',') + 1);
                		for(i = 0; i < strlen(operand1); i++)
                    		if(operand1[i] == ',')
		    		{
                        		operand1[i + 1] = '\0';
                        		break;
                    		}
                		num = 4;
            		}
        	}
	
		//directive 명령어 + RSUB + BASE + NOBASE 처리
        	if(!strcmp(operator, "START"))
		{
            		fprintf(mid, "=%4d\t%04X\t%-10s\t%-10s\t0\nS%s\n", line_num, location, node->name, operator,node->name);
            		line_num += 5;
		    	location = 0;
        	    	node-> location = location;
            		error_flag = build_symtab(symhead, node, line_num);
	            	continue;
        	}

	        else if(!strcmp(operator, "END"))
		{
            		fprintf(mid, "=%4d\t    \t%-10s\t%-10s\t%-20s\nE\n", line_num, node->name, operator, operand1);
            		line_num += 5;
            		break;
        	}
        	else if(!strcmp(operator, "BYTE"))
		{
            		node->location = location;
	            	if(operand1[0] == 'X')
		    	{
                		if((strlen(operand1) - 3) % 2)
				{
                	    		printf("LINE : %d HEXA DATA ERROR\n", line_num);
              			      	return;
                		//16진수 data인데 홀수개만 선언한 경우 ERROR
				}

	                	fprintf(mid, "%4d\t%04X\t%-10s\t%-10s\t%-20s\n!", line_num, location, node->name, operator, operand1);
        	        	for(i = 2; i < strlen(operand1) - 1; i++)
				{
					if(operand1[i] >= '0' && operand1[i] <= '9');
					else if(operand1[i] >= 'A' && operand1[i] <= 'F');
					else if(operand1[i] >= 'a' && operand1[i] <= 'f');
					else 
					{
						printf("LINE : %d HEXA DATA ERROR\n", line_num);
						return;
					}
					fprintf(mid, "%c", operand1[i]);
				}
				fprintf(mid, "\n");
                		error_flag = build_symtab(symhead, node, line_num);
	                	node->location = location = location + (strlen(operand1) - 3) / 2;
        	    	}
        		else if(operand1[0] == 'C')
			{
				fprintf(mid, "%4d\t%04X\t%-10s\t%-10s\t%-20s\n!", line_num, location, node->name, operator, operand1);
	                	for(i = 2; i < strlen(operand1) - 1; i++)   fprintf(mid, "%X", operand1[i]);
	                	fprintf(mid, "\n");
	                	error_flag = build_symtab(symhead, node, line_num);
	                	node->location = location = location + strlen(operand1) - 3;
	            	}
	            	line_num += 5;
        	    	continue;
	        }
        	else if(!strcmp(operator, "WORD"))
		{
            		fprintf(mid, "%4d\t%04X\t%-10s\t%-10s\t%-20s\n!", line_num, location, node->name, operator, operand1);
	            	sscanf(operand1, "%d", &var);
			if(var >= -0x800000 && var <= 0x7FFFFF)
				if(var <= -1 && var >= -0x800000)
					var += 0x1000000;
			else 
			{
				printf("LINE : %d WORD value overflow!\n", line_num);
				return;
			}
			//WORD value overflow처리
          		fprintf(mid, "%06X\n", var);
	            	error_flag = build_symtab(symhead, node, line_num);
          		node->location = location = location + 3;
           		line_num += 5;
           		continue;
        	}	
        	else if(!strcmp(operator, "RESW"))
		{
            		sscanf(operand1, "%d", &var);
            		fprintf(mid, "=%4d\t%04X\t%-10s\t%-10s\t%-20s\n~\n", line_num, location, node->name, operator, operand1);
            		error_flag = build_symtab(symhead, node, line_num);
            		node->location = location = location + 3 * var;
            		line_num += 5;
            		continue;
        	}
        	else if(!strcmp(operator, "RESB"))
		{
            		sscanf(operand1, "%d", &var);
            		fprintf(mid, "=%4d\t%04X\t%-10s\t%-10s\t%-20s\n~\n", line_num, location, node->name, operator, operand1);
            		error_flag = build_symtab(symhead, node, line_num);
            		node->location = location = location + var;
            		line_num += 5;
            		continue;
        	}

        	else if(!strcmp(operator, "BASE"))
		{
           	 	fprintf(mid, "=%4d\t    \t%-10s\t%-10s\t%-20s\nB%s\n", line_num, node->name, operator, operand1, operand1);
            		line_num += 5;
            		continue;
        	}
		
		else if(!strcmp(operator, "NOBASE"))
		{
			fprintf(mid, "=%4d\t    \t%-10s\t%-10s\t\nN\n", line_num, node->name, operator);
			line_num += 5;
			continue;
		}
		//directive 이외에 opcode table에 없는 잘못된 mnemonic 입력시 ERROR
        	else
		{
            		if(!check_mnemonic(operator, format, &opcode, ophead))
			{
                		printf("LINE : %d %s is not in OPCODE TABLE\n", line_num, operator);
                		return;
            		}
        	}

		//directive 이외 opcode table에 있는 mnemonic일 경우 정상 처리
        	node->location = location;
        	if(strcmp(node->name, ""))      
			error_flag = build_symtab(symhead, node, line_num);

      	  	location_counter(format, &location, operator);
        	strcpy(optemp, operand1);

        	if(strcmp(operand2, ""))
		{
            		optemp[strlen(optemp) - 1] = '\0';
            		strcat(optemp, "\t");
            		strcat(optemp, operand2);
            		strcat(operand1, " ");
            		strcat(operand1, operand2);
        	}

        //intermediate 파일에 필요한 자료 모두 입력
        	fprintf(mid, "%4d\t%04X\t%-10s\t%-10s\t%-20s\n\t%02X\t%2s\t%10s\t%10s\n", line_num, node->location, node->name, operator, operand1, opcode, format, operator, optemp);
        	line_num += 5;
    	}
    	length = location;
    	fclose(mid);

    	strcpy(lstname, command[1]);
    	lstname[strlen(command[1]) - 4] = '\0';
    	lst = fopen(strcat(lstname, ".lst"), "wt");

	
	////pass2////
	mid = fopen("mid.txt", "rt");
    	while(fgets(temp, sizeof(temp), mid))
	{
		//엔터있을 경우 출력양식 맞추기 위해 제거
        	if(temp[strlen(temp) - 1] == '\n') temp[strlen(temp) - 1 ] = '\0';
        
		//첫문자가 !  : BYTE, WORD
        	if(temp[0] == '!')
		{
            		fprintf(lst, "%s\n", temp + 1);
           	 	objectcode_insert(5, 0, 0, 0, location, &head, &tail, temp + 1, 0);
        	}

		//첫문자가 S  : START
        	else if(temp[0] == 'S')
		{
			strcpy(name, temp + 1);
			start_flag = 1;
		}
		
		//첫문자가 =  : objectcode 계산 불필요 따라서 바로 출력
		else if(temp[0] == '=') fprintf(lst, "%s\n", temp + 1);

		//첫문자가 N  : NOBASE
		else if(temp[0] == 'N') base_flag = 0;

		//첫문자가 B  : BASE
        	else if(temp[0] == 'B') base_flag = 1;

		//첫문자가 E  : END
	        else if(temp[0] == 'E') end_flag = 1;

		//첫문자가 ~  : RESB, RESW
        	else if(temp[0] == '~') tail->enter_flag = 1; //링크드리스트에서 개행해야된다고 알려주기

		//첫문자가 탭 : obectcode 계산
	        else if(temp[0] == '\t')
		{
			num = sscanf(temp, " %x %s %s %s %s ", &opcode, format, operator, operand1, operand2);
			//operand가 없는 경우 : RSUB, 1형식
        		if(num == 3)
			{		
				//RSUB
                		if(!strcmp(operator, "RSUB"))
				{
                    			fprintf(lst, "%-10s\n", "4F0000");
	                    		objectcode_insert(3, 0x4f, 0, 0, location, &head, &tail, "", 0);
        	        	}
				//1형식
                		else if(!strcmp(format, "1"))
				{
                 			pc = location + 1;
	                    		fprintf(lst, "%02X\n", opcode);
	                    		objectcode_insert(1, opcode, 0, 0, location, &head, &tail, "", 0);
        	        	}
				//이외 경우 ERROR
                		else
				{
                    			printf("LINE : %d OPERAND ERROR\n", line_num);
					fclose(lst);
					remove(lstname);
                    			return;
                		}
            		}
			
			//operand가 있는 경우 : 2, 3, 4형식
        	    	else if(num >= 4)
			{
				//2형식
                		if(!strcmp(format, "2"))
				{
                    			pc = location + 2;
					//operand가 하나인 2형식
					if(num == 4)
					{
						if(strcmp(operator, "TIXR") && strcmp(operator, "CLEAR") && strcmp(operator, "SVC"))	
						{
							printf("LINE : %d FORMAT2 OPERAND ERROR\n", line_num);
							fclose(lst);
							remove(lstname);
							return;
						}
	
						if(!strcmp(operator, "SVC")) sscanf(operand1, "%d", &reg1);
						else reg1 = reg_num(operand1);
	
        	        	        	if(reg1 == -1)
						{
                        		    		printf("LINE : %d REGISTER ERROR\n", line_num);
							fclose(lst);
							remove(lstname);
        	                    			return;
                	        		}
                        			fprintf(lst, "%02X%01X0\n", opcode, reg1);
                        			objectcode_insert(2, opcode, reg1*16, 0, location, &head, &tail, "", 0);
                    			}	
	
						//operand가 두개인 2형식
                    			else if(num == 5)
					{
						if(!strcmp(operator, "TIXR") || !strcmp(operator, "CLEAR") || !strcmp(operator, "SVC"))
						{
							printf("LINE : %d FORMAT2 OPERAND ERROR\n", line_num);
							fclose(lst);
							remove(lstname);
							return;
						}
						if(!strcmp(operator, "SHIFTL") || !strcmp(operator, "SHIFTR"))
						{	
							reg1 = reg_num(operand1);
							sscanf(operand2, "%d", &reg2);
						}
						else
						{
							reg1 = reg_num(operand1);
		                        		reg2 = reg_num(operand2);
						}
	
        	                		if(reg1 == -1 || reg2 == -1)
						{
                        	    			printf("LINE : %d REGISTER ERROR\n", line_num);
							fclose(lst);
							remove(lstname);
                            				return;
                     		   		}
                        			fprintf(lst, "%02X%01X%01X\n", opcode, reg1, reg2);
	                     			objectcode_insert(2, opcode, reg1*16 + reg2, 0, location, &head, &tail, "", 0);
        	            		}
                		}

				//3형식, 4형식
                		else if(!strcmp(format, "3") || !strcmp(format, "4"))
				{
                    			if(!strcmp(format, "3")) pc = location + 3;
	                    		else if(!strcmp(format, "4")) pc = location + 4;
			
					//immediate addressing
                		    	if(operand1[0] == '#')
					{
	                        		first_data = opcode + 1;
        	             		   	strcpy(operand1, operand1 + 1);
                	    		}
					//indirect addressing
                	    		else if(operand1[0] == '@')
					{
                        			first_data = opcode + 2;
	                        		strcpy(operand1, operand1 + 1);
        	            		}
					//simple addressing
	         		        else first_data = opcode + 3;
					//operand symbol location값 계산
		         	        symbol_flag = find_symbol(symhead, operand1);

					//잘못된 operand symbol일 경우 ERROR
					if(symbol_flag == -1)
					{
                        			printf("LINE : %d SYMBOL ERROR\n", line_num);
						fclose(lst);
						remove(lstname);
	        	        	        return;
        	         	   	}
						//operand symbol이 상수인경우
                    			else if(symbol_flag == -2)
					{
                        			sscanf(operand1, "%d", &disp);
						//4형식 overflow 체크
						if(!strcmp(format, "4"))
						{
							if(disp < 0 || disp >= 0x100000)
							{
								printf("LINE : %d disp overflow\n", line_num);
								fclose(lst);
								remove(lstname);
								return;
							}	
							mid_data = 1;
						}
							//3형식 overflow 체크
						else if(!strcmp(format, "3"))
						{
							if(disp < 0 || disp >= 0x1000)
							{	
	                            				printf("LINE : %d disp overflow\n", line_num);
								fclose(lst);
								remove(lstname);
		                        	    		return;
							}
                	        		}	
                    			}			
					//operand symbol 상수가 아니고 정상 확인된 경우
                    			else
					{
						//4형식
                        			if(!strcmp(format, "4"))
						{
                            				disp = symbol_flag;
		                            		mid_data = 1;
        		                	}
							//3형식
							// PC -> BASE relative overflow check
	                   		     	else if(!strcmp(format, "3"))
						{
                	        	    		disp = symbol_flag - pc;
	                		            	if(disp >= -0x800 && disp <= 0x7FF)
							{	
        	                        			if(disp >= -0x800 && disp <= -1)
								{
                                    					disp += 0x1000;
		                                		}
        		                        		mid_data = 2;
                		            		}
                        		    		else
							{
	                        		        	if(!base_flag)
								{
                        	            				printf("LINE : %d BASE FLAG IS FALSE\n",line_num);
									fclose(lst);
									remove(lstname);
                	        				            return;
                        	   		     		}
                                				symbol_flag = find_symbol(symhead, operand1);
	                                			disp = symbol_flag - base;
		        	                        	if(disp >= 0 && disp <= 0xFFF)   mid_data = 4;
        		        	                	else
								{
        	        	        	            		printf("LINE : %d disp overflow\n", line_num);
								    	fclose(lst);
						    			remove(lstname);
					    				return;
                                				}
                            				}			
                        			}
                    			}
					
					//indexed mode
		                    	if(num == 5)
					{
                		        	if(!strcmp(operand2, "X") || !strcmp(operand2, "x")) mid_data += 8;
	                		        else
						{
	                           			printf("LINE : %d %s must be X register\n", line_num, operand2);
							fclose(lst);
							remove(lstname);
                		            		return;
                        			}
                    			}		

					//3형식 출력
	                	    	if(!strcmp(format, "3"))
					{
                	        		fprintf(lst, "%02X%01X%03X\n", first_data, mid_data, disp);
	                	        	if(symbol_flag == -2)   objectcode_insert(3, first_data, mid_data, disp, location, &head, &tail, "#", 0);
        	                		else    objectcode_insert(3, first_data, mid_data, disp, location, &head, &tail, "", 0);
                	   	 	}	
	
					//4형식 출력
                	    		else if(!strcmp(format, "4"))
					{
                        			fprintf(lst, "%02X%01X%05X\n", first_data, mid_data, disp);
	                        		if(symbol_flag == -2)   objectcode_insert(4, first_data, mid_data, disp, location, &head, &tail, "", 0);
        	                		else    objectcode_insert(4, first_data, mid_data, disp, location, &head, &tail, "#", 0);
                	    		}
	                	    	first_data = mid_data = 0;
        	        	}
	
        	       		else
				{
	                    		printf("LINE : %d FORMAT ERROR\n", line_num);
					fclose(lst);
					remove(lstname);
                		    	return;
				}	
				//LDB인 경우 B register값 저장 -> 차후 BASE 명령어 입력시 relative mode 활성화 위해
		                if(strstr(operator, "LDB"))		
					base = symbol_flag;
        		}
		}
        // 나머지 : objectcode 계산이 필요
        	else
		{
            		sscanf(temp, "%d %x", &line_num, &location);
            		fprintf(lst, "%s", temp);
        	}
    	}
	
	//파일 내에 START가 없을 시 ERROR
	if(!start_flag)
	{
		printf("START NOT FOUND\n");
		fclose(lst);
		remove(lstname);
		return;
	}

	//파일 내에 END가 없을 시 ERROR
    	if(!end_flag)
	{
        	printf("END NOT FOUND\n");
		fclose(lst);
		remove(lstname);
        	return;
    	}


	//OBJECT FILE 작성

    	ptr1 = ptr2 = head;
	
	strcpy(objname, command[1]);
	objname[strlen(command[1]) - 4] = '\0';
	obj = fopen(strcat(objname, ".obj"), "wt");
	
	//HEADER RECORD
	fprintf(obj, "H%-6s000000%06X\n", name, length);

	length = 0;
	

	//TEXT RECORD
    	while(ptr1)
	{

        	fprintf(obj, "T%06X", ptr1->location);
	
		//한 LINE출력을 위해 LENGTH 계산
        	while(ptr2)
		{
            		if(ptr2->format == 5)
			{
                		if(length + strlen(ptr2->var) / 2 > 30) break;
                		else
				{
                    			length = length + strlen(ptr2->var) / 2;
                    			if(ptr2->enter_flag)
					{
                        			ptr2 = ptr2->list;
                        			break;
                    			}
                		}
            		}
            		else
			{
                		if(length + ptr2->format > 30) break;
                		else
				{
                    			length = length + ptr2->format;
	                   	 	if(ptr2->enter_flag)
					{
                        			ptr2 = ptr2->list;
	                        		break;
        	            		}
                		}
            		}
            		ptr2 = ptr2->list;
        	}
	       //한 LINE 계산 완료시
		
		//출력
		fprintf(obj, "%02X", length);
        	while(ptr1 != ptr2)
		{
            		if(ptr1->format == 1) fprintf(obj, "%02X", ptr1->first);
	            	else if(ptr1->format == 2) fprintf(obj, "%02X%02X", ptr1->first, ptr1->mid);
            		else if(ptr1->format == 3) fprintf(obj, "%02X%01X%03X", ptr1->first, ptr1->mid, ptr1->end);
	            	else if(ptr1->format == 4) fprintf(obj, "%02X%01X%05X", ptr1->first, ptr1->mid, ptr1->end);
            		else if(ptr1->format == 5) fprintf(obj, "%s", ptr1->var);
            		ptr1 = ptr1->list;
        	}
        	fprintf(obj, "\n");
        	length = 0;
    	}

	ptr1 = head;

	//MODIFICATION RECORD
    	while(ptr1)
	{
        	if(ptr1->format == 4 && strcmp(ptr1->var, "")) fprintf(obj, "M%06X05\n",ptr1->location + 1);
	        ptr1 = ptr1-> list;
    	}
	
	//END RECORD
    	fprintf(obj, "E000000\n");
    

	// "assemble" command 정상 수행 종료 알림
    	record(origin, command, 2);
	printf("output file : [%s], [%s]\n", lstname, objname);

    	free(lstname);
   	free(objname);
	lstname = objname = NULL;
    	fclose(mid);
    	fclose(fp);
    	fclose(lst);
    	fclose(obj);
    	*symbol_print_flag = 1;
	*output = symhead;
	remove("mid.txt");

    	return;
}
// "assemble file.asm" command 수행 시 나타나는 mnemonic의 opcode를 반환하는 함수
int check_mnemonic(char *mnemonic, char *format, int *opcode, Op **ophead)
{
    Op *temp;
    char data[10];
    int index, sum = 0;
	
    if(mnemonic[0] == '+')  strcpy(data, mnemonic + 1);
	//4형식일 경우 +문자 지우고 mnemonic만 받기
    else strcpy(data, mnemonic);
	//아닐 경우 그대로 진행
	
	for (int i = 0; i < strlen(data); i++)
		sum += data[i]; // 모든 아스키 값 더하기

	index = sum % 20;	// key값 정의
	
    temp = ophead[index];

    while(temp){
        if(!strcmp(temp->inst, data)){
                *opcode = temp->num;
                strcpy(format, temp->type);
                return 1;
				//중도 발견시 성공 return value 1
        }
        temp = temp->list;
    }
    return 0;
	//실패시 실패 return value 0
}
// assemble 과정에서 나타나는 symbol들을 저장하기 위한 symbol table build 함수
int build_symtab(Sym** symhead, Sym* node, int line_num)
{  
	int sum =0;
	
	for (int i = 0; i < strlen(node->name); i++)
		sum += node->name[i]; // 모든 아스키 값 더하기

	int index = sum % 100;	// key값 정의
	
    	Sym *temp, *ptr;

	if(node->name[0] >= '0' && node->name[0] <= '9')
	{
		printf("LINE : %d SYMBOL NAME ERROR\n", line_num);
		return 1;
	}
	//SYMBOL의 첫글자가 숫자인 경우 ERROR

    	if(!symhead[index])
    	{
        	symhead[index] = (Sym*)malloc(sizeof(Sym));
	        strcpy(symhead[index]->name, node->name);
        	symhead[index]->location = node->location;
	        symhead[index]->list = NULL;
    	}	
	//head가 비어있다면 head에 연결

	else
    	{
		ptr = symhead[index];
		while(ptr)
		{
			if(!strcmp(ptr->name, node->name))
			{
				printf("LINE : %d DUPLICATE SYMBOL ERROR\n", line_num);
				return 1;
				//중복된 SYMBOL이 발견되었다면 DUPLICATE SYMBOL ERROR
			}
			ptr = ptr->list;
		}
        	temp = (Sym*)malloc(sizeof(Sym));
	        strcpy(temp->name, node->name);
        	temp->location = node->location;
	        temp->list = symhead[index]->list;
        	symhead[index]->list = temp;
		//head가 비어있지 않다면 head의 뒤에 연결
    	}	
	return 0;
	//build 성공시 성공 return value 0
	//build 실패시 실패 return value 1
}

// object code 계산을 위해 operand symbol의 location값을 얻기 위한 함수
int find_symbol(Sym**symhead, char *symbol)
{
    	Sym *temp;
    	int i, index, sum = 0, const_flag = 0;

    	for(i = 0; i < strlen(symbol); i++)
    	{
        	if(symbol[i] >= '0' && symbol[i] <= '9') continue;
	        else const_flag = 1;
    	}
	//OPERAND가 상수인지 체크, 상수면 test = 0, 상수가 아니면 test = 1;

    	if(const_flag == 0)   return -2;
	//OPERAND 상수인 경우 상수 return value -2;

    	for(i=0;i<strlen(symbol);i++)
		sum += symbol[i];
    
	index = sum % 100;
   
    	temp = symhead[index];

    	while(temp)
    	{
        	if(!strcmp(temp->name, symbol))
	            return temp->location;
			//OPERAND SYMBOL 발견시 SYMBOL의 location값 return
        	temp = temp->list;
    	}
	return -1;
	//탐색 실패시 실패 return value -1;
}

// assemble과정 중 location을 계산하기 위한 함수
void location_counter(char *format,unsigned int *location, char *mnemonic)
{
    	if(!strcmp(format, "1")) *location += 1;
   	else if(!strcmp(format, "2")) *location += 2;
    	else if(!strcmp(format, "3")) *location += 3;
   	else if(!strcmp(format, "4")) *location += 4;
	else if(!strcmp(format, "3/4"))
    	{
        	if(mnemonic[0] == '+')
		{
            		*location += 4;
            		strcpy(format, "4");
        	}
        	else
		{
            		*location += 3;
            		strcpy(format, "3");
        	}
    	}
}

// 계산된 objectcode를 linked list node로 연결하는 함수
void objectcode_insert(int format, int first, int mid, int end, int location, Ob** head, Ob** tail, char *var, int enter_flag)
{
    	Ob *node = (Ob*)malloc(sizeof(Ob));

    	node->list = NULL;
	node->first = first;
    	node->mid = mid;
    	node->end = end;
    	node->location = location;
    	node->format = format;
    	strcpy(node->var, var);
    	node->enter_flag = enter_flag;
	//자료 복사

    	if(!(*head))
    	{
        	*head = node;
        	*tail = node;
    	}
	//head가 비어있다면 head에 연결

    	else
    	{
        	(*tail)->list = node;
        	*tail = node;
    	}
	//head가 비어있지 않으면 tail에 연결
}
int reg_num(char *reg)
{
	if(!strcmp(reg, "A")) return 0;
	else if(!strcmp(reg, "X")) return 1;
	else if(!strcmp(reg, "L")) return 2;
	else if(!strcmp(reg, "B")) return 3;
	else if(!strcmp(reg, "S")) return 4;
	else if(!strcmp(reg, "T")) return 5;
	else if(!strcmp(reg, "F")) return 6;
	else if(!strcmp(reg, "PC")) return 8;
	else if(!strcmp(reg, "SW")) return 9;
	else return -1;
}

void type(char *origin, char **command)
{
	FILE *fp = fopen(command[1],"r");
	char string[500];

	if(!fp) 
	{
		printf("There is no %s file\n",command[1]);
		return ;
	}

	record(origin, command, 2);

	while (!feof(fp))
        {
                fgets(string, 500, fp);
                printf("%s",string);
        }

        fclose(fp); //file close
}
void fill(unsigned int start, unsigned int end, unsigned int new)
{
	for (unsigned int i = start; i <= end; i++)
		MEMORY[i] = (char)new;	// 입력받은 범위에 새로운 값 지정
}
void edit(unsigned int origin, unsigned int new)
{
	MEMORY[origin] = (char)new; // 주어진 위치에 새로운 값 지정
}
void help()
{
	printf("h[elp]\n");
	printf("d[ir]\n");
	printf("q[uit]\n");
	printf("hi[story]\n");
	printf("du[mp] [start, end]\n");
	printf("e[dit] address, value\n");
	printf("f[ill] start, end, value\n");
	printf("reset\n");
	printf("opcode mnemonic\n");
	printf("opcodelist\n");
	printf("assemble filename\n");
	printf("type filename\n");
	printf("symbol\n");
}
//linked list 구현
void record(char *origin,char **command, int TOKEN_SIZE)
{
	Node *node;
	Node *tmp = head;
	
	node  = (Node *)malloc(sizeof(Node));
	node->list = NULL;

	strcpy(node->a,origin);			//명령어 저장
	
	if(head == NULL)
		head = node;
	else
	{
		while(tmp->list != NULL)
			tmp = tmp->list;
		tmp->list = node;
	}
}
//linked list에 들어있는 명령어 출력
void history()
{
	int count = 1;
	Node *tmp = head;

	while (1)
	{
		printf("%-3d\t%s\n", count++, tmp->a);
		tmp = tmp->list;

		if (!tmp) break;
	}
}
void dir()
{
	DIR *dir_info = opendir(".");
	struct dirent *dir_entry;
	struct stat buf;
	int space = 0;

	while ((dir_entry = readdir(dir_info)) != NULL)
	{
		stat(dir_entry->d_name, &buf);
		if (strcmp(dir_entry->d_name, "..") == 0 || strcmp(dir_entry->d_name, ".") == 0)
			continue;
		if (S_ISDIR(buf.st_mode)) // 디렉토리 출력
		{
			printf("%s/\t", dir_entry->d_name);
			space++;
		}
		else if (S_ISREG(buf.st_mode)) // 파일 출력
		{
			printf("%s*\t", dir_entry->d_name);
			space++;
		}

		if (space == 3)
		{
			printf("\n");
			space = 0;
		}
	}
	if (space % 3 != 0) printf("\n");

	closedir(dir_info);
}
void dump(unsigned int start, unsigned int end)
{
	int i, j;

	unsigned int start_h = start / 16;
	unsigned int end_h = end / 16;

	for (i = 0; i <= (end_h - start_h); i++)
	{
		//맨 왼쪽기둥 출력
		printf("%05X ", (start_h + i) * 16);
		//중간 기둥 출력
		for (j = 0; j<16; j++)
		{
			if ((start_h + i) * 16 + j < start || (start_h + i) * 16 + j > end)
				printf("   ");
			else if ((start_h + i) * 16 + j >= start && (start_h + i) * 16 + j <= end)
				printf("%02X ", MEMORY[(start_h + i) * 16 + j]);
		}

		printf(";");
		// 맨 오른쪽 기둥 출력
		for (j = 0; j<16; j++)
		{
			if ((start_h + i) * 16 + j < start || (start_h + i) * 16 + j > end)
				printf(".");
			else if ((start_h + i) * 16 + j >= start && (start_h + i) * 16 + j <= end)
			{
				if (MEMORY[(start_h + i) * 16 + j] >= 20 && MEMORY[(start_h + i) * 16 + j] <= 127)
					printf("%c", MEMORY[(start_h + i) * 16 + j]);
				else
					printf(".");
			}
		}
		printf("\n");
	}
}
