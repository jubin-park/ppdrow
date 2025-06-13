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
*/

#pragma pack(push, 1)
struct JAFFileHeader
{
	int8_t Version[32];
	int8_t Description[21];
	int8_t FileName[200];
	uint32_t InfoCount;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct JAFInfoHeader
{
	uint32_t FrameCount;
	int8_t TagName[256];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct JAFFrameHeader
{
	uint32_t OneBasedNumber;
	uint32_t Unknown3;
	uint32_t Unknown4;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct JSFFileHeader
{
	int8_t Version[32];
	int8_t Description[18];
	int8_t FileName[200];
	uint32_t InfoCount;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct JSFInfoHeader
{
	uint32_t Unknown11;
	uint32_t Unknown12;
	uint32_t Unknown13;
	uint32_t Unknown14;
	uint16_t Unknown15;
	uint32_t Width1;
	uint32_t Height1;
	uint32_t WordCount;
	uint16_t Width2;
	uint16_t Height2;
};
#pragma pack(pop)

/*
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

void AnalyzeJAF(const wchar_t* pWszFilePath);
void AnalyzeJSF(const wchar_t* pWszFilePath);

void AnalyzeJAF(const wchar_t* pWszFilePath)
{
	FILE* pFile = nullptr;
	errno_t err;
	uint32_t bufSize;
	int8_t* paBuf = nullptr;
	uint32_t readByteCount;
	JAFFileHeader* pJAFFileHeader;
	JAFInfoHeader* pJAFInfoHeader;
	JAFFrameHeader* pJAFFrameHeader;
	int8_t* pOffset = 0;

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

		paBuf = new int8_t[bufSize];
		readByteCount = static_cast<uint32_t>(fread_s(paBuf, bufSize, sizeof(int8_t), bufSize, pFile));
		pOffset = paBuf;

		pJAFFileHeader = reinterpret_cast<JAFFileHeader*>(pOffset);
		pOffset += sizeof(JAFFileHeader);

		printf("JSF fileName: \"%s\", infoCount: %d\n", pJAFFileHeader->FileName, pJAFFileHeader->InfoCount);
		for (uint32_t infoIndex = 0; infoIndex < pJAFFileHeader->InfoCount; ++infoIndex)
		{
			pJAFInfoHeader = (JAFInfoHeader*)pOffset;
			printf("\ttag: \"%s\", frameCount: %3d\n", pJAFInfoHeader->TagName, pJAFInfoHeader->FrameCount);
			pOffset += sizeof(JAFInfoHeader);

			for (uint32_t sliceIndex = 0; sliceIndex < pJAFInfoHeader->FrameCount; ++sliceIndex)
			{
				pJAFFrameHeader = (JAFFrameHeader*)pOffset;
				printf("\t\tnum: %3u, Unknown3: %u, Unknown4: %u\n",pJAFFrameHeader->OneBasedNumber, pJAFFrameHeader->Unknown3, pJAFFrameHeader->Unknown4);
				pOffset += sizeof(JAFFrameHeader);
			}
		}
		printf("------------------------------------------------------\n\n");


	} while (0);

	delete[] paBuf;
}

void AnalyzeJSF(const wchar_t* pWszFilePath)
{
	FILE* pFile = nullptr;
	errno_t err;
	uint32_t bufSize;
	int8_t* paBuf = nullptr;
	uint32_t readByteCount;
	JSFFileHeader* pJSFFileHeader;
	JSFInfoHeader* pJSFInfoHeader;
	int8_t* pOffset = 0;
	int8_t* pEnd = 0;

	uint16_t repeatCount;
	uint16_t startOffset;
	uint16_t bitmapByteCount;

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

		paBuf = new int8_t[bufSize];
		readByteCount = static_cast<uint32_t>(fread_s(paBuf, bufSize, sizeof(int8_t), bufSize, pFile));
		pOffset = paBuf;

		pJSFFileHeader = reinterpret_cast<JSFFileHeader*>(pOffset);
		pOffset += sizeof(JSFFileHeader);

		printf("BMP fileName: \"%s\", infoCount: %d\n", pJSFFileHeader->FileName, pJSFFileHeader->InfoCount);

		for (uint32_t infoIndex = 0; infoIndex < pJSFFileHeader->InfoCount; ++infoIndex)
		{
			pJSFInfoHeader = reinterpret_cast<JSFInfoHeader*>(pOffset);
			pOffset += sizeof(JSFInfoHeader);
			pEnd = pOffset + pJSFInfoHeader->WordCount * 2;

			printf("[%u]\t%u, %u, %u, %u, %hu"
				"\tW1: %4u, H1: %4u, Words: %4u, W2: %4hu, H2: %4hu (bytes: %hu)\n",
				infoIndex,
				pJSFInfoHeader->Unknown11, pJSFInfoHeader->Unknown12, pJSFInfoHeader->Unknown13, pJSFInfoHeader->Unknown14, pJSFInfoHeader->Unknown15,
				pJSFInfoHeader->Width1, pJSFInfoHeader->Height1, pJSFInfoHeader->WordCount, pJSFInfoHeader->Width2, pJSFInfoHeader->Height2,
				pJSFInfoHeader->WordCount * 2
			);

			while (pOffset < pEnd)
			{
				repeatCount = *reinterpret_cast<uint16_t*>(pOffset);
				pOffset += sizeof(uint16_t);
				if (repeatCount == 0)
				{
					continue;
				}
				else if (repeatCount == 1)
				{
					bitmapByteCount = pJSFInfoHeader->Width2 * 2;
					pOffset += bitmapByteCount;

					printf("\trepeat: %hu, bitmapByteCount: %hu\n", repeatCount, bitmapByteCount);
				}
				else
				{
					--repeatCount;
					for (uint16_t repeat = 1; repeat <= repeatCount; ++repeat)
					{
						startOffset = *reinterpret_cast<uint16_t*>(pOffset) * 2;
						pOffset += sizeof(uint16_t);

						bitmapByteCount = *reinterpret_cast<uint16_t*>(pOffset) * 2;
						pOffset += sizeof(uint16_t);
						pOffset += bitmapByteCount;

						printf("\trepeat: %hu/%hu, startOffset: %hu, bitmapBytes: %hu\n", repeat, repeatCount, startOffset, bitmapByteCount);
					}
				}
			}
		}

	} while (0);
}

int main()
{
	_wsetlocale(LC_ALL, L"ko-KR");

	//AnalyzeJAF(L"C:\\wordpp\\ani\\arrow.jaf");
	//AnalyzeJAF(L"C:\\wordpp\\ani\\boss\\boss001a.jaf");
	//AnalyzeJAF(L"C:\\wordpp\\ani\\3000\\3100.jaf");
	//AnalyzeJAF(L"C:\\wordpp\\ani\\3000\\3100.jaf");
	//AnalyzeJAF(L"C:\\wordpp\\ani\\cursor.jaf");


	//AnalyzeJSF(L"C:\\wordpp\\ani\\cursor.jsf");
	AnalyzeJSF(L"C:\\wordpp\\ani\\btn111.jsf");

	return 0;
}