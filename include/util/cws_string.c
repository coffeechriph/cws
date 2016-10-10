#include "cws_string.h"

void cws_string_build(cws_string *s, const char *s2)
{
	if(s->data != NULL)
	{
		free(s->data);
	}

	u32 lng = 0;
	while(*(s2+lng) != '\0')
	{
		lng++;
	}

	s->data = (char*)malloc(sizeof(char)*lng+1);
	if(s->data == NULL)
	{
		s->length = 0;
		cws_log("Error allocating memory for cws_string!");
		return;
	}

	s->length = lng;
	for(u32 i = 0; i < lng; ++i)
	{
		s->data[i] = s2[i];
	}
	s->data[lng] = '\0';
}

void cws_string_copy(cws_string *dest, cws_string *src)
{
	if(dest->length == src->length)
	{
		for(u32 i = 0; i < src->length; ++i)
		{
			dest->data[i] = src->data[i];
		}
	}
	else
	{
		if(dest->data != NULL)
		{
			free(dest->data);
		}
		
		dest->data = malloc(sizeof(char)*src->length+1);
		if(dest->data == NULL)
		{
			cws_log("Error allocating memory for cws_string!");
			return;
		}

		dest->length = src->length;
		for(u32 i = 0; i < src->length+1; ++i)
		{
			dest->data[i] = src->data[i];
		}
	}
}

void cws_string_free(cws_string* s)
{
	s->length = 0;

	if(s->data != NULL)
	{
		free(s->data);
	}
}