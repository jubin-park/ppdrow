#include <cstdio>
#include <cstdint>
#include <clocale>

/*
struct JafFixedHeader {
    u8 description[53];
    u8 filename[200];
    u32 count;
};

struct JafDataHeader
{
    u32 unknown1;
    s8 zero_based_index_str[256];
};

struct JafNumHeader
{
    s32 OneBasedNumber;
    s32 Unknown3;
    s32 Unknown4;
};

JafFixedHeader test @ 0x00;
JafDataHeader test2 @ 257;
JafNumHeader test3[10] @ 517;

struct JSFFixedHeader {
    u8 description[50];
    u8 filename[200];
    u32 spriteCount;
    u32 unknown1;
    u32 unknown2;
    u32 unknown3;
    u32 unknown4;
    u16 unknown5;
    u32 width1;
    u32 height1;
    u32 unknown8;
    u16 width2;
    u16 height2;
};

JSFFixedHeader test @ 0x00;
*/

#pragma pack(1)
struct JAFFileHeader
{
	char Description[53];
	char FileName[200];
	uint32_t InfoCount;
};

struct JAFInfoHeader
{
	uint32_t FrameCount;
	char TagName[256];
};

struct JAFFrameHeader
{
	uint32_t OneBasedNumber;
	uint32_t Unknown3;
	uint32_t Unknown4;
};

void AnalyzeJAF(const wchar_t* pWszFilePath)
{
	FILE* pFile = nullptr;
	errno_t err;
	uint32_t bufSize;
	uint8_t* paBuf = nullptr;
	uint32_t readByteCount;
	JAFFileHeader* pFormatJAF;
	JAFInfoHeader* pInfoHeader;
	JAFFrameHeader* pFrameHeader;
	char* offset = 0;

	do
	{
		err = _wfopen_s(&pFile, pWszFilePath, L"rb");
		if (err != 0 || pFile == nullptr)
		{
			break;
		}

		fseek(pFile, 0, SEEK_END);
		bufSize = static_cast<uint32_t>(ftell(pFile));
		fseek(pFile, 0, SEEK_SET);

		if (bufSize < 2)
		{
			break;
		}

		paBuf = new uint8_t[bufSize];
		readByteCount = static_cast<uint32_t>(fread_s(paBuf, bufSize, sizeof(uint8_t), bufSize, pFile));

		pFormatJAF = reinterpret_cast<JAFFileHeader*>(paBuf);

		offset = (char*)pFormatJAF + sizeof(JAFFileHeader);
		printf("JSF fileName: \"%s\", infoCount: %d\n", pFormatJAF->FileName, pFormatJAF->InfoCount);
		for (uint32_t infoIndex = 0; infoIndex < pFormatJAF->InfoCount; ++infoIndex)
		{
			pInfoHeader = (JAFInfoHeader*)offset;
			printf("\ttag: \"%s\", frameCount: %3d\n", pInfoHeader->TagName, pInfoHeader->FrameCount);
			offset += sizeof(JAFInfoHeader);

			for (uint32_t sliceIndex = 0; sliceIndex < pInfoHeader->FrameCount; ++sliceIndex)
			{
				pFrameHeader = (JAFFrameHeader*)offset;
				printf("\t\tnum: %3u, Unknown3: %u, Unknown4: %u\n",pFrameHeader->OneBasedNumber, pFrameHeader->Unknown3, pFrameHeader->Unknown4);
				offset += sizeof(JAFFrameHeader);
			}
		}
		printf("------------------------------------------------------\n");


	} while (0);

	delete[] paBuf;
}

int main()
{
	_wsetlocale(LC_ALL, L"ko-KR");

	//AnalyzeJAF(L"C:\\wordpp\\ani\\arrow.jaf");
	//AnalyzeJAF(L"C:\\wordpp\\ani\\boss\\boss001a.jaf");
	//AnalyzeJAF(L"C:\\wordpp\\ani\\3000\\3100.jaf");
	//AnalyzeJAF(L"C:\\wordpp\\ani\\3000\\3100.jaf");
	AnalyzeJAF(L"C:\\wordpp\\ani\\cursor.jaf");

	return 0;
}