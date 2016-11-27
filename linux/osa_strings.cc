#include "osa.h"
#include <string.h>

ret_e osa_strlen(char * str, u32_t &len)
{
	char * func = "osa_strlen";
	if(NULL == str)
	{
		osa_loge("%s: Input string is NULL", func);
		return OSA_ERR_BADPARAM;
	}

	len = strlen(str);

	return OSA_SUCCESS;
}

/* copy string src to dst. Max (dstSz-1) bytes will be copied */
ret_e osa_strcpy(char *dst, char * src, i32_t dstSz)
{
	char * func = "osa_strcpy";
	i32_t srcLen=0;

	if(NULL == dst)
	{
		osa_loge("%s:err:Destination Buffer is NULL", func);
		return OSA_ERR_BADPARAM;
	}

	if(NULL == src)
	{
		osa_loge("%s:err:Source Buffer is NULL", func);
		return OSA_ERR_BADPARAM;
	}

	srcLen = strlen(src);
	if(srcLen > dstSz-1)
	{
		osa_logd("%s:warning: dest buffer (%d) is smaller than src string (%d). Only %d bytes will be copied", 
					func, dstSz, srcLen, dstSz-1);
	}

	strncpy(dst, src, dstSz-1);

	return OSA_SUCCESS;
}

/* copy n bytes from src to dst. Max(dstSz-1) bytes will be copied */
ret_e osa_strncpy(char * dst, char *src, i32_t n, i32_t dstSz)
{
	char * func = "osa_strncpy";
	i32_t srcLen=0;

	/*##############
	  ############## */
	if(NULL == dst)
	{
		osa_loge("%s:err:Destination Buffer is NULL", func);
		return OSA_ERR_BADPARAM;
	}

	if(NULL == src)
	{
		osa_loge("%s:err:Source Buffer is NULL", func);
		return OSA_ERR_BADPARAM;
	}

	srcLen = strlen(src);
	if(n > srcLen)
	{
		osa_loge("%s:err:n (%d) is greater than src len (%d)", func, n, srcLen);
		return OSA_ERR_BADPARAM;
	}

	if(n > dstSz-1)
	{
		osa_logd("%s:warning: dest buffer (%d) is smaller than n (%d). Only %d bytes will be copied", dstSz, n, dstSz-1);
	}
	/*##############
	  ############## */

	strncpy(dst, src, dstSz-1);

	return OSA_SUCCESS;
}


/* Compares strings s1 and s2.
	Returns an integer less than, equal to, or greater than zero if s1 (or the first n bytes thereof) is found,  respectively,
       to be less than, to match, or be greater than s2 
   IMPORTANT: If either of the strings s1, s2 are NULL, the behavior is undefined
 */
i32_t osa_strcmp(char *s1, char *s2)
{
	return strcmp(s1,s2);
}

/* Compares first n bytes of strings s1 and s2.
	Returns an integer less than, equal to, or greater than zero if s1 (or the first n bytes thereof) is found,  respectively,
       to be less than, to match, or be greater than s2 
    IMPORTANT: If either of the strings s1, s2 are NULL, the behavior is undefined
 */
i32_t osa_strncmp(char *s1, char *s2, i32_t n)
{
	return strncmp(s1,s2, n);
}


/* Compares strings s1 and s2 --  ignoring case.
	Returns an integer less than, equal to, or greater than zero if s1 (or the first n bytes thereof) is found,  respectively,
       to be less than, to match, or be greater than s2 
   IMPORTANT: If either of the strings s1, s2 are NULL, the behavior is undefined
 */
i32_t osa_strcasecmp(char *s1, char *s2)
{
	return strcasecmp(s1,s2);
}

/* Compares first n bytes of strings s1 and s2 --  ignoring case.
	Returns an integer less than, equal to, or greater than zero if s1 (or the first n bytes thereof) is found,  respectively,
       to be less than, to match, or be greater than s2 
    IMPORTANT: If either of the strings s1, s2 are NULL, the behavior is undefined
 */
i32_t osa_strncasecmp(char *s1, char *s2, i32_t n)
{
	return strncasecmp(s1,s2, n);
}


ret_e osa_strcat(char *dst, char *src, i32_t dstSz)
{
	char * func = "osa_strcat";
	i32_t srcLen=0, dstLen=0;

	/*##############
	  ############## */
	if(NULL == dst)
	{
		osa_loge("%s:err:Destination Buffer is NULL", func);
		return OSA_ERR_BADPARAM;
	}

	if(NULL == src)
	{
		osa_loge("%s:err:Source Buffer is NULL", func);
		return OSA_ERR_BADPARAM;
	}
	
	srcLen = strlen(src);
	dstLen = strlen(dst);

	if(srcLen+dstLen+1 > dstSz)
	{
		osa_loge("%s:err:The concanated string is bigger (%d) than dstSz (%d)", func, srcLen+dstLen+1, dstSz);
		return OSA_ERR_INSUFFMEM;
	}
	/*##############
	  ############## */

	strcat(dst, src);

	return OSA_SUCCESS;
}

ret_e osa_strncat(char *dst, char *src, i32_t n, i32_t dstSz)
{
	char * func = "osa_strncat";
	i32_t srcLen=0, dstLen=0;
	
	/*##############
	  ############## */
	if(NULL == dst)
	{
		osa_loge("%s:err:Destination Buffer is NULL", func);
		return OSA_ERR_BADPARAM;
	}

	if(NULL == src)
	{
		osa_loge("%s:err:Source Buffer is NULL", func);
		return OSA_ERR_BADPARAM;
	}
	
	srcLen = strlen(src);
	if(n > srcLen)
	{
		osa_loge("%s:err:n (%d) is greater than src len (%d)", func, n, srcLen);
		return OSA_ERR_BADPARAM;
	}

	dstLen = strlen(dst);

	if(n+dstLen+1 > dstSz)
	{
		osa_loge("%s:err:The concanated string is bigger (%d) than dstSz (%d)", func, n+dstLen+1, dstSz);
		return OSA_ERR_INSUFFMEM;
	}
	/*##############
	  ############## */

	strncat(dst, src, n);

	return OSA_SUCCESS;
}


char * osa_strchr(char * s1, int c)
{
	char * func = "osa_strchr";
	/*##############
	  ############## */
	if(NULL == s1)
	{
		osa_loge("%s:err:String is NULL", func);
		return NULL;
	}
	/*##############
	  ############## */

	return strchr(s1, c);

}

char * osa_index(char *s1, int c)
{
	char * func = "osa_index";

	/*##############
	  ############## */
	if(NULL == s1)
	{
		osa_loge("%s:err:String is NULL", func);
		return NULL;
	}
	/*##############
	  ############## */

	return index(s1,c);
}

char * osa_strstr(char * haystack, char * needle)
{
	char * func = "osa_strstr";
	/*##############
	  ############## */
	if(NULL == haystack)
	{
		osa_loge("%s:err:haystack is NULL", func);
		return NULL;
	}

	if(NULL == needle)
	{
		osa_loge("%s:err:needle is NULL", func);
		return NULL;
	}
	/*##############
	  ############## */

	return strstr(haystack, needle);
}