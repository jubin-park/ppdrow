#include <cassert>
#include <cstdio>
#include <cstdint>
#include <clocale>
#include <shlwapi.h>
#include <strsafe.h>
#include <pathcch.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Pathcch.lib")

typedef uint16_t RGB565;

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
	uint32_t Unknown16;
	uint32_t Unknown17;
	uint32_t WordCount;
	uint16_t Width;
	uint16_t Height;
};
#pragma pack(pop)

struct BGR888
{
	uint8_t Blue;
	uint8_t Green;
	uint8_t Red;
};

enum : RGB565
{
	BLANK_COLOR1 = 0xf81f,
	BLANK_COLOR2 = 0xf81f//0x07e0
};

void AnalyzeJAF(const wchar_t* pWszFilePath);
void ConvertJSFToBMP(const wchar_t* pWszFilePath);
void SaveBMP(const wchar_t* const pFileName, const LONG width, const LONG height, const RGB565* const paRGB565);
void GetFileNameWithoutExtension(const wchar_t* const pWszFilePath, wchar_t* pDstFileName);

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
	size_t len;
	wchar_t wFileName[MAX_PATH];
	wchar_t wTagName[MAX_PATH];

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

		mbstowcs_s(&len, wFileName, MAX_PATH, (char*)pJAFFileHeader->FileName, MAX_PATH);
		wprintf(L"JSF fileName: \"%s\", infoCount: %d\n", wFileName, pJAFFileHeader->InfoCount);

		for (uint32_t infoIndex = 0; infoIndex < pJAFFileHeader->InfoCount; ++infoIndex)
		{
			pJAFInfoHeader = (JAFInfoHeader*)pOffset;

			mbstowcs_s(&len, wTagName, MAX_PATH, (char*)pJAFInfoHeader->TagName, MAX_PATH);
			wprintf(L"\ttag: \"%s\", frameCount: %3d\n", wTagName, pJAFInfoHeader->FrameCount);
			pOffset += sizeof(JAFInfoHeader);

			for (uint32_t sliceIndex = 0; sliceIndex < pJAFInfoHeader->FrameCount; ++sliceIndex)
			{
				pJAFFrameHeader = (JAFFrameHeader*)pOffset;
				wprintf(L"\t\tnum: %3u, Unknown3: %u, Unknown4: %u\n",pJAFFrameHeader->OneBasedNumber, pJAFFrameHeader->Unknown3, pJAFFrameHeader->Unknown4);
				pOffset += sizeof(JAFFrameHeader);
			}
		}
		wprintf(L"------------------------------------------------------\n\n");


	} while (0);

	delete[] paBuf;
}

void ConvertJSFToBMP(const wchar_t* pWszFilePath)
{
	wchar_t fileNameWithoutExtension[MAX_PATH];
	wchar_t bmpFileName[MAX_PATH];
	wchar_t directoryPath[MAX_PATH];

	FILE* pFile = nullptr;
	errno_t err;
	uint32_t bufSize;
	int8_t* paBuf = nullptr;
	uint32_t readByteCount;
	JSFFileHeader* pJSFFileHeader;
	JSFInfoHeader* pJSFInfoHeader;
	int8_t* pOffset = 0;
	int8_t* pEnd = 0;
	uint32_t area;

	uint16_t repeatCount;
	uint16_t baseOffset;
	uint16_t bitmapByteCount;

	RGB565* paRGB565 = nullptr;
	RGB565* pRGB565Iter;

	StringCchCopyW(directoryPath, _countof(directoryPath), pWszFilePath);
	PathCchRemoveFileSpec(directoryPath, _countof(directoryPath));
	GetFileNameWithoutExtension(pWszFilePath, fileNameWithoutExtension);

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
		fclose(pFile);

		pOffset = paBuf;

		pJSFFileHeader = reinterpret_cast<JSFFileHeader*>(pOffset);
		pOffset += sizeof(JSFFileHeader);

		wchar_t wFileName[MAX_PATH];
		size_t len;
		mbstowcs_s(&len, wFileName, MAX_PATH, (char*)pJSFFileHeader->FileName, MAX_PATH);

		for (uint32_t infoIndex = 0; infoIndex < pJSFFileHeader->InfoCount; ++infoIndex)
		{
			StringCbPrintfW(bmpFileName, sizeof(bmpFileName), L"%s\\%s_%03u.bmp", directoryPath, fileNameWithoutExtension, infoIndex);

			wprintf(L"\n-- Convert \"%s\" -> \"%s\", SpriteCount: %d --\n", pWszFilePath, bmpFileName, pJSFFileHeader->InfoCount);

			pJSFInfoHeader = reinterpret_cast<JSFInfoHeader*>(pOffset);
			pOffset += sizeof(JSFInfoHeader);
			pEnd = pOffset + pJSFInfoHeader->WordCount * sizeof(RGB565);
			area = pJSFInfoHeader->Width * pJSFInfoHeader->Height;

			/*
			wprintf(L"[%u]\t%u, %u, %u, %u, %hu, %u, %u"
				"\tWords: %4u, W: %hu x H: %hu = %u, (bytes: %zu)\n",
				infoIndex,
				pJSFInfoHeader->Unknown11, pJSFInfoHeader->Unknown12, pJSFInfoHeader->Unknown13, pJSFInfoHeader->Unknown14, pJSFInfoHeader->Unknown15, pJSFInfoHeader->Unknown16, pJSFInfoHeader->Unknown17,
				pJSFInfoHeader->WordCount, pJSFInfoHeader->Width, pJSFInfoHeader->Height, area, pJSFInfoHeader->WordCount * sizeof(RGB565)
			);
			*/
			
			paRGB565 = new RGB565[area];
			pRGB565Iter = paRGB565;

			while (pOffset < pEnd)
			{
				repeatCount = *reinterpret_cast<uint16_t*>(pOffset);
				pOffset += sizeof(uint16_t);
				if (repeatCount == 0)
				{
					for (int i = 0; i < pJSFInfoHeader->Width; ++i)
					{
						*pRGB565Iter++ = BLANK_COLOR1;
					}
				}
				else if (repeatCount == 1)
				{
					bitmapByteCount = pJSFInfoHeader->Width * sizeof(RGB565);

					memcpy(pRGB565Iter, pOffset, bitmapByteCount);
					pRGB565Iter += pJSFInfoHeader->Width;
					pOffset += bitmapByteCount;

					//wprintf("\trepeat: %hu, bitmapByteCount: %hu\n", repeatCount, bitmapByteCount);
				}
				else
				{
					--repeatCount;
					int sumOffset = 0;
					int sumLength = 0;

					for (uint16_t repeat = 1; repeat <= repeatCount; ++repeat)
					{
						baseOffset = *reinterpret_cast<uint16_t*>(pOffset);
						pOffset += sizeof(uint16_t);

						bitmapByteCount = *reinterpret_cast<uint16_t*>(pOffset) * sizeof(RGB565);
						pOffset += sizeof(uint16_t);

						for (int i = 0; i < baseOffset; ++i)
						{
							*pRGB565Iter++ = BLANK_COLOR1;
						}
						memcpy(pRGB565Iter, pOffset, bitmapByteCount);
						pRGB565Iter += bitmapByteCount / sizeof(RGB565);
						pOffset += bitmapByteCount;

						sumOffset += baseOffset;
						sumLength += bitmapByteCount;

						if (repeat == repeatCount)
						{
							int tailOffset = pJSFInfoHeader->Width - sumOffset - sumLength / sizeof(RGB565);
							for (int i = 0; i < tailOffset; ++i)
							{
								*pRGB565Iter++ = BLANK_COLOR2;
							}
							//wprintf("\trepeat: %hu/%hu, baseOffset: %hu, bitmapWords: %hu, tailOffset: %hu\n", repeat, repeatCount, baseOffset, bitmapByteCount / 2, tailOffset);
						}
						else
						{
							//wprintf("\trepeat: %hu/%hu, baseOffset: %hu, bitmapWords: %hu\n", repeat, repeatCount, baseOffset, bitmapByteCount / 2);
						}
					}
				}
			}

			if (pOffset != pEnd)
			{
				__debugbreak();
			}

			if (pRGB565Iter - paRGB565 != area)
			{
				__debugbreak();
			}

			SaveBMP(bmpFileName, pJSFInfoHeader->Width, pJSFInfoHeader->Height, paRGB565);

			delete[] paRGB565;
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

	//ConvertJSFToBMP(L"C:\\wordpp\\ani\\btn108.jsf");
	//ConvertJSFToBMP(L"C:\\wordpp\\ani\\cursor.jsf");
	//ConvertJSFToBMP(L"C:\\wordpp\\ani\\ui_shop.jsf");
	//ConvertJSFToBMP(L"C:\\wordpp\\ani\\3000\\3000.jsf");
	//ConvertJSFToBMP(L"C:\\wordpp\\ani\\bobj001.jsf");
	//ConvertJSFToBMP(L"C:\\wordpp\\ani\\bossitem000.jsf");
	//ConvertJSFToBMP(L"C:\\wordpp\\ani\\bossitem001.jsf");
	//ConvertJSFToBMP(L"C:\\wordpp\\ani\\boss\\collection000.jsf");

	wchar_t path[1024];

	while (fgetws(path, _countof(path), stdin) != nullptr)
	{
		size_t len = wcsnlen_s(path, _countof(path));
		path[--len] = L'\0';
		if (len > 0)
		{
			ConvertJSFToBMP(path);
		}
	}

	return 0;
}

void SaveBMP(const wchar_t* const pFileName, const LONG width, const LONG height, const RGB565* const paRGB565)
{
	assert(pFileName != nullptr);
	assert(width > 0);
	assert(height > 0);
	assert(paRGB565 != nullptr);

	int stride = ((((width * 24) + 31) & ~31) >> 3);
	int biSizeImage = abs(height) * stride;

	LONG area;
	uint8_t* paBGR888 = nullptr;
	uint8_t* pBGR888Iter;
	const RGB565* pRGB565Iter;
	RGB565 rgb565;
	BGR888 bgr888;
	FILE* pBitmapFile;
	BITMAPFILEHEADER bmFileHeader;
	BITMAPINFOHEADER bmInfoHeader;

	area = width * height;
	paBGR888 = new uint8_t[biSizeImage];
	if (paBGR888 == nullptr)
	{
		return;
	}

	pBGR888Iter = reinterpret_cast<uint8_t*>(paBGR888);
	pRGB565Iter = paRGB565 + area;

	LONG x;
	BGR888 padding = { 0, 0, 0 };

	for (LONG y = 0; y < height; ++y)
	{
		pRGB565Iter -= width;
		for (x = 0; x < width; ++x)
		{
			rgb565 = pRGB565Iter[x];

			bgr888.Blue = (((rgb565 & 0x1F) * 527) + 23) >> 6;
			bgr888.Green = ((((rgb565 >> 5) & 0x3F) * 259) + 33) >> 6;
			bgr888.Red = ((((rgb565 >> 11) & 0x1F) * 527) + 23) >> 6;

			*(BGR888*)pBGR888Iter = bgr888;
			pBGR888Iter += sizeof(bgr888);
		}
		pBGR888Iter += stride - width * 3;
	}

	pBitmapFile = nullptr;
	_wfopen_s(&pBitmapFile, pFileName, L"wb");

	if (pBitmapFile == nullptr)
	{
		wprintf(L"open wb error");
		delete[] paBGR888;
		return;
	}
	
	bmFileHeader.bfType = 0x4D42;
	bmFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmFileHeader.bfSize = bmFileHeader.bfOffBits + (biSizeImage * sizeof(BGR888));
	bmFileHeader.bfReserved1 = 0;
	bmFileHeader.bfReserved2 = 0;
	
	bmInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmInfoHeader.biWidth = width;
	bmInfoHeader.biHeight = height;
	bmInfoHeader.biPlanes = 1;
	bmInfoHeader.biBitCount = 8 * sizeof(BGR888);
	bmInfoHeader.biCompression = 0;
	bmInfoHeader.biSizeImage = 0;
	bmInfoHeader.biXPelsPerMeter = 0;
	bmInfoHeader.biYPelsPerMeter = 0;
	bmInfoHeader.biClrUsed = 0;
	bmInfoHeader.biClrImportant = 0;

	fwrite(&bmFileHeader, sizeof(bmFileHeader), 1, pBitmapFile);
	fwrite(&bmInfoHeader, sizeof(bmInfoHeader), 1, pBitmapFile);
	fwrite(paBGR888, 1, biSizeImage, pBitmapFile);

	fclose(pBitmapFile);

	delete[] paBGR888;
}

void GetFileNameWithoutExtension(const wchar_t* pWszFilePath, wchar_t* pDstFileName)
{
	const wchar_t* pFileName;
	const wchar_t* pFileExt;
	const wchar_t* pSrc;

	pFileName = PathFindFileNameW(pWszFilePath);
	pFileExt = PathFindExtensionW(pWszFilePath);
	pSrc = pFileName;

	while (pSrc < pFileExt)
	{
		*pDstFileName++ = *pSrc++;
	}
	*pDstFileName = L'\0';
}