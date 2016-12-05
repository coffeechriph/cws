#include "cws_string.h"

//Allocates a new string and copies the data from src into the new returned buffer
//This returned buffer must be deleted!
char* copy_conststr(const char *src, i32 src_length)
{
    char *copy = (char*)malloc(sizeof(char) * src_length+1);
    for(i32 i = 0; i < src_length; ++i)
    {
        copy[i] = src[i];
    }
    
    copy[src_length] = '\0';
    return copy;
}

void cws_str_init(cws_str *s)
{
    *s = (cws_str){.length = 0,.data = NULL};
}

void cws_str_build(cws_str *s, const char *s2)
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

void cws_str_append(cws_str *dest, const char *s2)
{
    if(s2 == NULL || dest == NULL)
    {
        return; 
    }
    
    if(dest->data == NULL)
    {
        cws_str_build(dest, s2);
    }
    else
    {
        char *nw = realloc(dest->data, dest->length + strlen(s2));
        if(nw)
        {
            dest->data = nw;
            strcpy(dest->data + dest->length, s2);
            dest->length += strlen(s2);
        }
        else
        {
            cws_log("Error reallocating memory for cws_str!");
        }
    }
}

void cws_str_copy(cws_str *dest, cws_str *src)
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

bool cws_str_cmp(cws_str *s1, cws_str *s2)
{
    if(s1->length != s2->length)
    {
        return false;
    }
    
    for(u32 i = 0; i < s1->length; ++i)
    {
        if(s1->data[i] != s2->data[i])
        {
            return false;
        }
    }
    
    return true;
}

void cws_str_free(cws_str* s)
{
	s->length = 0;

	if(s->data != NULL)
	{
		free(s->data);
	}
}