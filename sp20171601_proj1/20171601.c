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

	int flag = 0; // flag = 1일 경우 종료
	int TOKEN_SIZE; // token 덩어리의 갯수
	int i;

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
		
		for (i = 1; i < TOKEN_SIZE; i++) {
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
				check1(command,token); // check1() 함수로 이동
			else if (TOKEN_SIZE >= 2) // token의 갯수가 2개 이상이라면
				check2(command,token, TOKEN_SIZE); // check2() 함수로 이동
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
void check1(char *origin,char **command)
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
void check2(char *origin,char **command, int TOKEN_SIZE)
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
	//그 외 명령어가 들어올 경우 에러메세지 출력
	else printf("Error : Invalid Parameter!!\n");
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
