// ExtendLastSection.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include<stdio.h>
#include<stdlib.h>
#include<windows.h>

#define test 1
#define ordianl 3
#define write_movreloctable_file_path "D:\\test_dll_02111.dll"
#define write_dllinject_file_path " "
char file_path[] = " ";

//����PE�ļ���С
DWORD ToLoaderPE(LPSTR file_path, PVOID* pFileBuffer)
{
	FILE *pFile = NULL;
	DWORD FileSize = 0;
	PVOID pFileBufferTemp = NULL;

	pFile = fopen(file_path, "rb");

	if (!pFile)
	{
		printf("(ToLoaderPE)Can't open file!\n");
		return 0;
	}

	fseek(pFile, 0, SEEK_END);
	FileSize = ftell(pFile);
	printf("FileBuffer: %#x\n", FileSize);
	fseek(pFile, 0, SEEK_SET);
	pFileBufferTemp = malloc(FileSize);

	if (!pFileBufferTemp)
	{
		printf("(ToLoaderPE)Allocate dynamic memory failed!\n");
		fclose(pFile);
		return 0;
	}

	DWORD n = fread(pFileBufferTemp, FileSize, 1, pFile);

	if (!n)
	{
		printf("(ToLoaderPE)Read file failed!\n");
		free(pFileBufferTemp);
		fclose(pFile);
		return 0;
	}
	*pFileBuffer = pFileBufferTemp;
	pFileBufferTemp = NULL;
	fclose(pFile);
	return FileSize;
}

//����
BOOL MemoryToFile(PVOID pMemBuffer, DWORD size, LPSTR lpszFile)
{
	FILE *fp;
	fp = fopen(lpszFile, "wb");
	if (fp != NULL)
	{
		fwrite(pMemBuffer, size, 1, fp);
	}
	fclose(fp);
	printf("Store file success!\n");
	return 1;
}

//RVA ת FOA
DWORD RvaToFileOffset(PVOID pBuffer, DWORD dwRva)
{
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;

	if (!pBuffer)
	{
		printf("(RvaToFileOffset)Can't open file!\n");
		return 0;
	}

	if (*((PWORD)pBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(RvaToFileOffset)No MZ flag, not exe file!\n");
		return 0;
	}

	pDosHeader = (PIMAGE_DOS_HEADER)pBuffer;
	if (*((PDWORD)((DWORD)pBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(RvaToFileOffset)Not a valid PE flag!\n");
		return 0;
	}

	//printf("ImageOffset: %#x\n", dwRva);
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);

	PIMAGE_SECTION_HEADER pSectionTemp = pSectionHeader;

	if (dwRva <= pOptionHeader->SizeOfHeaders)
	{
		return (DWORD)dwRva;		//��ַ��ͷ�ڣ�����ת��
	}
	else
	{
		for (int n = 0; n < pPEHeader->NumberOfSections; n++, pSectionTemp++)
		{	//�ж� :   �ļ�����+�ļ�ƫ��>file_panyi>�ļ�ƫ��  (�������ļ����ĸ�����)
			if ((dwRva >= pSectionTemp->VirtualAddress) && (dwRva < pSectionTemp->VirtualAddress + pSectionTemp->Misc.VirtualSize))
			{
				return dwRva - pSectionTemp->VirtualAddress + pSectionTemp->PointerToRawData;
			}
		}
	}
	printf("RvaToFoa failed��\n");
	return 0;
}

//FOA ת RVA
DWORD FoaToImageOffset(PVOID pBuffer, DWORD dwFoa)
{
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;

	if (!pBuffer)
	{
		printf("(FoaToImageOffset)Can't open file!\n");
		return 0;
	}

	if (*((PWORD)pBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(FoaToImageOffset)No MZ flag, not exe file!\n");
		return 0;
	}

	pDosHeader = (PIMAGE_DOS_HEADER)pBuffer;
	if (*((PDWORD)((DWORD)pBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(FoaToImageOffset)Not a valid PE flag!\n");
		return 0;
	}
	printf("FileOffset: %#x\n", dwFoa);

	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);

	PIMAGE_SECTION_HEADER pSectionTemp = pSectionHeader;

	if (dwFoa <= pOptionHeader->SizeOfHeaders)
		return (DWORD)dwFoa;
	else
	{
		for (int n = 0; n < pPEHeader->NumberOfSections; n++, pSectionTemp++)
		{	//�ж� :   �ļ�����+�ļ�ƫ��>file_panyi>�ļ�ƫ��  (�������ļ����ĸ�����)
			if ((dwFoa >= pSectionTemp->PointerToRawData) && (dwFoa < pSectionTemp->PointerToRawData + pSectionTemp->SizeOfRawData))
			{
				return dwFoa - pSectionTemp->PointerToRawData + pSectionTemp->VirtualAddress;
			}
		}
	}
	printf("FoaToRva failed��\n");
	return 0;
}

//�ļ�����  alignment_value:δ�����ַ  addend:����ֵ  address:������ַ
DWORD Alignment(DWORD alignment_value, DWORD addend, DWORD address)
{
	int n = 0;
	if (addend / alignment_value)
	{
		if (addend%alignment_value)
		{
			n = addend / alignment_value + 1;
		}
		else
		{
			n = addend / alignment_value;
		}
	}
	else
	{
		if (addend)
			n = 1;
		else
			n = 0;
	}
	address += n * alignment_value;
	return address;
}

//��ȡ���һ���ڵĿ����С
DWORD GetSctionEmptySpace(PVOID pFileBuffer, DWORD SectionOrdinal)
{
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;

	if (!pFileBuffer)
	{
		printf("(GetSctionEmptySpace)Can't open file!\n");
		return 0;
	}

	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(GetSctionEmptySpace)No MZ flag, not exe file!\n");
		return 0;
	}

	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(GetSctionEmptySpace)Not a valid PE flag!\n");
		return 0;
	}

	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);

	if (SectionOrdinal > pPEHeader->NumberOfSections)
	{
		printf("(GetSctionEmptySpace)There are only %#x sections ,no %#x th.\n", pPEHeader->NumberOfSections, SectionOrdinal);
		return 0;
	}

	for (DWORD n = 0; n < SectionOrdinal - 1; n++)
	{
		pSectionHeader++;
	}

	DWORD EmptySpace = pSectionHeader->SizeOfRawData - pSectionHeader->Misc.VirtualSize;

	return EmptySpace;
}

//��ӡ������ִ�гɹ�����1  ʧ�ܷ���0
int LogExportTable(PVOID pFileBuffer)
{
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_EXPORT_DIRECTORY pExportDirectory = NULL;

	if (!pFileBuffer)
	{
		printf("(LogExportTable)Can't open file!\n");
		return 0;
	}

	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(LogExportTable)No MZ flag, not exe file!\n");
		return 0;
	}

	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(LogExportTable)Not a valid PE flag!\n");
		return 0;
	}

	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;

	if (!pDataDirectory->VirtualAddress)
	{
		printf("(LogExportTable)This program has no export table.\n");
		return 0;
	}

	printf("Export Table Rva: %#x\n", pDataDirectory->VirtualAddress);

	//��ȡ������ַ��FOA
	DWORD Foa_ExportTable = RvaToFileOffset(pFileBuffer, pDataDirectory->VirtualAddress);

	printf("Export Table Foa: %#x\n", Foa_ExportTable);

	pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((DWORD)pFileBuffer + Foa_ExportTable);

	//��ӡ������
	printf("Characteristics: %#x\n", pExportDirectory->Characteristics);
	printf("TimeDateStamp: %#x\n", pExportDirectory->TimeDateStamp);
	printf("MajorVersion: %#x\n", pExportDirectory->MajorVersion);
	printf("MinorVersion: %#x\n", pExportDirectory->MinorVersion);
	printf("Name: %s\n", (char*)(PVOID)((DWORD)pFileBuffer + RvaToFileOffset(pFileBuffer, pExportDirectory->Name)));
	printf("Base: %#x\n", pExportDirectory->Base);
	printf("NumberOfFunctions: %#x\n", pExportDirectory->NumberOfFunctions);
	printf("NumberOfNames: %#x\n", pExportDirectory->NumberOfNames);
	printf("AddressOfFunctions: %#x\n", pExportDirectory->AddressOfFunctions);
	printf("AddressOfNames: %#x\n", pExportDirectory->AddressOfNames);
	printf("AddressOfNameOrdinals: %#x\n", pExportDirectory->AddressOfNameOrdinals);
	return 1;
}

//ͨ����Ż�ȡ������ַ
DWORD GetFunctionAddrByOrdinals(PVOID pFileBuffer, DWORD fun_ordinal)
{
	printf("Function Ordinal: %#x\n", fun_ordinal);
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_EXPORT_DIRECTORY pExportDirectory = NULL;

	// �ж�pImageBuffer�Ƿ���Ч
	if (!pFileBuffer)
	{
		printf("(GetFunctionAddrByOrdinals)Can't open file!\n");
		return 0;
	}
	//�ж��ǲ���exe�ļ�
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(GetFunctionAddrByOrdinals)No MZ flag, not exe file!\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(GetFunctionAddrByOrdinals)Not a valid PE flag!\n");
		return 0;
	}

	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;

	if (!pDataDirectory->VirtualAddress)
	{
		printf("This program has no export table.\n");
		return 0;
	}

	printf("Export Table Rva: %#x\n", pDataDirectory->VirtualAddress);

	DWORD Foa_ExportTable = RvaToFileOffset(pFileBuffer, pDataDirectory->VirtualAddress);

	printf("Export Table Foa: %#x\n", Foa_ExportTable);

	pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((DWORD)pFileBuffer + Foa_ExportTable);

	if (!test)// test����
	{
		printf("Characteristics: %#x\n", pExportDirectory->Characteristics);
		printf("TimeDateStamp:%#x\n", pExportDirectory->TimeDateStamp);
		printf("MajorVersion: %#x\n", pExportDirectory->MajorVersion);
		printf("MinorVersion: %#x\n", pExportDirectory->MinorVersion);
		printf("Name: %s\n", (char *)(PVOID)((DWORD)pFileBuffer + RvaToFileOffset(pFileBuffer, pExportDirectory->Name)));
		printf("Base: %#x\n", pExportDirectory->Base);
		printf("NumberOfFunctions: %#x\n", pExportDirectory->NumberOfFunctions);
		printf("NumberOfNames: %#x\n", pExportDirectory->NumberOfNames);
		printf("AddressOfFunctions: %#x\n", pExportDirectory->AddressOfFunctions);
		printf("AddressOfNames: %#x\n", pExportDirectory->AddressOfNames);
		printf("AddressOfNameOrdinals: %#x\n", pExportDirectory->AddressOfNameOrdinals);
	}

	DWORD Sequence = fun_ordinal - pExportDirectory->Base;
	DWORD Foa_AddressOfFunctions = RvaToFileOffset(pFileBuffer, pExportDirectory->AddressOfFunctions);

	if (!test)// test����
	{
		DWORD test1 = Foa_AddressOfFunctions + (DWORD)pFileBuffer;
		printf("AddressOfFunctions in this moment: %#x\n", test1);
		printf("Foa_AddressOfFunctions: %#x\n", Foa_AddressOfFunctions);
	}

	//���ļ���ָ�򵼳���ĺ�����ַ����ʼ��ָ��
	PDWORD pFoa_AddressOfFunctions = (PDWORD)(PBYTE)(Foa_AddressOfFunctions + (DWORD)pFileBuffer);

	//��������ں�����ַ�����
	for (DWORD n = 0; n < Sequence; n++)
	{
		pFoa_AddressOfFunctions++;
	}
	//DWORD Foa_AddrFun = RvaToFileOffset(pFileBuffer, *pFoa_AddressOfFunctions);

	return *pFoa_AddressOfFunctions;
}

//ͨ�����ֻ�ȡ������ַ
DWORD GetFunctionAddrByName(PVOID pFileBuffer, LPSTR fun_name)
{
	printf("Function Name: %s\n", fun_name);
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_EXPORT_DIRECTORY pExportDirectory = NULL;

	// �ж�pImageBuffer�Ƿ���Ч
	if (!pFileBuffer)
	{
		printf("(GetFunctionAddrByName)Can't open file!\n");
		return 0;
	}
	//�ж��ǲ���exe�ļ�
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(GetFunctionAddrByName)No MZ flag, not exe file!\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(GetFunctionAddrByName)Not a valid PE flag!\n");
		return 0;
	}

	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;

	if (!pDataDirectory->VirtualAddress)
	{
		printf("This program has no export table.\n");
		return 0;
	}

	printf("Export Table Rva: %#x\n", pDataDirectory->VirtualAddress);
	DWORD Foa_ExportTable = RvaToFileOffset(pFileBuffer, pDataDirectory->VirtualAddress);
	printf("Export Table Foa: %#x\n", Foa_ExportTable);
	pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((DWORD)pFileBuffer + Foa_ExportTable);

	if (!test)
	{
		printf("Characteristics: %#x\n", pExportDirectory->Characteristics);
		printf("TimeDateStamp: %#x\n", pExportDirectory->TimeDateStamp);
		printf("MajorVersion: %#x\n", pExportDirectory->MajorVersion);
		printf("MinorVersion: %#x\n", pExportDirectory->MinorVersion);
		printf("Name: %s\n", (char*)(PVOID)((DWORD)pFileBuffer + RvaToFileOffset(pFileBuffer, pExportDirectory->Name)));
		printf("Base: %#x\n", pExportDirectory->Base);
		printf("NumberOfFunctions: %#x\n", pExportDirectory->NumberOfFunctions);
		printf("NumberOfNames: %#x\n", pExportDirectory->NumberOfNames);
		printf("AddressOfFunctions: %#x\n", pExportDirectory->AddressOfFunctions);
		printf("AddressOfNames: %#x\n", pExportDirectory->AddressOfNames);
		printf("AddressOfNameOrdinals: %#x\n", pExportDirectory->AddressOfNameOrdinals);
	}

	DWORD Foa_AddressOfNames = RvaToFileOffset(pFileBuffer, pExportDirectory->AddressOfNames);
	DWORD Foa_AddressOfNameOrdinals = RvaToFileOffset(pFileBuffer, pExportDirectory->AddressOfNameOrdinals);
	DWORD Foa_AddressOfFunctions = RvaToFileOffset(pFileBuffer, pExportDirectory->AddressOfFunctions);

	if (!test)
	{
		DWORD test1 = Foa_AddressOfNames + (DWORD)pFileBuffer;
		printf("AddressOfNames in this moment: %#x\n", test1);
		printf("Foa__AddressOfNames: %#x\n", Foa_AddressOfNames);
	}

	//1.ѭ�������ֱ�������Ŀ�꺯������ͬ�ģ����з��ظ������ڱ��е�����
	DWORD ordIndex = -1;
	for (DWORD i = 0; i < pExportDirectory->NumberOfNames; i++)
	{
		DWORD nameOffset = *(PDWORD)((DWORD)pFileBuffer + (DWORD)((LPDWORD)Foa_AddressOfNames + i));
		LPSTR nameAddr = (LPSTR)((DWORD)pFileBuffer + RvaToFileOffset(pFileBuffer, nameOffset));
		if (!strcmp(nameAddr, fun_name))
		{
			ordIndex = i;
			break;
		}
	}
	if (ordIndex == -1)
	{
		printf("The export table does not have this function name.\n");
		return 0;
	}

	//2.�û�õ���������ű����Һ��������
	WORD ord = *(PWORD)((DWORD)pFileBuffer + (DWORD)((LPWORD)Foa_AddressOfNameOrdinals + ordIndex));

	if (!test)
	{
		DWORD test1 = Foa_AddressOfNameOrdinals + (DWORD)pFileBuffer;
		printf("AddressOfNameOrdinals in this moment: %#x\n", test1);
		printf("Foa__AddressOfNameOrdinals: %#x\n", Foa_AddressOfNameOrdinals);
		printf("ordInex in AddressOfNames: %#x\n", ordIndex);
		printf("ordInex in AddressOfNameOrdinals: %#x\n", ord);
	}

	//3.����ű��в���������Ϊ�����Ӻ�����ַ�����Һ�����ַ
	DWORD addr = *(PDWORD)((DWORD)pFileBuffer + (DWORD)((LPDWORD)Foa_AddressOfFunctions + ord));
	//DWORD Foa_AddrFun = RvaToFileOffset(pFileBuffer, addr);

	return addr;
}

//��ӡͷ��Ϣ
int LogPEHeaderInfo(PVOID pFileBuffer)
{
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;

	if (!pFileBuffer)
	{
		printf("(LogPEHeaderInfo)Can't open file!\n");
		return 0;
	}

	//�ж��Ƿ�����Ч��MZ��־	
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(LogPEHeaderInfo)No MZ flag, not exe file!\n");
		free(pFileBuffer);
		return 0;
	}
	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	//��ӡDOCͷ	
	printf("\n********************DOSͷ********************\n");
	printf("MZ flag��%#x\n", pDosHeader->e_magic);
	printf("PE offset��%#x\n", pDosHeader->e_lfanew);
	//�ж��Ƿ�����Ч��PE��־	
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(LogPEHeaderInfo)Not a valid PE flag!\n");
		free(pFileBuffer);
		return 0;
	}
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer + pDosHeader->e_lfanew);
	//��ӡNTͷ	
	printf("********************NTͷ********************\n");
	printf("NT��%#x\n", pNTHeader->Signature);
	pPEHeader = (PIMAGE_FILE_HEADER)(((DWORD)pNTHeader) + 4);

	//int NumberOfSections = pPEHeader->NumberOfSections;
	//int SizeOfOptionalHeader = pPEHeader->SizeOfOptionalHeader;

	printf("********************PEͷ********************\n");
	printf("PE��%#x\n", pPEHeader->Machine);
	printf("Section number��%#x\n", pPEHeader->NumberOfSections);
	printf("SizeOfOptionalHeader��%#x\n", pPEHeader->SizeOfOptionalHeader);
	//��ѡPEͷ	
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	printf("********************OPTIOIN_PEͷ********************\n");
	printf("OPTION_PE��%#x\n", pOptionHeader->Magic);

	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);

	for (int n = 0; n < pPEHeader->NumberOfSections; n++)
	{
		printf("********************SECTION %d********************\n", n + 1);
		char name[9] = { 0 };
		memcpy(name, pSectionHeader->Name, 8);
		printf("Section Name: %s\n", name);
		printf("VirtualAddress: %#x\n", pSectionHeader->VirtualAddress);
		printf("SizeOfRawData: %#x\n", pSectionHeader->SizeOfRawData);
		printf("PointerToRawData: %#x\n", pSectionHeader->PointerToRawData);
		printf("Characteristics: %#x\n", pSectionHeader->Characteristics);
		pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pSectionHeader + IMAGE_SIZEOF_SECTION_HEADER);
	}
	return 1;
}

//��ӡ�ض�λ��
int LogBaseRelocationTable(PVOID pFileBuffer)
{
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_BASE_RELOCATION pRelocationTable = NULL;

	if (!pFileBuffer)
	{
		printf("(LogBaseRelocationTable)Can't open file!\n");
		return 0;
	}

	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(LogBaseRelocationTable)No MZ flag, not exe file!\n");
		return 0;
	}

	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(LogBaseRelocationTable)Not a valid PE flag!\n");
		return 0;
	}

	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;

	PIMAGE_DATA_DIRECTORY pDataDirectory_RelocTable = &pDataDirectory[5];

	if (!pDataDirectory_RelocTable->VirtualAddress)
	{
		printf("This program has no relocation table.\n");
		return 0;
	}

	DWORD Foa_RelocationTable = RvaToFileOffset(pFileBuffer, pDataDirectory_RelocTable->VirtualAddress);

	if (!test)
	{
		printf("Relocation Table Rva: %#x\n", pDataDirectory_RelocTable->VirtualAddress);
		printf("Relocation Table Foa: %#x\n", Foa_RelocationTable);
		printf("pFileBuffer: %#x\n", (DWORD)pFileBuffer);
		printf("Relocation Table in this moment: %#x\n", (DWORD)pFileBuffer + Foa_RelocationTable);
	}

	pRelocationTable = (PIMAGE_BASE_RELOCATION)(PIMAGE_EXPORT_DIRECTORY)((DWORD)pFileBuffer + Foa_RelocationTable);
	printf("pRelocationTable: %#x\n", (DWORD)pRelocationTable);

	for (DWORD n = 0; pRelocationTable->VirtualAddress != IMAGE_REL_BASED_ABSOLUTE; n++)
	{
		printf(".....................RelocationTable %#5xth.................................\n", n + 1);
		DWORD arrNumber = (pRelocationTable->SizeOfBlock - 8) / 2;  //��ǰ�ض�λ����ض�λ��Ŀ

		printf("TypeOffset Number: %#x\n", arrNumber);

		//PDWORD pTypeOffsetOrg = ((PDWORD)pRelocationTable + 2);
		//PWORD pTypeOffset = (PWORD)pTypeOffsetOrg;

		PWORD pTypeOffset = (PWORD)((PDWORD)pRelocationTable + 2);

		for (DWORD i = 0; i < arrNumber; i++)
		{
			//��ӡָ��ÿ�������ض�λ��ĵ�ַ
			printf("pTypeOffset: %#-10x", (WORD)pTypeOffset);   
			WORD TypeOffset = *pTypeOffset;  //ȡ����ַ���ֵ

			if (!test)
			{
				printf("TypeOffset(011) = %#x\n", TypeOffset);
			}
			//ȡ��16λ�ض�λ�ĸ�4λ
			BYTE attribute = (TypeOffset & 0b11000000000000) >> 12;

			//��12λ
			TypeOffset = (TypeOffset & 0b111111111111);
			if (!test)
			{
				printf("TypeOffset(000) = %#x\n", TypeOffset);
			}
			printf("Attribute: %-5x", attribute);
			DWORD Offset = pRelocationTable->VirtualAddress + (DWORD)TypeOffset;
			printf("Rva_BaseRelocation: %#-10x", Offset);
			printf("Foa_BaseRelocation: %#-10x\n", RvaToFileOffset(pFileBuffer, Offset));
			pTypeOffset++;
		}
		pRelocationTable = (PIMAGE_BASE_RELOCATION)(PDWORD)((DWORD)pRelocationTable + pRelocationTable->SizeOfBlock);
	}
	return 0;
}

//��ӡ�����
int LogImportTable(PVOID pFileBuffer)
{
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory_ImportTable = NULL;
	PIMAGE_IMPORT_DESCRIPTOR pImportTable = NULL;
	PIMAGE_IMPORT_BY_NAME  pImportByName = NULL;
	PIMAGE_THUNK_DATA pOriginalFirstThunk = NULL;
	PIMAGE_THUNK_DATA pFirstThunk = NULL;

	if (!pFileBuffer)
	{
		printf("(LogImportTable)Can't open file!\n");
		return 0;
	}

	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(LogImportTable)No MZ flag, not exe file!\n");
		return 0;
	}

	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(LogImportTable)Not a valid PE flag!\n");
		return 0;
	}

	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;
	pDataDirectory_ImportTable = &pDataDirectory[1];

	if (!pDataDirectory_ImportTable->VirtualAddress)
	{
		printf("(LogImportTable)This program has no import table.\n");
		return 0;
	}

	//������ַת��
	printf("Import Table Rva: %#x\n", pDataDirectory_ImportTable->VirtualAddress);
	DWORD Foa_ImportTable = RvaToFileOffset(pFileBuffer, pDataDirectory_ImportTable->VirtualAddress);
	printf("Import Table Foa: %#x\n", Foa_ImportTable);
	pImportTable = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)pFileBuffer + Foa_ImportTable);

	//����dllģ��
	while (pImportTable->Name != 0)
	{
		//��ӡdllģ����
		DWORD Foa_DllName = RvaToFileOffset(pFileBuffer, pImportTable->Name);
		PDWORD Foa_pDllName = (PDWORD)((DWORD)pFileBuffer + Foa_DllName);
		printf("%s\n", Foa_pDllName);
		//��ӡOriginalFirstThunk
		DWORD Foa_OrginalFirstThunkAddr = RvaToFileOffset(pFileBuffer, pImportTable->OriginalFirstThunk);
		PDWORD Foa_pOrginalFirstThunkAddr = (PDWORD)((DWORD)pFileBuffer + Foa_OrginalFirstThunkAddr);
		printf("-------------------Foa_OrginalFirstThunkAddr:%#-10x -----------------------\n", Foa_OrginalFirstThunkAddr);
		pOriginalFirstThunk = (PIMAGE_THUNK_DATA)Foa_pOrginalFirstThunkAddr;
		//while (strcmp(pOriginalFirstThunk,"0"))
		while (*(PDWORD)pOriginalFirstThunk)
		{
			DWORD value = *(PDWORD)pOriginalFirstThunk;
			DWORD judge = (value & IMAGE_ORDINAL_FLAG32) >> 31;
			//���λΪ1������ŷ�ʽ���룻���λΪ0���Ժ�������ʽ����
			if (judge)
			{
				//�������  ȡ��31λ
				value -= IMAGE_ORDINAL_FLAG32;
				printf("Import Ordinal  :%#-4x\n", value);
			}
			else
			{
				//��������
				DWORD Foa_ImportByNameAddr = RvaToFileOffset(pFileBuffer, value);
				PDWORD Foa_pImportByNameAddr = (PDWORD)((DWORD)pFileBuffer + Foa_ImportByNameAddr);
				pImportByName = (PIMAGE_IMPORT_BY_NAME)Foa_pImportByNameAddr;
				printf("Import Hint\\Name:%#-4x ,%s\n", pImportByName->Hint, pImportByName->Name);
			}
			pOriginalFirstThunk++;
		}
		//��ӡFirstThunk
		DWORD Foa_FirstThunkAddr = RvaToFileOffset(pFileBuffer, pImportTable->FirstThunk);
		PDWORD Foa_pFirstThunkAddr = (PDWORD)((DWORD)pFileBuffer + Foa_FirstThunkAddr);
		printf("-------------------Foa_FirstThunkAddr:%#-17x -----------------------\n", Foa_FirstThunkAddr);
		pFirstThunk = (PIMAGE_THUNK_DATA)Foa_pFirstThunkAddr;
		while (*(PDWORD)pFirstThunk)
		{
			DWORD value = *(PDWORD)pFirstThunk;
			DWORD judge = (value & IMAGE_ORDINAL_FLAG32) >> 31;
			//���λΪ1������ŷ�ʽ���룻���λΪ0���Ժ�������ʽ����
			if (judge)
			{
				//�������
				value -= IMAGE_ORDINAL_FLAG32;
				printf("Import Ordinal  :%#-4x\n", value);
			}
			else
			{
				//��������
				DWORD Foa_ImportByNameAddr = RvaToFileOffset(pFileBuffer, value);
				PDWORD Foa_pImportByNameAddr = (PDWORD)((DWORD)pFileBuffer + Foa_ImportByNameAddr);
				pImportByName = (PIMAGE_IMPORT_BY_NAME)Foa_pImportByNameAddr;
				printf("Import Hint\\Name:%#-4x ,%s\n", pImportByName->Hint, pImportByName->Name);
			}
			pFirstThunk++;
		}
		printf("\n");
		pImportTable++;
	}
	return 0;
}

//��ӡ�󶨵����
int LogBoundImportTable(PVOID pFileBuffer)
{
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory_BoundImportTable = NULL;
	PIMAGE_BOUND_IMPORT_DESCRIPTOR pBoundImportTable = NULL;

	if (!pFileBuffer)
	{
		printf("(LogBoundImportTable)Can't open file!\n");
		return 0;
	}

	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(LogBoundImportTable)No MZ flag, not exe file!\n");
		return 0;
	}

	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(LogBoundImportTable)Not a valid PE flag!\n");
		return 0;
	}

	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;
	pDataDirectory_BoundImportTable = pDataDirectory + 11;

	if (!pDataDirectory_BoundImportTable->VirtualAddress)
	{
		printf("(LogBoundImportTable)This program has no bound import table.\n");
		return 0;
	}

	printf("Bound Import Table Rva: %#x\n", pDataDirectory_BoundImportTable->VirtualAddress);
	DWORD Foa_ImportTable = RvaToFileOffset(pFileBuffer, pDataDirectory_BoundImportTable->VirtualAddress);
	printf("Bound Import Table Foa: %#x\n", Foa_ImportTable);
	pBoundImportTable = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)(PIMAGE_IMPORT_DESCRIPTOR)((DWORD)pFileBuffer + Foa_ImportTable);

	DWORD BoundImportTableBase = (DWORD)pBoundImportTable;

	while (*(PDWORD)pBoundImportTable)
	{
		PDWORD pOffsetModuleName = (PDWORD)(BoundImportTableBase + pBoundImportTable->OffsetModuleName);
		printf("Module Name :%s\n", pOffsetModuleName);
		pBoundImportTable++;
	}
	return 0;
}

//���㵼����Ĵ�С��������ڶ���
DWORD SizeOfExportTableSection(PVOID pFileBuffer)
{
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_EXPORT_DIRECTORY pExportDirectory = NULL;
	DWORD SizeOfExportTableSectionTotal = 0;

	if (!pFileBuffer)
	{
		printf("(SizeOfExportTableSection)Can't open file!\n");
		return 0;
	}

	if (*((PWORD)(pFileBuffer)) != IMAGE_DOS_SIGNATURE)
	{
		printf("(SizeOfExportTableSection)No MZ flag, not exe file!\n");
		return 0;
	}

	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(SizeOfExportTableSection)Not a valid PE flag!\n");
		return 0;
	}

	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;

	if (!pDataDirectory->VirtualAddress)
	{
		printf("This program has no export table.\n");
		return 0;
	}

	DWORD Foa_ExportTable = RvaToFileOffset(pFileBuffer, pDataDirectory->VirtualAddress);

	if (!test)
	{
		printf("Export Table Rva: %#x\n", pDataDirectory->VirtualAddress);
		printf("Export Table Foa: %#x\n", Foa_ExportTable);
	}

	//���ļ���ַ����
	pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((DWORD)pFileBuffer + Foa_ExportTable);

	if (!test)
	{
		printf("Characteristics: %#x\n", pExportDirectory->Characteristics);
		printf("TimeDateStamp: %#x\n", pExportDirectory->TimeDateStamp);
		printf("MajorVersion: %#x\n", pExportDirectory->MajorVersion);
		printf("MinorVersion: %#x\n", pExportDirectory->MinorVersion);
		printf("Name: %s\n", (PVOID)((DWORD)pFileBuffer + RvaToFileOffset(pFileBuffer, pExportDirectory->Name)));
		printf("Base: %#x\n", pExportDirectory->Base);
		printf("NumberOfFunctions: %#x\n", pExportDirectory->NumberOfFunctions);
		printf("NumberOfNames: %#x\n", pExportDirectory->NumberOfNames);
		printf("AddressOfFunctions: %#x\n", pExportDirectory->AddressOfFunctions);
		printf("AddressOfNames: %#x\n", pExportDirectory->AddressOfNames);
		printf("AddressOfNameOrdinals: %#x\n", pExportDirectory->AddressOfNameOrdinals);
	}

	//size�����40�ֽ�
	SizeOfExportTableSectionTotal += 40;
	//size AddressOfFunctions
	SizeOfExportTableSectionTotal += (4 * pExportDirectory->NumberOfFunctions);
	//size AddressOfNames
	SizeOfExportTableSectionTotal += (4 * pExportDirectory->NumberOfNames);
	//size AddressOfNameOrdinals
	SizeOfExportTableSectionTotal += (2 * pExportDirectory->NumberOfNames);

	//size Function Name string in AddressOfNames  ���ֱ����ʼ��ַ�����������ַ�����ռ��С
	DWORD Foa_AddressOfNames = RvaToFileOffset(pFileBuffer, pExportDirectory->AddressOfNames);

	if (!test)
	{
		DWORD test1 = Foa_AddressOfNames + (DWORD)pFileBuffer;
		printf("AddressOfNames in this moment: %#x\n", test1);
		printf("Foa__AddressOfNames: %#x\n", Foa_AddressOfNames);
	}

	DWORD namestringSizeTotal = 0;
	for (DWORD i = 0; i < pExportDirectory->NumberOfNames; i++)
	{
		//��ȡ�ַ����ڴ�ƫ��
		DWORD nameOffset = *(PDWORD)((DWORD)pFileBuffer + (DWORD)((LPDWORD)Foa_AddressOfNames + i));
		//ת�����ļ�ƫ��
		LPSTR nameAddr = (LPSTR)((DWORD)pFileBuffer + RvaToFileOffset(pFileBuffer, nameOffset));
		//strlen�õ�ÿ���������ĳ��ȣ�strlen������"\0",���+1
		DWORD namestringSize = strlen(nameAddr) + 1;
		namestringSizeTotal += namestringSize;
	}
	SizeOfExportTableSectionTotal += namestringSizeTotal;

	printf("SizeOfExportTableSectionTotal: %#x\n", SizeOfExportTableSectionTotal);

	//�ļ��ڶ���
	SizeOfExportTableSectionTotal = Alignment(pOptionHeader->SectionAlignment, SizeOfExportTableSectionTotal, 0);

	return SizeOfExportTableSectionTotal;
}

/*�ƶ����������ƶ��벻���ƶ���
*1*����һ����
*2*����3����
*3*���ƺ�����
*4*�޸�
*/
DWORD MoveExportTable(PVOID pFileBuffer, DWORD SizeFileBuffer, PVOID* pFileBuffer_ExportTable)
{
	//δ����pMoveExportTableTemp��������pFileBuffer�õ�pOptionHeaderFileBuffer->SectionAlignment
	PIMAGE_DOS_HEADER pDosHeaderFileBuffer = NULL;
	PIMAGE_NT_HEADERS pNTHeaderFileBuffer = NULL;
	PIMAGE_FILE_HEADER pPEHeaderFileBuffer = NULL;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeaderFileBuffer = NULL;
	pDosHeaderFileBuffer = (PIMAGE_DOS_HEADER)pFileBuffer;
	pNTHeaderFileBuffer = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer + pDosHeaderFileBuffer->e_lfanew);
	pPEHeaderFileBuffer = (PIMAGE_FILE_HEADER)(((DWORD)pNTHeaderFileBuffer) + 4);
	pOptionHeaderFileBuffer = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeaderFileBuffer + IMAGE_SIZEOF_FILE_HEADER);

	LPVOID pMoveExportTableTemp = NULL;
	DWORD SizeExportTableSection = SizeOfExportTableSection(pFileBuffer);
	size_t AddSecTotal = SizeFileBuffer + SizeExportTableSection;

	if (!test)
	{
		printf("SizeExportTableSection: %#x\n", SizeExportTableSection);
		printf("pFileBuffer: %#x\n", SizeFileBuffer);
		printf("pOptionHeaderFileBuffer->SizeOfImage: %#x\n", pOptionHeaderFileBuffer->SizeOfImage);
		printf("pFileBufferAddSecTotal: %#x\n", AddSecTotal);
		printf("NTHeader->OptionalHeader.SizeOfImage + SizeOfSection: %#x\n", pOptionHeaderFileBuffer->SizeOfImage + SizeExportTableSection);
	}

	//�ڴ����
	AddSecTotal = Alignment(pOptionHeaderFileBuffer->SectionAlignment, AddSecTotal, 0);

	if (!test)
	{
		printf("pFileBuffer: %#x\n", SizeFileBuffer);
		printf("pFileBufferAddSecTotal: %#x\n", AddSecTotal);
	}

	pMoveExportTableTemp = malloc(AddSecTotal);

	if (!pMoveExportTableTemp)
	{
		printf("(MoveExportTable)Allocate dynamic memory failed!\n");
		return 0;
	}

	memset(pMoveExportTableTemp, 0, AddSecTotal);
	memcpy(pMoveExportTableTemp, pFileBuffer, SizeFileBuffer);

	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_EXPORT_DIRECTORY pExportDirectory = NULL;

	if (!pMoveExportTableTemp)
	{
		printf("(MoveExportTable)Can't open file!\n");
		return 0;
	}

	if (*((PWORD)pMoveExportTableTemp) != IMAGE_DOS_SIGNATURE)
	{
		printf("(MoveExportTable)No MZ flag, not exe file!\n");
		free(pMoveExportTableTemp);
		return 0;
	}
	pDosHeader = (PIMAGE_DOS_HEADER)pMoveExportTableTemp;

	if (*((PDWORD)((DWORD)pMoveExportTableTemp + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(MoveExportTable)Not a valid PE flag!\n");
		free(pMoveExportTableTemp);
		return 0;
	}
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pMoveExportTableTemp + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)(((DWORD)pNTHeader) + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;

	PIMAGE_SECTION_HEADER pSectionHeaderTemp = pSectionHeader;

	for (DWORD i = 0; i < pPEHeader->NumberOfSections; i++, pSectionHeaderTemp++)
	{
		//ѭ���ҵ�ָ���ĩβ��ָ��
	}

	//�ӽ�
	int size_surplus_sizeofheader = 0x50;	//һ���ڼ�һ���հ׽�
	if (size_surplus_sizeofheader <= (pOptionHeader->SizeOfHeaders - ((DWORD)(pSectionHeaderTemp - pMoveExportTableTemp))))
	{
		//printf("Enough.\n");
		//�õ����һ���ڵ���Ϣ
		pSectionHeaderTemp--;
		PIMAGE_SECTION_HEADER pLastSectionHeader = pSectionHeaderTemp;

		//����	
		//memset(((PBYTE)(DWORD)pMoveExportTableTemp + ret_loc3), 0, SizeOfExportTableSection(&pFileBuffer));
		//�Ľ���Ŀ
		pPEHeader->NumberOfSections = pPEHeader->NumberOfSections + 1;
		//���ڱ�
		pSectionHeaderTemp++;
		memcpy(pSectionHeaderTemp, pSectionHeader, IMAGE_SIZEOF_SECTION_HEADER);
		memcpy(pSectionHeaderTemp, ".movExp", 8);

		pSectionHeaderTemp->VirtualAddress = pLastSectionHeader->VirtualAddress + pLastSectionHeader->Misc.VirtualSize;
		pSectionHeaderTemp->SizeOfRawData = SizeExportTableSection;
		pSectionHeaderTemp->PointerToRawData = pLastSectionHeader->PointerToRawData + pLastSectionHeader->SizeOfRawData;
		pSectionHeaderTemp->Misc.VirtualSize = SizeExportTableSection;
		pOptionHeader->SizeOfImage = pOptionHeaderFileBuffer->SizeOfImage + SizeExportTableSection;
		//����
		pSectionHeaderTemp->VirtualAddress = Alignment(pOptionHeaderFileBuffer->SectionAlignment, pSectionHeaderTemp->VirtualAddress, 0);
		pSectionHeaderTemp->PointerToRawData = Alignment(pOptionHeaderFileBuffer->FileAlignment, pSectionHeaderTemp->PointerToRawData, 0);
	}
	else
	{
		free(pMoveExportTableTemp);
		printf("Insufficient.\n");
	}

	if (!pDataDirectory->VirtualAddress)
	{
		printf("This program has no export table.\n");
		return 0;
	}

	printf("Export Table Rva: %#x\n", pDataDirectory->VirtualAddress);

	DWORD Foa_ExportTable = RvaToFileOffset(pMoveExportTableTemp, pDataDirectory->VirtualAddress);
	printf("Export Table Foa: %#x\n", Foa_ExportTable);

	pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((DWORD)pMoveExportTableTemp + Foa_ExportTable);
	PVOID pAddInsSction = (PDWORD)((DWORD)pMoveExportTableTemp + pSectionHeaderTemp->PointerToRawData);

	if (!test)
	{
		printf("The starting address of the new section��%#x\n", ((DWORD)pMoveExportTableTemp + pSectionHeaderTemp->PointerToRawData));
	}
	PDWORD pAddressOfFunctions = (PDWORD)((DWORD)pMoveExportTableTemp + RvaToFileOffset(pMoveExportTableTemp, pExportDirectory->AddressOfFunctions));

	printf("AddressOfFunctions: %#x\n", pExportDirectory->AddressOfFunctions);
	DWORD NewAddressOfFunctions = (DWORD)pAddInsSction;
	//���ƺ�����ַ��
	memcpy(pAddInsSction, pAddressOfFunctions, 4 * pExportDirectory->NumberOfFunctions);
	//ָ�����
	pAddInsSction = (PWORD)((DWORD)pAddInsSction + 4 * pExportDirectory->NumberOfFunctions);

	PDWORD pAddressOfNameOrdinals = (PDWORD)((DWORD)pMoveExportTableTemp + RvaToFileOffset(pMoveExportTableTemp, pExportDirectory->AddressOfNameOrdinals));
	DWORD NewAddressOfNameOrdinals = (DWORD)pAddInsSction;
	//������ű�
	memcpy(pAddInsSction, pAddressOfNameOrdinals, 2 * pExportDirectory->NumberOfNames);
	//ָ�����
	pAddInsSction = (PDWORD)((DWORD)pAddInsSction + 2 * pExportDirectory->NumberOfNames);

	PDWORD pAddressOfNames = (PDWORD)((DWORD)pMoveExportTableTemp + RvaToFileOffset(pMoveExportTableTemp, pExportDirectory->AddressOfNames));
	PDWORD pAddressOfNamesOrg = pAddressOfNames;
	if (!test)
	{
		printf("pAddressOfNamesOrg: %#x\n", *pAddressOfNamesOrg);
		printf("%#x\n", *pAddressOfNamesOrg);
	}

	DWORD NewAddressOfNames = (DWORD)pAddInsSction;
	//���ƺ�������
	memcpy(pAddInsSction, pAddressOfNames, 4 * pExportDirectory->NumberOfNames);
	//ָ�����
	pAddInsSction = (PDWORD)((DWORD)pAddInsSction + 4 * pExportDirectory->NumberOfNames);

	//���ƺ����ַ���
	for (DWORD n = 0; n < pExportDirectory->NumberOfNames; n++)
	{
		PVOID pAddressNameString = (PVOID)*pAddressOfNames;
		DWORD nameSize = strlen((CHAR*)((DWORD)pMoveExportTableTemp + RvaToFileOffset(pMoveExportTableTemp, *(PDWORD)pAddressNameString)));

		*pAddressOfNames = RvaToFileOffset(pMoveExportTableTemp, *(PDWORD)pAddressNameString);

		if (!test)
		{
			printf("pAddressNameString: %#x\n", (DWORD)pAddressNameString);
			printf("&pAddressOfNames: %#x\n", &pAddressOfNames);
			printf("pAddressOfNames: %#x\n", pAddressOfNames);
			printf("*pAddressOfNames: %#x\n", *pAddressOfNames);
		}
		//�����ַ���
		memcpy(pAddInsSction, (PVOID)((DWORD)pMoveExportTableTemp + RvaToFileOffset(pMoveExportTableTemp, *(PDWORD)pAddressNameString)), nameSize);
		//�޸�AddressOfNames
		*(PDWORD)NewAddressOfNames = FoaToImageOffset(pMoveExportTableTemp, (DWORD)pAddInsSction - (DWORD)pMoveExportTableTemp);

		printf("pAddInsSction: %#x\n", (DWORD)pAddInsSction);
		
		//ָ�����
		pAddInsSction = (PDWORD)((DWORD)pAddInsSction + nameSize);

		//�ַ������"\0"������
		memcpy(pAddInsSction, "\0", 1);
		pAddInsSction = (PDWORD)((DWORD)pAddInsSction + 1);
		pAddressOfNames++;

		//((PDWORD)NewAddressOfNames)++;			����ͨ��������������
		NewAddressOfNames = *((PDWORD)NewAddressOfNames + 1);
	}
	DWORD NewExportDirectory = (DWORD)pAddInsSction;
	memcpy(pAddInsSction, pExportDirectory, 0x28);
	PIMAGE_EXPORT_DIRECTORY pAddInsSctionTemp = (PIMAGE_EXPORT_DIRECTORY)pAddInsSction;

	//�޸ĵ�������Ϣ
	if (!test)
	{
		printf("pMoveExportTableTemp: %#x\n", (DWORD)pMoveExportTableTemp);
		printf("pSectionHeaderTemp->PointerToRawData: %#x\n", pSectionHeaderTemp->PointerToRawData);
		printf("pSectionHeaderTemp->VirtualAddress: %#x\n", pSectionHeaderTemp->VirtualAddress);
	}
	pAddInsSctionTemp->AddressOfFunctions = FoaToImageOffset(pMoveExportTableTemp, NewAddressOfFunctions - (DWORD)pMoveExportTableTemp);
	pAddInsSctionTemp->AddressOfNames = FoaToImageOffset(pMoveExportTableTemp, NewAddressOfNames - (DWORD)pMoveExportTableTemp);
	pAddInsSctionTemp->AddressOfNameOrdinals = FoaToImageOffset(pMoveExportTableTemp, NewAddressOfNameOrdinals - (DWORD)pMoveExportTableTemp);

	//�޸�����Ŀ¼����Ϣ
	pDataDirectory->VirtualAddress = FoaToImageOffset(pMoveExportTableTemp, NewExportDirectory - (DWORD)pMoveExportTableTemp);

	/*size_t ret_loc5 = MemoryToFile(pMoveExportTableTemp, AddSecTotal, write_movexportable_file_path);
	if (!ret_loc5)
	{
		printf("(MoveExportTable)Store memory failed.\n");
		return 0;
	}*/

	*pFileBuffer_ExportTable = pMoveExportTableTemp; //�ݴ�����ݴ����������ͷ�
	pMoveExportTableTemp = NULL;

	return AddSecTotal;
}

//�����ض�λ��С��������ڶ���
DWORD SizeOfBaseRelocationTable(PVOID pFileBuffer)
{
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_BASE_RELOCATION pRelocationTable = NULL;
	DWORD SizeOfBaseRelocationTableSectionTotal = 0;

	if (!pFileBuffer)
	{
		printf("(SizeOfBaseRelocationTable)Can't open file!\n");
		return 0;
	}

	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(SizeOfBaseRelocationTable))No MZ flag, not exe file!\n");
		return 0;
	}

	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(SizeOfBaseRelocationTable)Not a valid PE flag!\n");
		return 0;
	}

	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;

	PIMAGE_DATA_DIRECTORY pDataDirectory_RelocTable = &pDataDirectory[5];

	if (!pDataDirectory_RelocTable->VirtualAddress)
	{
		printf("This program has no relocation table.\n");
		return 0;
	}

	DWORD Foa_RelocationTable = RvaToFileOffset(pFileBuffer, pDataDirectory_RelocTable->VirtualAddress);

	pRelocationTable = (PIMAGE_BASE_RELOCATION)((DWORD)pFileBuffer + Foa_RelocationTable);

	for (DWORD n = 0; pRelocationTable->VirtualAddress != IMAGE_REL_BASED_ABSOLUTE; n++)
	{
		SizeOfBaseRelocationTableSectionTotal += pRelocationTable->SizeOfBlock;
		pRelocationTable = (PIMAGE_BASE_RELOCATION)((DWORD)pRelocationTable + pRelocationTable->SizeOfBlock);
	}

	SizeOfBaseRelocationTableSectionTotal = Alignment(pOptionHeader->SectionAlignment, SizeOfBaseRelocationTableSectionTotal, 0);

	return SizeOfBaseRelocationTableSectionTotal;
}

DWORD MoveBaseRelocationTable(PVOID pFileBuffer, DWORD FileBuffer, PVOID* pFileBuffer_ExportTable)
{
	//δ����pMoveExportTableTemp��������pFileBuffer�õ�pOptionHeaderFileBuffer->SectionAlignment
	PIMAGE_DOS_HEADER pDosHeaderFileBuffer = NULL;
	PIMAGE_NT_HEADERS pNTHeaderFileBuffer = NULL;
	PIMAGE_FILE_HEADER pPEHeaderFileBuffer = NULL;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeaderFileBuffer = NULL;
	pDosHeaderFileBuffer = (PIMAGE_DOS_HEADER)pFileBuffer;
	pNTHeaderFileBuffer = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer + pDosHeaderFileBuffer->e_lfanew);
	pPEHeaderFileBuffer = (PIMAGE_FILE_HEADER)(((DWORD)pNTHeaderFileBuffer) + 4);
	pOptionHeaderFileBuffer = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeaderFileBuffer + IMAGE_SIZEOF_FILE_HEADER);

	PVOID pMoveBaseRelocationTableTemp = NULL;
	DWORD SizeBaseRelocationTableSection = SizeOfBaseRelocationTable(pFileBuffer);
	size_t AddSecTotal = FileBuffer + SizeBaseRelocationTableSection;

	if (!test)
	{
		printf("SizeBaseRelocationTableSection: %#x\n", SizeBaseRelocationTableSection);
		printf("pFileBuffer: %#x\n", FileBuffer);
		printf("pOptionHeaderFileBuffer->SizeOfImage: %#x\n", pOptionHeaderFileBuffer->SizeOfImage);
		printf("AddSecTotal: %#x\n", AddSecTotal);
		printf("NTHeader->OptionalHeader.SizeOfImage + SizeOfSection: %#x\n", pOptionHeaderFileBuffer->SizeOfImage + SizeBaseRelocationTableSection);
	}

	//�ڴ����
	AddSecTotal = Alignment(pOptionHeaderFileBuffer->SectionAlignment, AddSecTotal, 0);

	if (!test)
	{
		printf("pFileBuffer: %#x\n", FileBuffer);
		printf("pFileBufferAddSecTotal: %#x\n", AddSecTotal);
	}

	pMoveBaseRelocationTableTemp = malloc(AddSecTotal);

	if (!pMoveBaseRelocationTableTemp)
	{
		printf("(MoveBaseRelocationTable)Allocate dynamic memory failed!\n");
		return 0;
	}

	memset(pMoveBaseRelocationTableTemp, 0, AddSecTotal);
	memcpy(pMoveBaseRelocationTableTemp, pFileBuffer, FileBuffer);

	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_BASE_RELOCATION pRelocationTable = NULL;

	if (!pMoveBaseRelocationTableTemp)
	{
		printf("(MoveBaseRelocationTable)Can't open file!\n");
		return 0;
	}

	if (*((PWORD)pMoveBaseRelocationTableTemp) != IMAGE_DOS_SIGNATURE)
	{
		printf("(MoveBaseRelocationTable))No MZ flag, not exe file!\n");
		free(pMoveBaseRelocationTableTemp);
		return 0;
	}
	pDosHeader = (PIMAGE_DOS_HEADER)pMoveBaseRelocationTableTemp;

	if (*((PDWORD)((DWORD)pMoveBaseRelocationTableTemp + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(MoveBaseRelocationTable)Not a valid PE flag!\n");
		free(pMoveBaseRelocationTableTemp);
		return 0;
	}
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pMoveBaseRelocationTableTemp + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)(((DWORD)pNTHeader) + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;
	PIMAGE_DATA_DIRECTORY pDataDirectory_RelocTable = &pDataDirectory[5];

	PIMAGE_SECTION_HEADER pSectionHeaderTemp = pSectionHeader;

	for (DWORD i = 0; i < pPEHeader->NumberOfSections; i++, pSectionHeaderTemp++)
	{
		//
	}

	//�ӽ�
	int size_surplus_sizeofheader = 80;
	if (size_surplus_sizeofheader <= (pOptionHeader->SizeOfHeaders - ((DWORD)(pSectionHeaderTemp - pMoveBaseRelocationTableTemp))))
	{
		//printf("Enough.\n");
		//�õ����һ���ڵ���Ϣ
		pSectionHeaderTemp--;
		PIMAGE_SECTION_HEADER pLastSectionHeader = pSectionHeaderTemp;

		//����	
		//�Ľ���Ŀ
		pPEHeader->NumberOfSections = pPEHeader->NumberOfSections + 1;
		//���ڱ�
		pSectionHeaderTemp++;
		memcpy(pSectionHeaderTemp, pSectionHeader, IMAGE_SIZEOF_SECTION_HEADER);
		memcpy(pSectionHeaderTemp, ".movRel", 8);

		pSectionHeaderTemp->VirtualAddress = pLastSectionHeader->VirtualAddress + pLastSectionHeader->Misc.VirtualSize;
		pSectionHeaderTemp->SizeOfRawData = SizeBaseRelocationTableSection;
		pSectionHeaderTemp->PointerToRawData = pLastSectionHeader->PointerToRawData + pLastSectionHeader->SizeOfRawData;
		pSectionHeaderTemp->Misc.VirtualSize = SizeBaseRelocationTableSection;
		pOptionHeader->SizeOfImage = pOptionHeaderFileBuffer->SizeOfImage + SizeBaseRelocationTableSection;

		pSectionHeaderTemp->VirtualAddress = Alignment(pOptionHeaderFileBuffer->SectionAlignment, pSectionHeaderTemp->VirtualAddress, 0);
		pSectionHeaderTemp->VirtualAddress = Alignment(pOptionHeaderFileBuffer->FileAlignment, pSectionHeaderTemp->PointerToRawData, 0);
	}
	else
	{
		free(pMoveBaseRelocationTableTemp);
		printf("Insufficient.\n");
	}

	if (!pDataDirectory_RelocTable->VirtualAddress)
	{
		printf("This program has no relocation table.\n");
		return 0;
	}

	printf("Relocation Table Rva: %#x\n", pDataDirectory_RelocTable->VirtualAddress);
	DWORD Foa_RelocationTable = RvaToFileOffset(pMoveBaseRelocationTableTemp, pDataDirectory_RelocTable->VirtualAddress);
	printf("Relocation Table Foa: %#x\n", Foa_RelocationTable);
	pRelocationTable = (PIMAGE_BASE_RELOCATION)((DWORD)pMoveBaseRelocationTableTemp + Foa_RelocationTable);
	PVOID pAddInsSction = (PDWORD)((DWORD)pMoveBaseRelocationTableTemp + pSectionHeaderTemp->PointerToRawData);

	memcpy(pAddInsSction, pRelocationTable, pDataDirectory_RelocTable->Size);

	DWORD NewpRelocationTable = *(PDWORD)pAddInsSction;
	PIMAGE_BASE_RELOCATION pNewpRelocationTable = (PIMAGE_BASE_RELOCATION)pAddInsSction;

	if (!test)
	{
		printf("The starting address of the new section��%#x\n", ((DWORD)pMoveBaseRelocationTableTemp + pSectionHeaderTemp->PointerToRawData));
	}

	//���Թ��ܣ��޸�imagebase
	DWORD ImageBaseOrg = pOptionHeader->ImageBase;
	pOptionHeader->ImageBase += 0x10000000;
	DWORD ImageBaseNew = pOptionHeader->ImageBase;

	for (DWORD n = 0; pNewpRelocationTable->VirtualAddress != IMAGE_REL_BASED_ABSOLUTE; n++)
	{
		printf(".....................RelocationTable %#5xth.................................\n", n + 1);
		DWORD arrNumber = (pNewpRelocationTable->SizeOfBlock - 8) / 2;
		printf("TypeOffset Number: %#x\n", arrNumber);
		PDWORD pTypeOffsetOrg = ((PDWORD)pNewpRelocationTable + 2);
		PWORD pTypeOffset = (PWORD)pTypeOffsetOrg;
		for (DWORD i = 0; i < arrNumber; i++)
		{
			//printf("pTypeOffset: %#-10x", (WORD)pTypeOffset);
			WORD TypeOffset = *pTypeOffset;
			TypeOffset = (TypeOffset & 0b111111111111);
			DWORD OffsetOrg = pNewpRelocationTable->VirtualAddress + (DWORD)TypeOffset;
			//printf("Rva_BaseRelocation: %#-10x", OffsetOrg);
			DWORD Foa_OffsetOrg = RvaToFileOffset(pMoveBaseRelocationTableTemp, OffsetOrg);
			PDWORD pFoa_Offset = (PDWORD)((DWORD)pMoveBaseRelocationTableTemp + Foa_OffsetOrg);
			printf("�Ļ�ַǰFoa_BaseRelocation: %#-10x   ", *pFoa_Offset);
			*pFoa_Offset = *pFoa_Offset - ImageBaseOrg + ImageBaseNew;
			printf("�Ļ�ַ��Foa_BaseRelocation: %#-10x\n", *pFoa_Offset);
			pTypeOffset++;
		}
		pNewpRelocationTable = (PIMAGE_BASE_RELOCATION)(PDWORD)((DWORD)pNewpRelocationTable + pNewpRelocationTable->SizeOfBlock);
	}

	//�޸�����Ŀ¼����Ϣ
	pDataDirectory_RelocTable->VirtualAddress = FoaToImageOffset(pMoveBaseRelocationTableTemp, NewpRelocationTable - (DWORD)pMoveBaseRelocationTableTemp);

	size_t ret_loc6 = MemoryToFile(pMoveBaseRelocationTableTemp, AddSecTotal, write_movreloctable_file_path);
	if (!ret_loc6)
	{
		printf("(MoveRelocationTable)store memory failed.\n");
		return 0;
	}

	*pFileBuffer_ExportTable = pMoveBaseRelocationTableTemp; //�ݴ�����ݴ����������ͷ�
	free(pMoveBaseRelocationTableTemp);
	pMoveBaseRelocationTableTemp = NULL;

	return AddSecTotal;
}

//���õ����DLLע��
DWORD DLLInject(PVOID pFileBuffer, PVOID* pNewFileBuffer, char* dllname, char* dllfunction)
{
	PVOID pAddSectionTemp = NULL;
	//����.exe��PE�ṹ
	DWORD ret_loc2 = ToLoaderPE(file_path, &pFileBuffer);

	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_SECTION_HEADER pNewSection = NULL;
	PIMAGE_SECTION_HEADER pNewSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory_ImportTable = NULL;
	PIMAGE_IMPORT_DESCRIPTOR pImportTable = NULL;
	PIMAGE_IMPORT_DESCRIPTOR pNewImportTable = NULL;
	PIMAGE_IMPORT_BY_NAME  pImportByName = NULL;
	PIMAGE_THUNK_DATA pOriginalFirstThunk = NULL;
	PIMAGE_THUNK_DATA pFirstThunk = NULL;

	if (!pFileBuffer)
	{
		printf("(DLLInject_ToLoaderPE)Can't open file!\n");
		return 0;
	}

	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)  // IMAGE_DOS_SIGNATURE��4�ֽڣ���pFileBufferǿ������ת��Ϊ4�ֽ�ָ�����ͣ�PWORD��
	{
		printf("(DLLInject_ToLoaderPE)No MZ flag, not exe file!\n");
		return 0;
	}

	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(DLLInject_ToLoaderPE)Not a valid PE flag!\n");
		return 0;
	}
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	//�жϿռ乻����
	int size_surplus_sizeofheader = 80;
	if ((pOptionHeader->SizeOfHeaders - ((DWORD)pSectionHeader - (DWORD)pFileBuffer + pPEHeader->NumberOfSections * 40)) < size_surplus_sizeofheader)
	{
		printf("Insufficient.\n");
		return 0;
	}
	//�¼ӽڱ�
	pNewSection = (PIMAGE_SECTION_HEADER)(pSectionHeader + pPEHeader->NumberOfSections);
	//�޸��½�����
	memcpy(pNewSection->Name, ".addimp", 8);
	PIMAGE_SECTION_HEADER pLastSectionHeader = (PIMAGE_SECTION_HEADER)(pSectionHeader + (pPEHeader->NumberOfSections - 1));

	DWORD max = pLastSectionHeader->Misc.VirtualSize > pLastSectionHeader->SizeOfRawData ? pLastSectionHeader->Misc.VirtualSize : pLastSectionHeader->SizeOfRawData;

	pNewSection->VirtualAddress = pLastSectionHeader->VirtualAddress + max;
	pNewSection->Misc.VirtualSize = 0x1000;
	pNewSection->PointerToRawData = pLastSectionHeader->PointerToRawData + max;
	pNewSection->SizeOfRawData = 0x1000;
	pNewSection->Characteristics = 0xE0000060;
	//��ӽں���հ���40
	memset(pNewSection + 1, 0, 40);
	//�޸Ŀ�ѡͷ����׼ͷ��Ϣ
	pPEHeader->NumberOfSections += 1;
	pOptionHeader->SizeOfImage += 0x1000;
	DWORD AddSecTotal = pOptionHeader->SizeOfImage;
	pAddSectionTemp = malloc(AddSecTotal);
	//��ʼ���ڴ�
	memset(pAddSectionTemp, 0, AddSecTotal);
	memcpy(pAddSectionTemp, pFileBuffer, AddSecTotal);

	if (!pAddSectionTemp)
	{
		printf("(DLLInject)Can't open file!\n");
		return 0;
	}

	if (*((PWORD)pAddSectionTemp) != IMAGE_DOS_SIGNATURE)
	{
		printf("(DLLInject)No MZ flag, not exe file!\n");
		return 0;
	}

	pDosHeader = (PIMAGE_DOS_HEADER)pAddSectionTemp;
	if (*((PDWORD)((DWORD)pAddSectionTemp + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(DLLInject)Not a valid PE flag!\n");
		return 0;
	}

	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pAddSectionTemp + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pNewSectionHeader = &pSectionHeader[pPEHeader->NumberOfSections - 1];
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;
	pDataDirectory_ImportTable = &pDataDirectory[1];

	if (!pDataDirectory_ImportTable->VirtualAddress)
	{
		printf("(DLLInject)This program has no import table.\n");
		return 0;
	}

	printf("Import Table Rva: %#x\n", pDataDirectory_ImportTable->VirtualAddress);
	DWORD Foa_ImportTable = RvaToFileOffset(pAddSectionTemp, pDataDirectory_ImportTable->VirtualAddress);
	printf("Import Table Foa: %#x\n", Foa_ImportTable);
	pImportTable = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)pAddSectionTemp + Foa_ImportTable);

	DWORD Rva_NewSectionAddr = pNewSectionHeader->PointerToRawData;
	DWORD Foa_NewSectionAddr = RvaToFileOffset(pAddSectionTemp, Rva_NewSectionAddr);
	PDWORD Foa_pNewSectionAddr = (PDWORD)((DWORD)pAddSectionTemp + Foa_NewSectionAddr);
	//�ƶ������
	memcpy((PVOID)Foa_pNewSectionAddr, (PVOID)pImportTable, pDataDirectory_ImportTable->Size);

	//�޸�Ŀ¼��
	pDataDirectory_ImportTable->VirtualAddress = Rva_NewSectionAddr;

	//�˻�һ�������Ĵ�С����Ϊ���һ�������ȫΪ0
	pNewImportTable = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)Foa_pNewSectionAddr + pDataDirectory_ImportTable->Size - sizeof(IMAGE_IMPORT_DESCRIPTOR));

	PIMAGE_THUNK_DATA pNewINT = (PIMAGE_THUNK_DATA)((DWORD)pNewImportTable + sizeof(IMAGE_IMPORT_DESCRIPTOR) * 2);//pNewImport->OriginalFirstThunkָ���INT��*2������һ�������ṹ��С�Ŀռ�ȫ0��ʾ������Ѿ�����
	PIMAGE_THUNK_DATA pNewIAT = (PIMAGE_THUNK_DATA)((DWORD)pNewINT + sizeof(IMAGE_THUNK_DATA) * 2);	//pNewImport->FirstThunkָ���IAT�� *2������һ��IMAGE_THUNK_DATA��С�Ŀռ�ȫ0��ʾpINT�Ѿ�����
	DWORD Rva_NewINTAddr = FoaToImageOffset(pAddSectionTemp, (DWORD)pNewINT - (DWORD)pAddSectionTemp);
	DWORD Rva_NewIATAddr = FoaToImageOffset(pAddSectionTemp, (DWORD)pNewIAT - (DWORD)pAddSectionTemp);


	//����dll����
	PDWORD pDLLName = (PDWORD)((DWORD)pNewIAT + sizeof(PIMAGE_THUNK_DATA) * 2);
	DWORD DLLName_Size = strlen(dllname) + 1;
	memcpy(pDLLName, dllname, DLLName_Size);

	pNewImportTable->Name = FoaToImageOffset(pAddSectionTemp, (DWORD)pDLLName - (DWORD)pAddSectionTemp);

	PIMAGE_IMPORT_BY_NAME pNewImportByName = (PIMAGE_IMPORT_BY_NAME)(PVOID)((DWORD)pDLLName + DLLName_Size);
	DWORD Rva_NewImportByNameAddr = FoaToImageOffset(pAddSectionTemp, (DWORD)pNewImportByName - (DWORD)pAddSectionTemp);

	//���ƺ�������
	PDWORD pDLLFunName = (PDWORD)((DWORD)pNewImportByName + 2);
	pNewImportByName->Hint = 0;
	DWORD DLLFunName_Size = strlen(dllfunction) + 1;
	memcpy(pDLLFunName, dllfunction, DLLFunName_Size);

	pNewImportTable->OriginalFirstThunk = Rva_NewINTAddr;
	printf("pNewImport->OriginalFirstThunk:%#X\n", pNewImportTable->OriginalFirstThunk);

	pNewImportTable->FirstThunk = Rva_NewIATAddr;
	printf("pNewImport->FirstThunk:%#X\n", pNewImportTable->FirstThunk);
	pNewINT->u1.AddressOfData = Rva_NewImportByNameAddr;
	pNewIAT->u1.AddressOfData = Rva_NewImportByNameAddr;

	DWORD ret_loc4 = MemoryToFile(pAddSectionTemp, AddSecTotal, write_dllinject_file_path);
	if (!ret_loc4)
	{
		printf("(DLLInject)Store memory failed.\n");
		return 0;
	}

	*pNewFileBuffer = pAddSectionTemp;
	return AddSecTotal;
}

VOID operate()
{
	PVOID pFileBuffer = NULL;
	PVOID pNewFileBuffer = NULL;
	PVOID pImageBuffer = NULL;
	PVOID* pFileBuffer_ExportTable = NULL;
	char file_path[] = "D:\\test_dll_02.dll";
	char fun_name[] = "D:\\test_dll_02111.dll";
	//DWORD pRVA = 0x0003123;
	//DWORD pFOA = 0x00020450;

	DWORD ret1 = ToLoaderPE(file_path, &pFileBuffer);  // &pFileBuffer(void**����) ���ݵ�ַ����ֵ���Խ����޸�
	printf("exe->filebuffer  ����ֵΪ���������ļ���С��%#x\n", ret1);

	//LogExportTable(pFileBuffer);
	//LogPEHeaderInfo(pFileBuffer);
	//LogBaseRelocationTable(pFileBuffer);
	//LogImportTable(pFileBuffer);
	//LogBoundImportTable(pFileBuffer);

	//DWORD ret9 = GetFunctionAddrByOrdinals(pFileBuffer, ordianl);
	//printf("GetFunctionAddr is %#x\n", ret9);
	//DWORD ret10 = GetFunctionAddrByName(pFileBuffer, fun_name);
	//printf("GetFunctionAddr is %#x\n", ret10);

	//int size = MoveExportTable(pFileBuffer, ret1, pFileBuffer_ExportTable);
	//MemoryToFile(pFileBuffer_ExportTable,size, fun_name);

	DWORD ret13 = MoveBaseRelocationTable(pFileBuffer, ret1, &pNewFileBuffer);
	printf("SizeOfRelocationTableSection: %#x\n", ret13);

	free(pFileBuffer);
	free(pNewFileBuffer);
	free(pImageBuffer);
	free(pFileBuffer_ExportTable);
}

int main()
{
	operate();

	getchar();
	return 0;
}
