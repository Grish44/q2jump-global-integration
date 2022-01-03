//
// q2admin
//
// zb_util.c
//
// copyright 2000 Shane Powell
//

#include "g_local.h"

int breakLine(char *buffer, char *buff1, char *buff2, int buff2size)
{
	char *cp, *dp;

	cp = buffer;
	dp = buff1;

	while(*cp && *cp != ' ' && *cp != '\t')
	{
		*dp++ = *cp++;
	}
	*dp = 0x0;

	if(dp == buff1 || !*cp)
	{
		return 0;
	}

  SKIPBLANK(cp);

	if(*cp != '\"')
	{
		return 0;
	}
	cp++;

  cp = processstring(buff2, cp, buff2size, '\"');

	if(!buff2[0] || *cp != '\"')
	{
		return 0;
	}

	return 1;
}

int startContains(char *src, char *cmp)
{
	while(*cmp)
	{
		if(!(*src) || toupper(*src) != toupper(*cmp))
		{
			return 0;
		}

		src++;
		cmp++;
	}

	return 1;
}

int stringContains(char *buff1, char *buff2)
{
  char strbuffer1[4096];
  char strbuffer2[4096];

  strcpy(strbuffer1, buff1);
  q_strupr(strbuffer1);
  strcpy(strbuffer2, buff2);
  q_strupr(strbuffer2);
  return (strstr(strbuffer1, strbuffer2) != NULL);
}

int isBlank(char *buff1)
{
  while(*buff1 == ' ')
  {
    buff1++;
  }

  return !(*buff1);
}


char *processstring(char *output, char *input, int max, char end)
{

	while(*input && *input != end && max)
	{
		if(*input == '\\')
		{
			*input++;

			switch(*input)
			{
			case 'n':
			case 'N':
				*output++ = '\n';
				input++;
				break;

      case 'd':
      case 'D':
				*output++ = '$';
				input++;
				break;

			case 'q':
			case 'Q':
				*output++ = '\"';
				input++;
				break;

			case 's':
			case 'S':
				*output++ = ' ';
				input++;
				break;

			case 'm':
			case 'M':
			{
				int modlen = strlen(moddir);
				if(max >= modlen && modlen)
				{
					strcpy(output, moddir);
					output += modlen;
					max -= (modlen - 1);
				}
				input++;
				break;
			}

         case 't':
         case 'T':
         {
            struct tm *timestamptm;
            time_t timestampsec;
            char *timestampcp;
            int timestamplen;
            
            time( &timestampsec );                     /* Get time in seconds */
            timestamptm = localtime( &timestampsec );  /* Convert time to struct */
                                                       /* tm form */

            timestampcp = asctime( timestamptm );      /* get string version of date / time */
            timestamplen = strlen( timestampcp ) - 1;  /* length minus the '\n' */

            if(timestamplen && max >= timestamplen)
				{
					strncpy (output, timestampcp, timestamplen);
					output += timestamplen;
					max -= (timestamplen - 1);
				}
				input++;
				break;
         }

			default:
				*output++ = *input++;
				break;
			}

      max--;
		}
		else
		{
			*output++ = *input++;
      max--;
		}
	}
	
	*output = 0x0;

  return input;
}


qboolean getLogicalValue(char *arg)
{
  if(Q_stricmp(arg, "Yes") == 0 ||
      Q_stricmp(arg, "1") == 0 ||
      Q_stricmp(arg, "Y") == 0)
  {
    return true;
  }

  return false;
}


int getLastLine(char *buffer, FILE *dumpfile, long *fpos)
{
	char *bp = zbbuffer2;
	int length = 255;

	if(*fpos < 0)
	{
		return 0;
	}

	while(length && *fpos >= 0)
	{
		fseek(dumpfile, *fpos, SEEK_SET);
		(*fpos)--;

		if(fread(bp, 1, 1, dumpfile) != 1)
		{
			break;
		}
		
		if(*bp == '\n')
		{
			break;
		}

		bp++;
		length--;
	}

	if(bp != zbbuffer2)
	{
		bp--;

		// reverse string
		while(bp >= zbbuffer2)
		{
			*buffer++ = *bp--;
		}
	}
	
	*buffer = 0;
	return 1;
}


void q_strupr(char *c)
{
  while(*c)
  {
    if(islower((*c)))
    {
      *c = toupper((*c));
    }

    c++;
  }
}
