#ifndef _SH_STRING_FNS_H_
#define _SH_STRING_FNS_H_

#ifdef __cplusplus
extern "C" {
#endif

static inline __attribute__((always_inline))  int sh_strsplit(char *string, int stringlen, char **tokens, int maxtokens, char delim)
{
	int i, tok = 0;
	int tokstart = 1; /* first token is right at start of string */

	if (!string || !tokens)
		return 0;

	for (i = 0; i < stringlen; i++) 
	{
		if (string[i] == '\0' || tok >= maxtokens)
		{
			break;
		}
		if (tokstart) 
		{
			if(string[i] == delim)
			{
				continue;
			}
			tokstart = 0;
			tokens[tok++] = &string[i];
		}
		if (string[i] == delim) 
		{
			string[i] = '\0';
			tokstart = 1;
		}
	}
	return tok;
}

#ifdef __cplusplus
}
#endif

#endif