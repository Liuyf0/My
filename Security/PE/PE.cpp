#include "stdafx.h"
#include <string.h>
#include <malloc.h>
#include <windows.h>


// exe->filebuffer  ����ֵΪ���������ļ���С
int ReadPEFile(char* file_path, PVOID* pFileBuffer)
{
	FILE* pfile = NULL;  // �ļ�ָ��
	DWORD file_size = 0;   //�洢��������ļ���С
	LPVOID pTempFilebuffer = NULL;

	// ���ļ�
    fopen_s(&pfile, file_path, "rb");  // ������µ�ָ�룬��Ҫ�����ж�
	if (!pfile)
	{
		printf("��exe�ļ�ʧ�ܣ�\n");//�������ʧ�ܾ�Ҫ�ر��ļ����ͷŶ�̬�ڴ桢ָ��ָ��NULL
		return 0;
	}
	// ��ȡ�ļ���С
	fseek(pfile, 0, SEEK_END);
	file_size = ftell(pfile);
	fseek(pfile, 0, SEEK_SET);
	// ����ռ�
	pTempFilebuffer = malloc(file_size);  // ������µ�ָ�룬��Ҫ�����ж�
	if (!pTempFilebuffer)
	{
		printf("����ռ�ʧ�ܣ�\n");//�������ʧ�ܾ�Ҫ�ر��ļ����ͷŶ�̬�ڴ桢ָ��ָ��NULL
		fclose(pfile);
		return 0;
	}
	// �����ݶ�ȡ���ڴ���
	size_t n = fread(pTempFilebuffer, file_size, 1, pfile);
	if (!n)
	{
		printf("���ݶ�ȡ���ڴ���ʧ�ܣ�\n"); //�������ʧ�ܾ�Ҫ�ر��ļ����ͷŶ�̬�ڴ桢ָ��ָ��NULL
		fclose(pfile);
		free(pTempFilebuffer);
		return 0;
	}
	// �ر��ļ�(�Ѿ���ȡ���ڴ���)
	*pFileBuffer = pTempFilebuffer;
	pTempFilebuffer = NULL;   //ָ��ָ��NULL
	fclose(pfile);
	return file_size;
}

// filebuffer -> imagebuffer
DWORD CopyFileBufferToImageBuffer(PVOID pFileBuffer, PVOID* pImageBuffer)
{
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	// ��ʼ��IMAGE_BUFFERָ��(temparay)
	LPVOID pTempImagebuffer = NULL;

	if (!pFileBuffer)
	{
		printf("(2pimagebuffer�׶�)��ȡ���ڴ��pfilebuffer��Ч��\n");
		return 0;
	}
	// �ж��Ƿ��ǿ�ִ���ļ�
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)  // IMAGE_DOS_SIGNATURE��4�ֽڣ���pFileBufferǿ������ת��Ϊ4�ֽ�ָ�����ͣ�PWORD��
	{
		printf("(2pimagebuffer�׶�)����MZ��־������exe�ļ���\n");
		return 0;
	}
	//ǿ�ƽṹ������ת��pDosHeader
	pDosHeader = PIMAGE_DOS_HEADER(pFileBuffer);
	//�ж��Ƿ���PE��־       
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE) // ע��ָ��ļӷ��ǣ�ȥ��һ��*���������ӡ�����ת��ΪDWORD�����ټӼ���
	{																			  //��Ӻ�ĺ� ǿ������ת��Ϊ4�ֽ�ָ�����ͣ�PWORD�� IMAGE_NT_SIGNATURE 4BYTES
		printf("(2pimagebuffer�׶�)������Ч��PE��־��\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��  ����ǿ������ת����������
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);

	// ���䶯̬�ڴ棬SizeOfImage��ʾװ���ڴ��ܴ�С
	pTempImagebuffer = malloc(pOptionHeader->SizeOfImage);
	if (!pTempImagebuffer)
	{
		printf("���䶯̬�ڴ�ʧ�ܣ�\n");
		free(pTempImagebuffer);
		return 0;
	}
	// ��ʼ����̬�ڴ�
	memset(pTempImagebuffer, 0, pOptionHeader->SizeOfImage);
	// ����ͷ��
	memcpy(pTempImagebuffer, pDosHeader, pOptionHeader->SizeOfHeaders);
	// ѭ�������ڱ�ע�����
	PIMAGE_SECTION_HEADER pTempSectionHeader = pSectionHeader;
	for (DWORD i = 0; i<pPEHeader->NumberOfSections; i++, pTempSectionHeader++)
	{
		memcpy((void*)((DWORD)pTempImagebuffer + pTempSectionHeader->VirtualAddress), (void*)((DWORD)pFileBuffer + pTempSectionHeader->PointerToRawData), pTempSectionHeader->SizeOfRawData);
	}
	// ��������
	*pImageBuffer = pTempImagebuffer;
	pTempImagebuffer = NULL;    //ָ��ָ��NULL
	return pOptionHeader->SizeOfImage;
}

//imagebuffer->newbuffer
DWORD CopyImageBufferToNewBuffer(PVOID pImageBuffer, PVOID* pNewBuffer)
{
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;

	// ��ʼ��NEW_BUFFERָ��(temparay)
	LPVOID pTempNewbuffer = NULL;

	// �ж�pImageBuffer�Ƿ���Ч
	if (!pImageBuffer)
	{
		printf("(2pnewbuffer�׶�)��ȡ���ڴ��pimagebuffer��Ч��\n");
		return 0;
	}
	//�ж��ǲ���exe�ļ�
	if (*((PWORD)pImageBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(2pnewbuffer�׶�)����MZ��־������exe�ļ���\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pDosHeader = (PIMAGE_DOS_HEADER)pImageBuffer;
	if (*((PDWORD)((DWORD)pImageBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(2pnewbuffer�׶�)������Ч��PE��־��\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pImageBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);

	//��ȡnew_buf	fer�Ĵ�С
	int new_buffer_size = pOptionHeader->SizeOfHeaders;
	for (DWORD i = 0; i<pPEHeader->NumberOfSections; i++)
	{
		new_buffer_size += pSectionHeader[i].SizeOfRawData;  // pSectionHeader[i]��һ�ּӷ�
	}
	// �����ڴ棨newbuffer��
	pTempNewbuffer = malloc(new_buffer_size);
	if (!pTempNewbuffer)
	{
		printf("(2pnewbuffer�׶�)����Newbufferʧ�ܣ�\n");
		return 0;
	}
	memset(pTempNewbuffer, 0, new_buffer_size);
	// ����ͷ��
	memcpy(pTempNewbuffer, pDosHeader, pOptionHeader->SizeOfHeaders);
	// ѭ����������
	PIMAGE_SECTION_HEADER pTempSectionHeader = pSectionHeader;
	for (DWORD j = 0; j<pPEHeader->NumberOfSections; j++, pTempSectionHeader++)
	{	//PointerToRawData�������ļ��е�ƫ��,VirtualAddress�������ڴ��е�ƫ�Ƶ�ַ,SizeOfRawData�����ļ��ж����ĳߴ�
		memcpy((PDWORD)((DWORD)pTempNewbuffer + pTempSectionHeader->PointerToRawData), (PDWORD)((DWORD)pImageBuffer + pTempSectionHeader->VirtualAddress), pTempSectionHeader->SizeOfRawData);
	}
	//��������
	*pNewBuffer = pTempNewbuffer; //�ݴ�����ݴ����������ͷ�
	pTempNewbuffer = NULL;
	return new_buffer_size;  // ���ؼ���õ��ķ����ڴ�Ĵ�С
}

//newbuffer->����
int newbuffer_write2_exe(PVOID NewFileBuffer, DWORD FileSize, char* FilePath)
{
	FILE* fpl;
	fopen_s(&fpl, FilePath, "wb");
	if (fpl != NULL)
	{
		fwrite(NewFileBuffer, FileSize, 1, fpl);
	}
	fclose(fpl);
	return 1;
}

DWORD MoveExportTable(PVOID pFileBuffer, DWORD FileBuffer, PVOID* pFileBuffer_ExportTable)
{

}

void operate_pe()
{   // ��ʼ������
	PVOID pFileBuffer = NULL;
	PVOID pImageBuffer = NULL;
	PVOID pNewFileBuffer = NULL;
	DWORD NewFileBufferSize = 0;
	//char file_path[] = "D:\\Lib\\IPMSG2007.exe";
	char file_path[] = "C:\\Windows\\System32\\notepad.exe";
	char write_file_path[] = "D:\\Lib\\cp_notepad.exe";

	// exe->filebuffer
	int ret1 = ReadPEFile(file_path, &pFileBuffer);  // &pFileBuffer(void**����) ���ݵ�ַ����ֵ���Խ����޸�
	printf("exe->filebuffer  ����ֵΪ���������ļ���С��%#x\n", ret1);
	// filebuffer -> imagebuffer
	int ret2 = CopyFileBufferToImageBuffer(pFileBuffer, &pImageBuffer);
	printf("filebuffer -> imagebuffer����ֵΪ���������ļ���С��%#x\n", ret2);
	//imagebuffer->newbuffer
	int FileSize = CopyImageBufferToNewBuffer(pImageBuffer, &pNewFileBuffer);
	printf("imagebuffer->newbuffer����ֵΪ���������ļ���С��%#x\n", FileSize);
	//newbuffer->����
	int ret4 = newbuffer_write2_exe(pNewFileBuffer, FileSize, write_file_path);
	printf("newbuffer->���̷���ֵΪ��%d\n", ret4);
}


int main()
{
	operate_pe();
	getchar();
	return 0;
}
