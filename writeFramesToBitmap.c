#include <stdio.h>
#include <stdlib.h> // malloc()
#include <string.h>

#define FRAME_WIDTH  359
#define FRAME_HEIGHT 757
#define MAX_LINES    5299
#define MAX_FRAMES   7

#define WHITE 1

/*
* File Write , check this link : https://www.codingunit.com/c-tutorial-binary-file-io
*
*/


typedef enum { FALSE, TRUE } boolean;

/* For 54 byte header data
*
* http://blog.paphus.com/blog/2012/08/14/write-your-own-bitmaps/
*/
#pragma pack(push,1)
typedef struct BitmapHeaderType {
	unsigned char      b0;                      // 0
	unsigned char      b1;                      // 1
	unsigned int       fullFileSize;            // 2...5
	unsigned int       reserved;                // 6...9
	unsigned int       pixelOffset;             // 10..13 HEADER SIZE
	unsigned int       bitmapInfoHeaderSize;    // 14..17 BITMAP INFO HEADER SIZE
	unsigned int       pixelWidth;              // 18..21 image width
	unsigned int       pixelHeight;             // 22..25 image height
	unsigned short int numberOfColorPlanes;     // 26..27 the number of color planes
	unsigned short int bitPerPixel;             // 28..29 24bit, 32bit, bit size
	unsigned int       compessionState;         // 30..33 compression state, 0 for disable compression
	unsigned int       sizeOfRawPixel;          // 34..37 size of pixel data including paddings (24 bit padding changes data section)
	unsigned int       horizontalResolution;    // 38..41 just leave 2835
	unsigned int       verticalResolution;      // 42..45 just leave 2835
	unsigned int       numberOfColors;          // 46..49 set 0 for default
	unsigned int       numberOfImportantColors; // 50..53 set 0 for default
} BitmapHeader;
#pragma pack(pop)

void getFrame(char frame[FRAME_WIDTH][FRAME_HEIGHT], char *bytes, int frameNum);
void handleFrameToBitmap(char frame[FRAME_WIDTH][FRAME_HEIGHT], char fileName[]);
void printFrame(char *, int);
int handleGetHeight(char *);
boolean areLineEqual(char *, char *);
void getLine(char*, char *, int);
char* loadFile(char[]);
boolean checkOnes(char[]);
boolean isZero(char);
void handleBitmapHeader(BitmapHeader *header, int width, int height);
int closestMultipleOfFour(int num);
void writeToFile(unsigned char* bytes, int len, char fileName[]);
void setPixel(unsigned char *data, int x, int y, unsigned char red, unsigned char green, unsigned char blue, int width, int height);

int main()
{
	char fileName[] = "SETI_message.txt";
	char frameFile[50] = "frameX.bmp";
	char* bytes = loadFile(fileName);
	char frame[FRAME_WIDTH][FRAME_HEIGHT];
	
	for(int i = 0; i < MAX_FRAMES; i++)
	{
		getFrame(frame, bytes, i);
		frameFile[5] = '0' + i;
		handleFrameToBitmap(frame, frameFile);
	}
	
	free(bytes);
	
	return 0;
}

void getFrame(char frame[FRAME_WIDTH][FRAME_HEIGHT], char *bytes, int frameNum)
{
	if(frameNum >= 7 || frameNum < 0)
	{
		printf("ERROR::printFrame()::Invalid Frame Num::%d\n", frameNum);
		return;
	}
	
	int frameSize = FRAME_WIDTH * FRAME_HEIGHT;
	int pivot = frameNum * frameSize;
	
	int x = 0, y = 0;
	for(int i = pivot; i < pivot + frameSize; i++)
	{
		frame[x++][y] = bytes[i]-'0';
		
		if((i+1) % FRAME_WIDTH == 0)
		{
			x=0;
			y++;
		}
	}
	
}

void handleFrameToBitmap(char frame[FRAME_WIDTH][FRAME_HEIGHT], char fileName[])
{
	BitmapHeader *header = (BitmapHeader *)malloc(sizeof(BitmapHeader));
	
	int width =  FRAME_WIDTH;
	int height = FRAME_HEIGHT;

	handleBitmapHeader(header, width, height);

	unsigned char *data = (unsigned char *)malloc(header->sizeOfRawPixel);
	unsigned char *bitmap = (unsigned char *)malloc(header->fullFileSize);

	// copy header to bitmap
	memcpy(bitmap, header, header->pixelOffset);
	
	// read frame and set pixels
	for(int j = 0; j < FRAME_HEIGHT; j++)
		for(int i = 0; i < FRAME_WIDTH; i++)
			if(frame[i][j] == 1)
				setPixel(data, i, j,   0,   255,   0  , width, height);
			else if(frame[i][j] == 0)
				setPixel(data, i, j,   0,   0,   0  , width, height);
	
	// copy data to bitmap
	memcpy(bitmap + header->pixelOffset, data, header->sizeOfRawPixel);
	
	// write	
	writeToFile((unsigned char *)bitmap, header->fullFileSize, fileName);

	free(header);
	free(data);
	free(bitmap);
}

void printFrame(char *bytes, int frameNum)
{
	if(frameNum >= 7 || frameNum < 0)
	{
		printf("ERROR::printFrame()::Invalid Frame Num::%d\n", frameNum);
		return;
	}
	
	int frameSize = FRAME_WIDTH * FRAME_HEIGHT;
	int pivot = frameNum * frameSize;
	
	for(int i = pivot; i < pivot + frameSize; i++)
	{
		putchar(bytes[i]);
		
		if((i+1) % FRAME_WIDTH == 0)
			putchar('\n');
	}
	
}

int handleGetHeight(char *bytes)
{
	int lineCounter = 0;
	char refLine[FRAME_WIDTH];
	char line[FRAME_WIDTH];
	
	// first line is reference line
	getLine(bytes, refLine, 1);
	
	// jump the first line that implies frame width
	lineCounter++;
	
	int tempLine = 1;
	while(tempLine < MAX_LINES)
	{
		getLine(bytes, line, lineCounter);
		
		if(areLineEqual(refLine, line) == FALSE)
			return lineCounter;
		else
			lineCounter++;
	}
	
}

boolean areLineEqual(char *ln1, char *ln2)
{
	for(int i = 0; i < FRAME_WIDTH; i++)
		if(ln1[i] != ln2[i])
			return FALSE;
		
	return TRUE;
}

void getLine(char* bytes, char *line, int lineNum)
{
	if(lineNum >= 5299)
	{
		printf("ERROR::getLine()::Invalid Line Number::%d\n", lineNum);
		return;
	}
	
	int pivot = lineNum * FRAME_WIDTH;
	
	for(int i = 0, pivot = lineNum * FRAME_WIDTH; i < FRAME_WIDTH; i++, pivot++)
		line[i] = bytes[pivot];	
}

char* loadFile(char fileName[])
{
	FILE *file = fopen(fileName, "r");
	
	char *bytes = (char *)malloc(sizeof(char) * FRAME_WIDTH * MAX_LINES );
	char *c = bytes;
	
	while((*c = fgetc(file)) != EOF)
		c++;
	
	return bytes;
}

// Check if all chars of a frame line is 1
boolean checkOnes(char line[])
{
	for(int i = 0; i < FRAME_WIDTH; i++)
		if(isZero(line[i]))
			return TRUE;
		
	return FALSE;
}

boolean isZero(char c)
{
	if(c == '0')
		return TRUE;
	
	return FALSE;
}

void handleBitmapHeader(BitmapHeader *header, int width, int height)
{
	// Calculate row with padding
	int rowWithPadding = closestMultipleOfFour(width * 3);
	int rawSize = height * rowWithPadding;
	printf("Row With Padding : %4d\n", rowWithPadding);
	printf("Raw Size : Decimal[%4d], Hex[%4x]\n", rawSize, rawSize);

	header->b0 = 'B';
	header->b1 = 'M';
	header->fullFileSize = rawSize + 54;
	header->reserved = 0x00000000;
	header->pixelOffset = 54;
	header->bitmapInfoHeaderSize = 40;
	header->pixelWidth = (unsigned int)width;
	header->pixelHeight = (unsigned int)height;
	header->numberOfColorPlanes = 1;
	header->bitPerPixel = 24;
	header->compessionState = 0;
	header->sizeOfRawPixel = (unsigned int)rawSize;
	header->horizontalResolution = 0x2835;
	header->verticalResolution = 0x2835;
	header->numberOfColors = 0;
	header->numberOfImportantColors = 0;
}

int closestMultipleOfFour(int num)
{
	return (num + 3) & ~0x03;
}

void writeToFile(unsigned char* bytes, int len, char fileName[])
{

	FILE *filePtr;

	printf("HIT A : len = %d\n", len);

	filePtr = fopen(fileName, "wb");	// wb for write binary	

	printf("HIT B\n");

	if (!filePtr)
	{
		printf("ERROR::writeToFile()::Unable to open file : \"%s\"\n", fileName);
		return;
	}

	fwrite(bytes, len, 1, filePtr);

	fclose(filePtr);
}

void setPixel(unsigned char *data, int x, int y, unsigned char red, unsigned char green, unsigned char blue, int width, int height)
{
	y = height - 1 - y;

	int index = 0;

	if (x < width && y == 0)	 // no need to calculate padding
		index = x * 3;
	else
	{
		boolean isPadding = ((width * 3) % 4 == 0) ? FALSE : TRUE;

		if (isPadding == TRUE) {
			int padWidth = closestMultipleOfFour(width*3);
			index = y*padWidth + x*3;
		}
		else {
			index = (y*width + x) * 3;
		}
	}

	int bytes = 0;
	*(data + index + bytes++) = blue;
	*(data + index + bytes++) = green;
	*(data + index + bytes) = red;

}


