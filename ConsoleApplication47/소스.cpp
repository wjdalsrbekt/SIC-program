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
int errorFlag;		//���� �߻��� Ƚ�� ������ ����
int symIndex;		//ã���� �ϴ� �ɺ��� �ε����� ������ ����
int countSym;		//�ɺ��� ������ ������ ����
int indexOP;		//ã���� �ϴ� opcode�� �ε����� ������ ����
int bfLOC;			//intermediate������ locctr�� ������ ����
int programlength;	//���α׷��� ũ�⸦ ������ ���� (object ���α׷����� �� �� ���α׷����̸� ǥ���ϱ� ���� ���)
void PASS2();
LINE divideLine(char* readLine);	//pass1���� ������ labe, opcode, operand�� ������ �Լ�
LINE  divideLine2(char* readLine);	//pass2���� ������ label, opcode, operand locctr�� ������ �Լ�
int conversionDec(char *hex);		//16������ 10������ �ٲ��ִ� �Լ�
bool searchOPtab(char *opcode);		//�Ű����� opcode�� optable�� �ִ��� �˻��Ͽ� ��,���� ��ȯ index�� ����
void insertSym(char *lb, int LOC);	//symbol�� �����ϴ� �Լ�
bool searchSymtab(char* label);		//�Ű�������� symbol�� �����ϴ��� �˻��Ͽ� ��,���� ��ȯ index�� ����
void optab();		//optable�� �����ϴ� �Լ�

void PASS1(FILE *sourceCode)		//pass1 �Լ�
{
	FILE* intermediate;		

	int LOCCTR;		
	int startAddress;		//���� �ּ� ������ ����
	char line[255] = "";	//sourceprogram ������ ������ ����
	LINE linecode = { 0 };	//label, opcode, operand, locctr�� ������ ����ü pass1������ locctr�� �������� ����

	if ((intermediate = fopen("intermediate.txt", "w")) == NULL)		//������ ���� �˻�
	{
		fprintf(stderr, "������ ����");
		exit(1);
	}
	fgets(line, 255, sourceCode);	//sourceprogram ������ �о��
	linecode = divideLine(line);	//������ label, opcode, operand�� �и�

	if (strcmp(linecode.opcode, "START") == 0)		//���� opcode�� START���
	{
		startAddress = conversionDec(linecode.operand);	//operand�� 10������ �ٲ� startAddress�� ����
		LOCCTR = startAddress;		//startaddress�� �ʱ� LOCCTR�� ����
		fprintf(intermediate, "%s\t%s\t%s\t\t%04X\n", linecode.label, linecode.opcode, linecode.operand, LOCCTR);	//intermediate���Ͽ� label, opcode, operand, LOCCTR�� ����
		fgets(line, 255, sourceCode);	
		linecode = divideLine(line);
	}
	else
		LOCCTR = 0;

	while (strcmp(linecode.opcode, "END") != 0)
	{
		bfLOC = LOCCTR;		//���� LOCCTR�� ����(intermediate���Ͽ��� ������ �ּҸ� ���� ���̱� ����)
		if (strcmp(linecode.label, ".") != 0)		//�ּ��� �ƴ϶��
		{
			if (strcmp(linecode.label, "\t") != 0)	//���� ������
			{
				if (searchSymtab(linecode.label) == true)	//label�� symboltable�� ������ִٸ� �����̹Ƿ�
				{
					errorFlag += 1;
					printf("duplicate error\n");
				}	
				else	//�翬�� ������ symboltable�� ����
					insertSym(linecode.label, LOCCTR);
			}
			if (searchOPtab(linecode.opcode) == true)		//opcode �ϳ��� 3����Ʈ �̹Ƿ� LOCCTR�� +3���ش�.
				LOCCTR += 3;
			else if (strcmp(linecode.opcode, "WORD") == 0)
				LOCCTR += 3;
			else if (strcmp(linecode.opcode, "RESW") == 0)
				LOCCTR += (3 * atoi(linecode.operand));
			else if (strcmp(linecode.opcode, "RESB") == 0)
				LOCCTR += atoi(linecode.operand);
			else if (strcmp(linecode.opcode, "BYTE") == 0)
			{
				if (linecode.operand[0] == 'X')		//OPCODE�� BYTE�� ��� operand���� X�ΰ��� C�� ��찡 �ִ�. (listing ������ Ȯ���ϸ� X�� 1�� �����ϰ� C�� 3�� ������
					LOCCTR += 1;
				else
					LOCCTR += 3;
			}
			else
			{
				printf("error�߻�\n");
				errorFlag += 1;
			}
		}
		if (strcmp(linecode.label, ".") != 0)	// �ּ��� �ƴ� sourcecode�� intermediate���Ͽ� �����ϴ� ���
		{
			if (strcmp(linecode.label, "\t") == 0 && strcmp(linecode.operand, "\t") == 0)//RSUB�� ���
			{
				fprintf(intermediate, "%s%s\t%s\t%04X\n", linecode.label, linecode.opcode, linecode.operand, bfLOC);
			}
			else if (strcmp(linecode.label, "\t") != 0)//���� �������
			{
				fprintf(intermediate, "%s\t%s\t%s\t\t%04X\n", linecode.label, linecode.opcode, linecode.operand, bfLOC);
			}
			else //�󺧸� ���� ���
			{
				if(strcmp(linecode.operand,"BUFFER,X") == 0)
					fprintf(intermediate, "%s%s\t%s\t%04X\n", linecode.label, linecode.opcode, linecode.operand, bfLOC);
				else
					fprintf(intermediate, "%s%s\t%s\t\t%04X\n", linecode.label, linecode.opcode, linecode.operand, bfLOC);
				
			}
		}
		else		//�ּ��� ���ִ� ���
		{
			fprintf(intermediate, "%s\t%s%s\n", linecode.label, linecode.opcode, linecode.operand);	
		}
		fgets(line, 255, sourceCode);
		linecode = divideLine(line);
	}
	programlength = LOCCTR - startAddress;		//���α׷� ���̴� ���ݱ��� ���ؿ� LOCCTR���� �ʱ� �ּҰ�(1000)�� ���� ��.
	fprintf(intermediate, "%s%s\t%s", linecode.label,linecode.opcode, linecode.operand);
	fclose(intermediate);
}
bool searchOPtab(char *opcode)	//opcode�� optab �����ϴ��� �˻�
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
void insertSym(char *lb, int LOC)		//symboltable�� ����
{
	strcpy(SYM[countSym].label, lb);
	SYM[countSym].address = LOC;
	countSym += 1;
	
}
bool searchSymtab(char* label)		//�Ű������� symboltable�� �����ϴ��� �˻�
{
	for (int i = 0; i < 100; i++)	//symtab ����ü ������ 100��
	{
		if (strcmp(label, SYM[i].label) == 0)
		{
			symIndex = i;
			return true;
		}
		else if (strcmp(label, "BUFFER,X") == 0)		//BUFFER,X�� symtab�� ���� ����
		{
			symIndex = 1000;
			return true;
		}
	}
	return false;
}
int conversionDec(char *hexadecimal)		//16������ 10������ �ٲ��ִ� �Լ�(�׳� ������ �� ��)
{
	int decimal = 0;                  // 10������ ������ ����

	int position = 0;
	for (int i = strlen(hexadecimal) - 1; i >= 0; i--)    // ���ڿ��� �������� �ݺ�
	{
		char ch = hexadecimal[i];         // �� �ڸ����� �ش��ϴ� ���ڸ� ����

		if (ch >= 48 && ch <= 57)         // ���ڰ� 0~9�̸�(ASCII �ڵ� 48~57)
		{
			// ���ڿ��� 0�� �ش��ϴ� ASCII �ڵ� ���� ����
			// 16�� �ڸ����� �ŵ������� ���� ����
			decimal += (ch - 48) * pow(16, position);
		}
		else if (ch >= 65 && ch <= 70)    // ���ڰ� A~F�̸�(ASCII �ڵ� 65~70)
		{                                 // �빮�ڷ� �� 16������ ó��
										  // ���ڿ��� (A�� �ش��ϴ� ASCII �ڵ� �� - 10)�� ����
										  // 16�� �ڸ����� �ŵ������� ���� ����
			decimal += (ch - (65 - 10)) * pow(16, position);
		}
		else if (ch >= 97 && ch <= 102)   // ���ڰ� a~f�̸�(ASCII �ڵ� 97~102)
		{                                 // �ҹ��ڷ� �� 16������ ó��
										  // ���ڿ��� (a�� �ش��ϴ� ASCII �ڵ� �� - 10)�� ����
										  // 16�� �ڸ����� �ŵ������� ���� ����
			decimal += (ch - (97 - 10)) * pow(16, position);
		}

		position++;
	}
	return decimal;
}
int conversionHex(char *dec)		//10������ 16������ �ٲ���
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
LINE divideLine(char* readLine)		//sourceprogram ������ ������ �Լ�
{
	char *token;
	char awd[100];
	char first[15], second[100], third[150] = "";
	LINE saveLine = { 0 };
	int i=0;
	token = strtok(readLine, "\t\n");	//���� tab ���͸� ��ū���� �Ͽ� �� �ܾ token�� ����
	strcpy(first, token);		//token�� first�� �ӽ� ����
	if (strcmp(first, ".") == 0)	//�ּ��̶�� �ּ� ������ �����Ѵ�.
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
	else		//�ּ��� �ƴ϶��
	{
		while (token != NULL)	//token�� ����������(���� ������)
		{
			//�� �� code�� label, opcode, operand ���¿� ���� �з�
			if (i == 1)		//opcode�ۿ� ���°��(RSUB)
				strcpy(second, token);
			if (i == 2)		//opcode�� operand�ۿ� ���� ���
				strcpy(third, token);
			i++;
			token = strtok(NULL, " .\t\n");
		}
		if (i == 1)	//opcode�ۿ� ���°��(RSUB)
		{
			strcpy(saveLine.label, "\t");
			strcpy(saveLine.opcode, first);
			strcpy(saveLine.operand, "\t");
		}
		else if (i == 2)	//opcode�� operand�ۿ� ���� ���
		{
			strcpy(saveLine.label, "\t");
			strcpy(saveLine.opcode, first);
			strcpy(saveLine.operand, second);
		}
		else if (i == 3)	//label, opcode, operand �� �ִ� ���
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
	//������ �� ���� �ְ�
	if ((obj = fopen("objectProgram.txt", "w")) == NULL)
	{
		fprintf(stderr, "���� ����");
		exit(1);
	}
	if ((list = fopen("listing.txt", "w")) == NULL)
	{
		fprintf(stderr, "���� ����");
		exit(1);
	}
	if ((intermediate = fopen("intermediate.txt", "r")) == NULL)
	{
		fprintf(stderr, "���� ����");
		exit(1);
	}
	fgets(oneLine, 255, intermediate);	//intermeidate ������ �а�
	linecode = divideLine2(oneLine);	//�� ���� label opcode operand locctr�� �и��Ͽ� ������ ����ü�� linecode�� ����
	if (strcmp(linecode.opcode, "START") == 0)		//opcdoe�� start���
	{
		fprintf(obj, "H%-6s%06X%06X\n", linecode.label, linecode.locctr, programlength);		//objectprogram ù�ٿ� opcode�� locctr�� ���α׷��� ũ�⸦ ���ش�.(objectprogram�� ù���� �� �ǹ��̴�.)
		fprintf(list, "%04X\t%s\t%s\t%s\t\n", linecode.locctr, linecode.label, linecode.opcode, linecode.operand);		//listing ������ ù�ٿ��� ���ش�.
	}
	fgets(oneLine, 255, intermediate);		//�� �� �а�
	linecode = divideLine2(oneLine);		//�и�
	while (strcmp(linecode.opcode, "END") != 0)		//opcode END ���ö�����
	{
		if (linecode.label[0] != '.')		//�ּ��̶��
		{
			if (start == true)				//object���α׷����� �� �� �տ��ٰ� �� ���� ���� �ּҸ� ǥ���ϱ� ���� 
			{
				startAddress = linecode.locctr;			//locctr�� ���� �� �տ��ٰ� ���ֱ� ���� startaddress�� ����
				start = false;						//objectprogram�� ���ٿ� locctr�� 60���� ǥ�� ���������� start�� false�� �Ͽ� �����ּҸ� �����ϴ� ���� ����
			}
		}
		if (linecode.label[0] != '.')		//�ּ��� �ƴ϶��
		{
			if (searchOPtab(linecode.opcode) == true)		//opcode�� optable�� �����ϸ�
			{
				lineopcode = OP[indexOP].opcodeNum;		//opcode�� 10������ ǥ���� ���� lineopcode�� ���� indexOP�� searchOPtab���� ������ (��������)
				if (linecode.operand[0] != '\0')		//operand������ 
				{
					if (searchSymtab(linecode.operand) == true)			//operand�� symtab�� �����ϸ�
					{
						lineAddress = SYM[symIndex].address;		//symbol�� ������ �ּҸ� lineAddress�� ����
						if (strcmp(linecode.operand, "BUFFER,X") == 0)		//BUFFER,X�� ��� ������ 9039�� object�ڵ忡 ǥ��
						{
							lineAddress = 36921; //36921 16������ ��ȯ�� 9039�� ��
						}
					}
					else
					{
						lineAddress = 0;		//operand�� symboltable�� ��������������� ����
						errorFlag += 1;
					}
				}
				else
				{
					lineAddress = 0;		
				}
				sprintf(sumAddress, "%02X%04X", lineopcode, lineAddress);		//object�ڵ� ���� 
			}
			else if (strcmp(linecode.opcode, "BYTE") == 0)		//opcode�� BYTE�̸� C�ΰ�찡 �ְ� X�ΰ�찡 �ִ�. 
			{
				if (strcmp(linecode.operand, "C'EOF'") == 0)	//C'EOF'�� ��� objectcode�� 454F46�̴�.
				{
					sprintf(sumAddress, "%s", "454F46");
					sumAddress[6] = NULL;
				}
				else if (linecode.operand[0] == 'X')		//X'���ñ�' �ΰ�� ���ñ⸸ object�ڵ忡 ����.
				{
					sprintf(sumAddress, "%c%c", linecode.operand[2], linecode.operand[3]);
					sumAddress[2] = NULL;
				}
			}
			else if (strcmp(linecode.opcode, "WORD") == 0)		//WORD �ΰ��
			{
				if (strcmp(linecode.label, "MAXLEN") == 0)		//label�� MAXLEN�� ���
				{
					temp = conversionHex(linecode.operand);		//operand�� 16������ �ٲ��ش�(4096�� 16������ ��ȯ�ϸ� 1000�̴�)
					itoa(temp, wordad, 10);		//10������ �ٲ��ش�.
					sprintf(sumAddress, "00%s", wordad);		//001000
				}
				else
					sprintf(sumAddress, "%06s", linecode.operand);		//operand�� �ٷ� ���ش�.(000003),(000000)
			}
			else if (strcmp(linecode.opcode, "RESW") == 0)		//REW�� ��� object�ڵ� x
			{
				sprintf(sumAddress, "%s", "");
			}
			else if (strcmp(linecode.opcode, "RESB") == 0)		//RESB�� ��� obejct�ڵ� x
			{
				sprintf(sumAddress, "%s", "");
			}
			if (strcmp(linecode.label, "\t") == 0 && strcmp(linecode.operand, "\t") == 0)//label�� operand�� ���� ���
			{
				fprintf(list, "%04X\t\t%s\t%s\t%s\n", linecode.locctr, linecode.opcode, linecode.operand, sumAddress);
			}
			else if (strcmp(linecode.label, "\t") == 0)		//label�� ���� ���
			{
				if(strcmp(linecode.operand,"BUFFER,X") == 0)
					fprintf(list, "%04X\t\t%s\t%s\t%s\n", linecode.locctr, linecode.opcode, linecode.operand, sumAddress);
				else
					fprintf(list, "%04X\t\t%s\t%s\t\t%s\n", linecode.locctr, linecode.opcode, linecode.operand, sumAddress);
			}
			else		//label opcode operand ��� �ִ� ���
			{
				fprintf(list, "%04X\t%s\t%s\t%s\t\t%s\n", linecode.locctr, linecode.label, linecode.opcode, linecode.operand, sumAddress);
			}

			strcat(objectLine, sumAddress);		//objectprogram�� ������ object�ڵ带 �ֱ� ���� objectLine�̶�� ���ڿ� ���� ������ sumAddress�� ��� �̾�ٿ��ش�.
			objnum += strlen(sumAddress);		//objectLine�� ���̸� �����ϴ� ojbnum

			if (strcmp(linecode.opcode, "RESB") == 0)		//Byte�� operand��ŭ �߰��ϴ� ���̹Ƿ� objnum�� operand ����ŭ �����ش�.
			{
				objnum += conversionHex(linecode.operand);
			}
			if (strcmp(linecode.opcode, "RESW") == 0)		//WORD�� 3����Ʈ�̹Ƿ�
			{
				objnum += conversionHex(linecode.operand) * 3;
			}

			if (objnum > 55)		//objnum�� 60���� �̹Ƿ�
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
LINE  divideLine2(char* readLine)		//pass2 intermediate�� label,opcode,operand,locctr�� �и�
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
	else		//�ּ��� �ƴ� ���
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
		}			//��ū�� ������ ���� 
		if (strcmp(first, "END") == 0)
		{
			strcpy(saveLine.opcode, first);
			strcpy(saveLine.operand, second);
			return saveLine;
		}
		if (i == 2)		//��ū�� 2���̸�
		{
			strcpy(saveLine.label, "\t");
			strcpy(saveLine.opcode, first);
			strcpy(saveLine.operand, "\t");
			saveLine.locctr = conversionDec(second);
		}
		else if (i == 3)		//3���̸�
		{
			strcpy(saveLine.label, "\t");
			strcpy(saveLine.opcode, first);
			strcpy(saveLine.operand, second);
			saveLine.locctr = conversionDec(third);
		}
		else if (i == 4)		//4���̸�
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
		fprintf(stderr, "���� ����");
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
