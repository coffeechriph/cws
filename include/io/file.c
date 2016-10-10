#include "file.h"

char *cwsReadFile(const char *file, i32 *length)
{
	FILE *f = fopen(file, "r");
	if(f == NULL)
	{
		cws_log("Couldn't open file %s", file);
		return NULL;
	}

	u32 lng = 0;

	fseek(f, 0, SEEK_END);
	lng = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buffer = (char*)malloc(sizeof(char)*lng);
	size_t read_l = fread(buffer, sizeof(char), lng, f);
	if(read_l != lng)
	{
		cws_log("Error file %s was not read correctly!", file);
	}

	fclose(f);

	(*length) = lng;
	return buffer;
}

bool cwsOpenFile(const char *file)
{
	FILE *f = fopen(file, "w");
	if(f == NULL)
	{
		return false;
	}

	fclose(f);
	return true;
}

bool cwsCloseFile(const char *file)
{
	return remove(file) == 0;
}
