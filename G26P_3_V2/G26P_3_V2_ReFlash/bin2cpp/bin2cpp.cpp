// bin2cpp.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <windows.h>


static const char* sFileName = 0;
static const char* dFileName = 0;

static char dFNStr[1024];

/*static bool CreateDestFileName(const char *n)
{
	int i = strlen(n) - 1;

	while(i >= 0 && (n[i] == '"' || n[i] == ' '))
	{
		i--;
	};

	if (i < 0) { return false; };

	int e = i;

	while (i >= 0)
	{
		if (n[i] == '/' || n[i] == '\\')
		{
			i++;
			break;
		}
		else
		{
			i--;
		};
	};

	if (i < 0) { i = 0; };

	return true;
}*/

int main(int argc, const char* argv[])
{
	if (argc < 3)
	{
		return 1;
	};

	sFileName = argv[1];
	dFileName = argv[2];

	HANDLE sfh;
	HANDLE sfmh;
	DWORD *p = 0;
	DWORD sflen = 0;
	FILE *df;
	
	sfh = CreateFile(sFileName, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);

	if (sfh != INVALID_HANDLE_VALUE)
	{
		sflen = SetFilePointer(sfh, 0, 0, FILE_END);

		sfmh = CreateFileMapping(sfh, 0, PAGE_READONLY, 0, 0, 0);

		if (sfmh != 0)
		{
			p = (DWORD*)MapViewOfFile(sfmh, FILE_MAP_READ, 0, 0, 0);
		};
	};

	if (sfh == INVALID_HANDLE_VALUE || sfmh == 0 || p == 0)
	{
		printf("Error open file %s\r\n", sFileName);
		return 2;
	};

	if (fopen_s(&df, dFileName, "w") != 0)
	{
		printf("Error create file %s\r\n", dFileName);
		return 3;
	};

	DWORD *sp = p;

	int l = (sflen+3) >> 2;

	int i = 0;

	while (i < l)
	{
		for (int j = 0; j < 16 && i < l; i++, j++)
		{
			if ((i+1) < l) 
			{
				fprintf(df, " 0x%08lX,", *sp);
			}
			else
			{
				fprintf(df, " 0x%08lX", *sp);
			};

			sp++;
		};

		fprintf(df, "\n");
	};


	return 0;
}

