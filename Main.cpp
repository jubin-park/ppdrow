#include <cstdio>
#include <cstdint>

/*
* struct JafFixedHeader {
    u8 description[53];
    u8 filename[200];
    u32 count;
};

struct JafDataHeader
{
    u32 unknown1;
    u32 unknown2;
    s8 spacebar;
    s8 zero_based_index_str[251];  
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
	uint32_t Code;
	char Spacebar;
	char ZeroBasedNumStr[251];
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
			printf("\tinfoIndex: %3s, frameCount: %3d, code: %X\n", pInfoHeader->ZeroBasedNumStr, pInfoHeader->FrameCount, pInfoHeader->Code);
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
	//AnalyzeJAF(L"C:\\wordpp\\ani\\arrow.jaf");
	AnalyzeJAF(L"C:\\wordpp\\ani\\boss\\boss001a.jaf");
	//AnalyzeJAF(L"C:\\wordpp\\ani\\3000\\3100.jaf");
	//AnalyzeJAF(L"C:\\wordpp\\ani\\3000\\3100.jaf");


	return 0;
}