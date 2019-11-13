#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

typedef struct
{
	char OPCODE[10];
	int  opcodeNum;
}OPTABLE;

typedef struct
{
	char label[10];
	int address;
}SYMTABLE;

typedef struct
{
	char label[150];
	char opcode[150];
	char operand[150];
	int locctr;
}LINE;

OPTABLE OP[100];
SYMTABLE SYM[100];
int errorFlag;		//에러 발생한 횟수 저장할 변수
int symIndex;		//찾고자 하는 심볼의 인덱스를 저장할 변수
int countSym;		//심볼의 개수를 저장할 변수
int indexOP;		//찾고자 하는 opcode의 인덱스를 저장할 변수
int bfLOC;			//intermediate파일의 locctr을 저장할 변수
int programlength;	//프로그램의 크기를 저장할 변수 (object 프로그램에서 맨 위 프로그램길이를 표현하기 위해 사용)
void PASS2();
LINE divideLine(char* readLine);	//pass1에서 한줄을 labe, opcode, operand로 나누는 함수
LINE  divideLine2(char* readLine);	//pass2에서 한줄을 label, opcode, operand locctr로 나누는 함수
int conversionDec(char *hex);		//16진수를 10진수로 바꿔주는 함수
bool searchOPtab(char *opcode);		//매개변수 opcode가 optable에 있는지 검사하여 참,거짓 반환 index를 저장
void insertSym(char *lb, int LOC);	//symbol을 삽입하는 함수
bool searchSymtab(char* label);		//매개변수라는 symbol이 존재하는지 검사하여 참,거짓 반환 index를 저장
void optab();		//optable을 생성하는 함수

void PASS1(FILE *sourceCode)		//pass1 함수
{
	FILE* intermediate;		

	int LOCCTR;		
	int startAddress;		//시작 주소 저장할 변수
	char line[255] = "";	//sourceprogram 한줄을 저장할 변수
	LINE linecode = { 0 };	//label, opcode, operand, locctr을 저장할 구조체 pass1에서는 locctr을 저장하지 않음

	if ((intermediate = fopen("intermediate.txt", "w")) == NULL)		//파일을 열고 검사
	{
		fprintf(stderr, "파일을 오류");
		exit(1);
	}
	fgets(line, 255, sourceCode);	//sourceprogram 한줄을 읽어옴
	linecode = divideLine(line);	//한줄을 label, opcode, operand로 분리

	if (strcmp(linecode.opcode, "START") == 0)		//만약 opcode가 START라면
	{
		startAddress = conversionDec(linecode.operand);	//operand를 10진수로 바꿔 startAddress에 저장
		LOCCTR = startAddress;		//startaddress를 초기 LOCCTR로 설정
		fprintf(intermediate, "%s\t%s\t%s\t\t%04X\n", linecode.label, linecode.opcode, linecode.operand, LOCCTR);	//intermediate파일에 label, opcode, operand, LOCCTR을 저장
		fgets(line, 255, sourceCode);	
		linecode = divideLine(line);
	}
	else
		LOCCTR = 0;

	while (strcmp(linecode.opcode, "END") != 0)
	{
		bfLOC = LOCCTR;		//현재 LOCCTR을 저장(intermediate파일에는 현재의 주소를 써줄 것이기 때문)
		if (strcmp(linecode.label, ".") != 0)		//주석이 아니라면
		{
			if (strcmp(linecode.label, "\t") != 0)	//라벨이 있으면
			{
				if (searchSymtab(linecode.label) == true)	//label이 symboltable에 저장되있다면 오류이므로
				{
					errorFlag += 1;
					printf("duplicate error\n");
				}	
				else	//당연히 없으면 symboltable에 삽입
					insertSym(linecode.label, LOCCTR);
			}
			if (searchOPtab(linecode.opcode) == true)		//opcode 하나당 3바이트 이므로 LOCCTR을 +3해준다.
				LOCCTR += 3;
			else if (strcmp(linecode.opcode, "WORD") == 0)
				LOCCTR += 3;
			else if (strcmp(linecode.opcode, "RESW") == 0)
				LOCCTR += (3 * atoi(linecode.operand));
			else if (strcmp(linecode.opcode, "RESB") == 0)
				LOCCTR += atoi(linecode.operand);
			else if (strcmp(linecode.opcode, "BYTE") == 0)
			{
				if (linecode.operand[0] == 'X')		//OPCODE가 BYTE인 경우 operand앞이 X인경우와 C인 경우가 있다. (listing 파일을 확인하면 X는 1이 증가하고 C는 3이 증가함
					LOCCTR += 1;
				else
					LOCCTR += 3;
			}
			else
			{
				printf("error발생\n");
				errorFlag += 1;
			}
		}
		if (strcmp(linecode.label, ".") != 0)	// 주석이 아닌 sourcecode를 intermediate파일에 저장하는 경우
		{
			if (strcmp(linecode.label, "\t") == 0 && strcmp(linecode.operand, "\t") == 0)//RSUB의 경우
			{
				fprintf(intermediate, "%s%s\t%s\t%04X\n", linecode.label, linecode.opcode, linecode.operand, bfLOC);
			}
			else if (strcmp(linecode.label, "\t") != 0)//라벨이 있을경우
			{
				fprintf(intermediate, "%s\t%s\t%s\t\t%04X\n", linecode.label, linecode.opcode, linecode.operand, bfLOC);
			}
			else //라벨만 없는 경우
			{
				if(strcmp(linecode.operand,"BUFFER,X") == 0)
					fprintf(intermediate, "%s%s\t%s\t%04X\n", linecode.label, linecode.opcode, linecode.operand, bfLOC);
				else
					fprintf(intermediate, "%s%s\t%s\t\t%04X\n", linecode.label, linecode.opcode, linecode.operand, bfLOC);
				
			}
		}
		else		//주석을 써주는 경우
		{
			fprintf(intermediate, "%s\t%s%s\n", linecode.label, linecode.opcode, linecode.operand);	
		}
		fgets(line, 255, sourceCode);
		linecode = divideLine(line);
	}
	programlength = LOCCTR - startAddress;		//프로그램 길이는 지금까지 더해온 LOCCTR에서 초기 주소값(1000)을 빼준 것.
	fprintf(intermediate, "%s%s\t%s", linecode.label,linecode.opcode, linecode.operand);
	fclose(intermediate);
}
bool searchOPtab(char *opcode)	//opcode가 optab 존재하는지 검사
{
	int i = 0;
	for (i = 0; i < 100; i++)
	{
		if (strcmp(opcode, OP[i].OPCODE) == 0)
		{
			indexOP = i;
			return true;
		}
	}
	return false;
}
void insertSym(char *lb, int LOC)		//symboltable에 삽입
{
	strcpy(SYM[countSym].label, lb);
	SYM[countSym].address = LOC;
	countSym += 1;
	
}
bool searchSymtab(char* label)		//매개변수가 symboltable에 존재하는지 검사
{
	for (int i = 0; i < 100; i++)	//symtab 구조체 개수가 100개
	{
		if (strcmp(label, SYM[i].label) == 0)
		{
			symIndex = i;
			return true;
		}
		else if (strcmp(label, "BUFFER,X") == 0)		//BUFFER,X는 symtab에 넣지 않음
		{
			symIndex = 1000;
			return true;
		}
	}
	return false;
}
int conversionDec(char *hexadecimal)		//16진수를 10진수로 바꿔주는 함수(그냥 가져다 쓸 것)
{
	int decimal = 0;                  // 10진수를 저장할 변수

	int position = 0;
	for (int i = strlen(hexadecimal) - 1; i >= 0; i--)    // 문자열을 역순으로 반복
	{
		char ch = hexadecimal[i];         // 각 자릿수에 해당하는 문자를 얻음

		if (ch >= 48 && ch <= 57)         // 문자가 0~9이면(ASCII 코드 48~57)
		{
			// 문자에서 0에 해당하는 ASCII 코드 값을 빼고
			// 16에 자릿수를 거듭제곱한 값을 곱함
			decimal += (ch - 48) * pow(16, position);
		}
		else if (ch >= 65 && ch <= 70)    // 문자가 A~F이면(ASCII 코드 65~70)
		{                                 // 대문자로 된 16진수의 처리
										  // 문자에서 (A에 해당하는 ASCII 코드 값 - 10)을 빼고
										  // 16에 자릿수를 거듭제곱한 값을 곱함
			decimal += (ch - (65 - 10)) * pow(16, position);
		}
		else if (ch >= 97 && ch <= 102)   // 문자가 a~f이면(ASCII 코드 97~102)
		{                                 // 소문자로 된 16진수의 처리
										  // 문자에서 (a에 해당하는 ASCII 코드 값 - 10)을 빼고
										  // 16에 자릿수를 거듭제곱한 값을 곱함
			decimal += (ch - (97 - 10)) * pow(16, position);
		}

		position++;
	}
	return decimal;
}
int conversionHex(char *dec)		//10진수를 16진수로 바꿔줌
{
	int i, j;
	char result[11] = "00000000";
	int k = 9;
	int last = 0;

	i = atoi(dec);
	while (i >= 16)
	{
		j = i % 16;
		if (j > 9)
			result[k--] = j + 46;
		result[k--] = j + 48;
		i = i / 16;
	}
	result[k] = i + 48;
	i = 0;
	while (result[i++] == '0');
	strcpy(result, result + i - 1);
	last = atoi(result);
	return last;
	
}
LINE divideLine(char* readLine)		//sourceprogram 한줄을 나누는 함수
{
	char *token;
	char awd[100];
	char first[15], second[100], third[150] = "";
	LINE saveLine = { 0 };
	int i=0;
	token = strtok(readLine, "\t\n");	//띄어쓰기 tab 엔터를 토큰으로 하여 한 단어를 token에 저장
	strcpy(first, token);		//token을 first에 임시 저장
	if (strcmp(first, ".") == 0)	//주석이라면 주석 내용을 저장한다.
	{
		strcpy(saveLine.label, first);
		token = strtok(NULL, "\n");
		if (token != NULL)
		{
			strcpy(saveLine.opcode, token);
			strcpy(saveLine.operand, "\t");
		}
		printf("%s", saveLine.opcode);
		return saveLine;
	}
	else		//주석이 아니라면
	{
		while (token != NULL)	//token이 없을때까지(한줄 끝까지)
		{
			//한 줄 code에 label, opcode, operand 유뮤에 따른 분류
			if (i == 1)		//opcode밖에 없는경우(RSUB)
				strcpy(second, token);
			if (i == 2)		//opcode와 operand밖에 없는 경우
				strcpy(third, token);
			i++;
			token = strtok(NULL, " .\t\n");
		}
		if (i == 1)	//opcode밖에 없는경우(RSUB)
		{
			strcpy(saveLine.label, "\t");
			strcpy(saveLine.opcode, first);
			strcpy(saveLine.operand, "\t");
		}
		else if (i == 2)	//opcode와 operand밖에 없는 경우
		{
			strcpy(saveLine.label, "\t");
			strcpy(saveLine.opcode, first);
			strcpy(saveLine.operand, second);
		}
		else if (i == 3)	//label, opcode, operand 다 있는 경우
		{
			strcpy(saveLine.label, first);
			strcpy(saveLine.opcode, second);
			strcpy(saveLine.operand, third);
		}
		return saveLine;
	}
}
void PASS2()
{
	bool start = true;
	int startAddress = 0;
	int lineAddress = 0;
	char sumAddress[10] = "";		//
	char wordad[10] = "";
	char objectLine[61] = "";
	int objnum = 0;
	int temp;
	int i;
	LINE linecode = { 0 };
	FILE *obj;
	FILE *intermediate;
	FILE *list;
	char oneLine[255] = "";
	int lineopcode;
	//파일을 다 열어 주고
	if ((obj = fopen("objectProgram.txt", "w")) == NULL)
	{
		fprintf(stderr, "파일 에러");
		exit(1);
	}
	if ((list = fopen("listing.txt", "w")) == NULL)
	{
		fprintf(stderr, "파일 에러");
		exit(1);
	}
	if ((intermediate = fopen("intermediate.txt", "r")) == NULL)
	{
		fprintf(stderr, "파일 에러");
		exit(1);
	}
	fgets(oneLine, 255, intermediate);	//intermeidate 한줄을 읽고
	linecode = divideLine2(oneLine);	//한 줄을 label opcode operand locctr로 분리하여 저장한 구조체를 linecode에 저장
	if (strcmp(linecode.opcode, "START") == 0)		//opcdoe가 start라면
	{
		fprintf(obj, "H%-6s%06X%06X\n", linecode.label, linecode.locctr, programlength);		//objectprogram 첫줄에 opcode와 locctr과 프로그램의 크기를 써준다.(objectprogram의 첫줄은 이 의미이다.)
		fprintf(list, "%04X\t%s\t%s\t%s\t\n", linecode.locctr, linecode.label, linecode.opcode, linecode.operand);		//listing 파일의 첫줄에도 써준다.
	}
	fgets(oneLine, 255, intermediate);		//한 줄 읽고
	linecode = divideLine2(oneLine);		//분리
	while (strcmp(linecode.opcode, "END") != 0)		//opcode END 나올때까지
	{
		if (linecode.label[0] != '.')		//주석이라면
		{
			if (start == true)				//object프로그램에서 줄 맨 앞에다가 한 줄의 시작 주소를 표현하기 위해 
			{
				startAddress = linecode.locctr;			//locctr을 시작 맨 앞에다가 써주기 위해 startaddress에 저장
				start = false;						//objectprogram은 한줄에 locctr을 60까지 표현 그전까지는 start를 false로 하여 시작주소를 저장하는 것을 막음
			}
		}
		if (linecode.label[0] != '.')		//주석이 아니라면
		{
			if (searchOPtab(linecode.opcode) == true)		//opcode가 optable에 존재하면
			{
				lineopcode = OP[indexOP].opcodeNum;		//opcode의 10진수로 표현된 값을 lineopcode에 저장 indexOP는 searchOPtab에서 설정됨 (전역변수)
				if (linecode.operand[0] != '\0')		//operand있으면 
				{
					if (searchSymtab(linecode.operand) == true)			//operand가 symtab에 존재하면
					{
						lineAddress = SYM[symIndex].address;		//symbol의 지정된 주소를 lineAddress에 저장
						if (strcmp(linecode.operand, "BUFFER,X") == 0)		//BUFFER,X인 경우 무조건 9039가 object코드에 표현
						{
							lineAddress = 36921; //36921 16진수로 변환시 9039가 됨
						}
					}
					else
					{
						lineAddress = 0;		//operand가 symboltable에 저장되있지않으면 오류
						errorFlag += 1;
					}
				}
				else
				{
					lineAddress = 0;		
				}
				sprintf(sumAddress, "%02X%04X", lineopcode, lineAddress);		//object코드 생성 
			}
			else if (strcmp(linecode.opcode, "BYTE") == 0)		//opcode가 BYTE이면 C인경우가 있고 X인경우가 있다. 
			{
				if (strcmp(linecode.operand, "C'EOF'") == 0)	//C'EOF'인 경우 objectcode는 454F46이다.
				{
					sprintf(sumAddress, "%s", "454F46");
					sumAddress[6] = NULL;
				}
				else if (linecode.operand[0] == 'X')		//X'뭐시기' 인경우 뭐시기만 object코드에 들어간다.
				{
					sprintf(sumAddress, "%c%c", linecode.operand[2], linecode.operand[3]);
					sumAddress[2] = NULL;
				}
			}
			else if (strcmp(linecode.opcode, "WORD") == 0)		//WORD 인경우
			{
				if (strcmp(linecode.label, "MAXLEN") == 0)		//label이 MAXLEN인 경우
				{
					temp = conversionHex(linecode.operand);		//operand를 16진수로 바꿔준다(4096을 16진수로 변환하면 1000이다)
					itoa(temp, wordad, 10);		//10진수로 바꿔준다.
					sprintf(sumAddress, "00%s", wordad);		//001000
				}
				else
					sprintf(sumAddress, "%06s", linecode.operand);		//operand를 바로 써준다.(000003),(000000)
			}
			else if (strcmp(linecode.opcode, "RESW") == 0)		//REW인 경우 object코드 x
			{
				sprintf(sumAddress, "%s", "");
			}
			else if (strcmp(linecode.opcode, "RESB") == 0)		//RESB인 경우 obejct코드 x
			{
				sprintf(sumAddress, "%s", "");
			}
			if (strcmp(linecode.label, "\t") == 0 && strcmp(linecode.operand, "\t") == 0)//label과 operand가 없는 경우
			{
				fprintf(list, "%04X\t\t%s\t%s\t%s\n", linecode.locctr, linecode.opcode, linecode.operand, sumAddress);
			}
			else if (strcmp(linecode.label, "\t") == 0)		//label만 없는 경우
			{
				if(strcmp(linecode.operand,"BUFFER,X") == 0)
					fprintf(list, "%04X\t\t%s\t%s\t%s\n", linecode.locctr, linecode.opcode, linecode.operand, sumAddress);
				else
					fprintf(list, "%04X\t\t%s\t%s\t\t%s\n", linecode.locctr, linecode.opcode, linecode.operand, sumAddress);
			}
			else		//label opcode operand 모두 있는 경우
			{
				fprintf(list, "%04X\t%s\t%s\t%s\t\t%s\n", linecode.locctr, linecode.label, linecode.opcode, linecode.operand, sumAddress);
			}

			strcat(objectLine, sumAddress);		//objectprogram에 한줄의 object코드를 넣기 위해 objectLine이라는 문자열 저장 변수에 sumAddress를 계속 이어붙여준다.
			objnum += strlen(sumAddress);		//objectLine의 길이를 저장하는 ojbnum

			if (strcmp(linecode.opcode, "RESB") == 0)		//Byte를 operand만큼 추가하는 것이므로 objnum에 operand 값만큼 더해준다.
			{
				objnum += conversionHex(linecode.operand);
			}
			if (strcmp(linecode.opcode, "RESW") == 0)		//WORD는 3바이트이므로
			{
				objnum += conversionHex(linecode.operand) * 3;
			}

			if (objnum > 55)		//objnum은 60까지 이므로
			{
				fprintf(obj, "T%06X%02X%s\n", startAddress, strlen(objectLine) / 2, objectLine);
				start = true;
				for (i = 0; i < 61; i++)
					objectLine[i] = '\0';
				objnum = 0;
			}
		}
		else
			fprintf(list, "\t%s\t%s\n",  linecode.label, linecode.opcode);
		fgets(oneLine, 255, intermediate);
		linecode = divideLine2(oneLine);
	}
	fprintf(obj, "T%06X%02X%s\n", startAddress, strlen(objectLine) / 2, objectLine);
	if (strcmp(linecode.opcode, "END") == 0)
	{
		searchSymtab(linecode.operand);
		fprintf(obj, "E%06X", SYM[symIndex].address);
		fprintf(list, "\t\t%s\t%s", linecode.opcode, linecode.operand);
	}
	fclose(obj);
	fclose(list);
	fclose(intermediate);
}
LINE  divideLine2(char* readLine)		//pass2 intermediate를 label,opcode,operand,locctr로 분리
{
	int i=0;
	char *token;
	char first[10], second[100], third[100], fourth[100];
	LINE saveLine = { 0 };
	token = strtok(readLine, "\t\n");
	strcpy(first, token);
	if (strcmp(first, ".") == 0)
	{
		strcpy(saveLine.label, first);
		token = strtok(NULL, "\n");
		if (token != NULL)
		{
			strcpy(saveLine.opcode, token);
			strcpy(saveLine.operand, "\t");
		}
		printf("%s", saveLine.opcode);
		return saveLine;
	}
	else		//주석이 아닐 경우
	{
		while (token != NULL)
		{
			if (i == 1) 
				strcpy(second, token);
			if (i == 2)
				strcpy(third, token);
			if (i == 3)
				strcpy(fourth, token);
			i++;
			token = strtok(NULL, " .\t\n");
		}			//토큰의 개수에 따라 
		if (strcmp(first, "END") == 0)
		{
			strcpy(saveLine.opcode, first);
			strcpy(saveLine.operand, second);
			return saveLine;
		}
		if (i == 2)		//토큰이 2개이면
		{
			strcpy(saveLine.label, "\t");
			strcpy(saveLine.opcode, first);
			strcpy(saveLine.operand, "\t");
			saveLine.locctr = conversionDec(second);
		}
		else if (i == 3)		//3개이면
		{
			strcpy(saveLine.label, "\t");
			strcpy(saveLine.opcode, first);
			strcpy(saveLine.operand, second);
			saveLine.locctr = conversionDec(third);
		}
		else if (i == 4)		//4개이면
		{
			strcpy(saveLine.label, first);
			strcpy(saveLine.opcode, second);
			strcpy(saveLine.operand, third);
			saveLine.locctr = conversionDec(fourth);
		}
		
		return saveLine;
	}
}
void optab()
{
	strcpy(OP[0].OPCODE, "ADD");
	OP[0].opcodeNum = 24;
	strcpy(OP[1].OPCODE, "ADDF");
	OP[1].opcodeNum = 88;
	strcpy(OP[2].OPCODE, "ADDR");
	OP[2].opcodeNum = 144;
	strcpy(OP[3].OPCODE, "AND");
	OP[3].opcodeNum = 64;
	strcpy(OP[4].OPCODE, "CLEAR");
	OP[4].opcodeNum = 180;
	strcpy(OP[5].OPCODE, "COMP");
	OP[5].opcodeNum = 40;
	strcpy(OP[6].OPCODE, "COMPF");
	OP[6].opcodeNum = 136;
	strcpy(OP[7].OPCODE, "COMPR");
	OP[7].opcodeNum = 160;
	strcpy(OP[8].OPCODE, "DIV");
	OP[8].opcodeNum = 36;
	strcpy(OP[9].OPCODE, "DIVF");
	OP[9].opcodeNum = 100;
	strcpy(OP[10].OPCODE, "DIVR");
	OP[10].opcodeNum = 156;
	strcpy(OP[11].OPCODE, "FIX");
	OP[11].opcodeNum = 196;
	strcpy(OP[12].OPCODE, "FLOAT");
	OP[12].opcodeNum = 192;
	strcpy(OP[13].OPCODE, "HIO");
	OP[13].opcodeNum = 244;
	strcpy(OP[14].OPCODE, "J");
	OP[14].opcodeNum = 60;
	strcpy(OP[15].OPCODE, "JEQ");
	OP[15].opcodeNum = 48;
	strcpy(OP[16].OPCODE, "JGT");
	OP[16].opcodeNum = 52;
	strcpy(OP[17].OPCODE, "JLT");
	OP[17].opcodeNum = 56;
	strcpy(OP[18].OPCODE, "JSUB");
	OP[18].opcodeNum = 72;
	strcpy(OP[19].OPCODE, "LDA");
	OP[19].opcodeNum = 0;
	strcpy(OP[20].OPCODE, "LDB");
	OP[20].opcodeNum = 104;
	strcpy(OP[21].OPCODE, "LDCH");
	OP[21].opcodeNum = 80;
	strcpy(OP[22].OPCODE, "LDF");
	OP[22].opcodeNum = 112;
	strcpy(OP[23].OPCODE, "LDL");
	OP[23].opcodeNum = 8;
	strcpy(OP[24].OPCODE, "LDS");
	OP[24].opcodeNum = 108;
	strcpy(OP[25].OPCODE, "LDT");
	OP[25].opcodeNum = 116;
	strcpy(OP[26].OPCODE, "LDX");
	OP[26].opcodeNum = 4;
	strcpy(OP[27].OPCODE, "LPS");
	OP[27].opcodeNum = 208;
	strcpy(OP[28].OPCODE, "MUL");
	OP[28].opcodeNum = 32;
	strcpy(OP[29].OPCODE, "MULF");
	OP[29].opcodeNum = 96;
	strcpy(OP[30].OPCODE, "MULR");
	OP[30].opcodeNum = 152;
	strcpy(OP[31].OPCODE, "NORM");
	OP[31].opcodeNum = 200;
	strcpy(OP[32].OPCODE, "OR");
	OP[32].opcodeNum = 68;
	strcpy(OP[33].OPCODE, "RD");
	OP[33].opcodeNum = 216;
	strcpy(OP[34].OPCODE, "RMO");
	OP[34].opcodeNum = 172;
	strcpy(OP[35].OPCODE, "RSUB");
	OP[35].opcodeNum = 76;
	strcpy(OP[36].OPCODE, "SHIFTL");
	OP[36].opcodeNum = 164;
	strcpy(OP[37].OPCODE, "SHIFTR");
	OP[37].opcodeNum = 168;
	strcpy(OP[38].OPCODE, "SIO");
	OP[38].opcodeNum = 240;
	strcpy(OP[39].OPCODE, "SSK");
	OP[39].opcodeNum = 236;
	strcpy(OP[40].OPCODE, "STA");
	OP[40].opcodeNum = 12;
	strcpy(OP[41].OPCODE, "STB");
	OP[41].opcodeNum = 120;
	strcpy(OP[42].OPCODE, "STCH");
	OP[42].opcodeNum = 84;
	strcpy(OP[43].OPCODE, "STF");
	OP[43].opcodeNum = 128;
	strcpy(OP[44].OPCODE, "STI");
	OP[44].opcodeNum = 212;
	strcpy(OP[45].OPCODE, "STL");
	OP[45].opcodeNum = 20;
	strcpy(OP[46].OPCODE, "STS");
	OP[46].opcodeNum = 124;
	strcpy(OP[47].OPCODE, "STSW");
	OP[47].opcodeNum = 232;
	strcpy(OP[48].OPCODE, "STT");
	OP[48].opcodeNum = 132;
	strcpy(OP[49].OPCODE, "STX");
	OP[49].opcodeNum = 16;
	strcpy(OP[50].OPCODE, "SUB");
	OP[50].opcodeNum = 28;
	strcpy(OP[51].OPCODE, "SUBF");
	OP[51].opcodeNum = 92;
	strcpy(OP[52].OPCODE, "SUBR");
	OP[52].opcodeNum = 148;
	strcpy(OP[53].OPCODE, "SVC");
	OP[53].opcodeNum = 176;
	strcpy(OP[54].OPCODE, "TD");
	OP[54].opcodeNum = 224;
	strcpy(OP[55].OPCODE, "TIO");
	OP[55].opcodeNum = 248;
	strcpy(OP[56].OPCODE, "TIX");
	OP[56].opcodeNum = 44;
	strcpy(OP[57].OPCODE, "TIXR");
	OP[57].opcodeNum = 184;
	strcpy(OP[58].OPCODE, "WD");
	OP[58].opcodeNum = 220;
}
void main()
{
	FILE* sourceCode;
	FILE* intermediate;
	FILE* objectProgram;
	int i;
	if ((sourceCode = fopen("sourcecode.txt", "r")) == NULL)
	{
		fprintf(stderr, "파일 오류");
		exit(1);
	}
	optab();
	PASS1(sourceCode);
	PASS2();
	printf("\n\n");
	printf("\n      SymbolTable\n");
	for (i = 0; i < countSym; i++)
	{
		printf("%s\t\t%X\n", SYM[i].label, SYM[i].address);
	}
	
	
}
