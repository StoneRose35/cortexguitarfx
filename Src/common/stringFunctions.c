
/**
 * @file stringFunctions.c
 * @author Philipp	Fuerholz (fuerholz@gmx.ch)
 * @brief contains various string conversion functions
 * @version 0.1
 * @date 2021-12-23
 * 
 * 
 */
#include <stdint.h>
#include "stringFunctions.h"
#include <string.h>
#include <stdlib.h>

/**
 * @brief converts a number from 0 1 into a string showing percent from 0 to 100. The precision is fixed to three digits.
 * 
 * @param percentVal the value to convert
 * @param out the character array holding the converted string, must be initialized and of length 8 minimum
 */
void toPercentChar(float percentVal,char * out)
{
	uint32_t ival = (uint32_t)(percentVal*100000.0);
	UInt32ToChar(ival,out);
	fillWithLeadingZeros(4,out);
	//re-insert comma
	uint8_t str_len = 0;
	char swap, swap2;
	while (*(out+str_len) != 0)
	{
		str_len++;
	}
	swap = out[str_len-3];
	out[str_len-3] = '.';
	for(uint8_t c=str_len-2;c<str_len+1;c++)
	{
		swap2 = out[c];
		out[c] = swap;
		swap = swap2;
	}
	out[str_len+1] = 0;
}

void fixedPointInt16ToChar(char * str,uint16_t nr,uint8_t fracDecimals)
{
	uint32_t fracBase=1;
	uint8_t c=0,fracLength=0,strPtr;
	uint32_t fracNr;
	uint16_t intNr;
	char fracStr[16];
	strPtr=0;

	if ((nr & 0x8000) != 0) // fill up with ones if the sign bit is set
	{
		nr = (nr ^ 0xFFFF) + 1;
		*(str+strPtr++) = '-';
	}

	for(c=0;c<fracDecimals;c++)
	{
		fracBase *= 5;
	}
	fracNr = (nr &  ((1 << fracDecimals) - 1))*fracBase;
	UInt32ToChar(fracNr,fracStr);
	c=0;
	while(*(fracStr+c) != 0)
	{
		fracLength++;
		c++;
	}
	intNr = nr >> fracDecimals;


	Int16ToChar(intNr,str+strPtr);
	c=0;

	while(c<fracDecimals+1)
	{
		if (*(str+strPtr) == 0 && c==0)
		{
			*(str+strPtr) = '.';
			c++;
		}
		else if ( c > 0 && c < (fracDecimals - fracLength) + 1)
		{
			*(str+strPtr) = '0';
			c++;
		}
		else if (c>0)
		{
			*(str+strPtr) = *(fracStr + c - 1);
			c++;
		}
		strPtr++;
	}
	*(str+strPtr) = 0;
}


void fixedPointUInt16ToChar(char * str,uint16_t nr,uint8_t fracDecimals)
{
	uint32_t fracBase=1;
	uint8_t c=0,fracLength=0,strPtr;
	uint16_t fracNr,intNr;
	char fracStr[16];
	for(c=0;c<fracDecimals;c++)
	{
		fracBase *= 5;
	}
	fracNr = (nr &  ((1 << fracDecimals) - 1))*fracBase;
	UInt16ToChar(fracNr,fracStr);
	c=0;
	while(*(fracStr+c) != 0)
	{
		fracLength++;
		c++;
	}
	intNr = nr >> fracDecimals;
	UInt16ToChar(intNr,str);
	c=0;
	strPtr=0;
	
	while(c<fracDecimals+1)
	{
		if (*(str+strPtr) == 0 && c==0)
		{
			*(str+strPtr) = '.';
			c++;
		}
		else if ( c > 0 && c < (fracDecimals - fracLength) + 1)
		{
			*(str+strPtr) = '0';
			c++;
		}
		else if (c>0)
		{
			*(str+strPtr) = *(fracStr + c - 1);
			c++;
		}
		strPtr++;
	}
	*(str+strPtr) = 0;
}

/**
 * @brief converts a uint8_t to a string
 * 
 * @param nr the number to convert
 * @param out the character array,must be initialized and of length 4 minimum
 */
void UInt8ToChar(uint8_t nr, char * out)
{
	uint8_t pos=100;
	uint8_t cntr=0,charpos=0;
	uint8_t firstDigit = 0;
	if (nr==0)
	{
		out[charpos++]=0x30;
	}
	else
	{
		while (pos > 0)
		{
			cntr=0;
			while (nr >= pos)
			{
				nr -= pos;
				cntr++;
			}
			if (cntr > 0 || firstDigit > 0)
			{
				out[charpos++] = cntr + 0x30;
				firstDigit = 1;
			}
			pos /= 10;
		}
	}
	out[charpos]=0;
}

/**
 * @brief converts a uint16_t to a string
 * 
 * @param nr the number to convert
 * @param out the character array,must be initialized and of length 6 minimum
 */
void UInt16ToChar(uint16_t nr, char * out)
{
	uint16_t pos=10000;
	uint16_t cntr=0,charpos=0;
	uint16_t firstDigit = 0;
	if (nr==0)
	{
		out[charpos++]=0x30;
	}
	else
	{
		while (pos > 0)
		{
			cntr=0;
			while (nr >= pos)
			{
				nr -= pos;
				cntr++;
			}
			if (cntr > 0 || firstDigit > 0)
			{
				out[charpos++] = cntr + 0x30;
				firstDigit = 1;
			}
			pos /= 10;
		}
	}
	out[charpos]=0;
}

void decimalInt16ToChar(int16_t nr,char * out,uint8_t decimalPlace)
{
	uint16_t pos=10000;
	uint16_t cntr=0,charpos=0;
	uint16_t firstDigit = 0;
	uint16_t interm_nr;
	uint8_t c2;
	if (nr==0)
	{
		out[charpos++]=0x30;
	}
	else
	{
		if (((uint16_t)nr & 0x8000) != 0)
		{
			interm_nr = (nr ^ 0xFFFF) + 1;
			//nr++; 
			out[charpos++] = '-';
		}
		else
		{
			interm_nr = nr;
		}
		while (pos > 0)
		{
			cntr=0;
			while (interm_nr >= pos)
			{
				interm_nr -= pos;
				cntr++;
			}
			if (cntr > 0 || firstDigit > 0)
			{
				out[charpos++] = cntr + 0x30;
				firstDigit = 1;
			}
			pos /= 10;
		}
	}
	out[charpos]=0;
	if (((uint16_t)nr & 0x8000) != 0)
	{
		// have to add least one zero before the decimal separator
		if(charpos < decimalPlace+3)
		{
			// shift digits back
			c2=charpos;
			for(uint8_t c=charpos;c>0;c--)
			{
				out[c2+(decimalPlace-charpos)+3] = out[c2];
				c2--;
			}

			// fill with zeros
			for(uint8_t c=0;c<(decimalPlace-charpos)+3;c++)
			{
				if (c==1)
				{
					out[c+1] = '.';
				}
				else
				{
					out[c+1] = '0';
				}
			}
		}
		else
		{
			// shift one position back
			c2=charpos;
			for(uint8_t c=0;c<decimalPlace;c++)
			{
				out[c2+1] = out[c2];
				c2--;
			}
			// put in decimal separator
			out[charpos-decimalPlace]='.';
		}
	}
	else
	{
		// have to add least one zero before the decimal separator
		if(charpos < decimalPlace+2)
		{
			// shift digits back
			c2=charpos;
			for(uint8_t c=charpos;c>=0;c--)
			{
				out[c2+(decimalPlace-charpos)+2] = out[c2];
				c2--;
			}

			// fill with zeros
			for(uint8_t c=0;c<(decimalPlace-charpos)+2;c++)
			{
				if (c==1)
				{
					out[c+1] = '.';
				}
				else
				{
					out[c+1] = '0';
				}
			}
		}
		else
		{
			// shift one position back
			c2=charpos;
			for(uint8_t c=0;c<decimalPlace;c++)
			{
				out[c2+1] = out[c2];
				c2--;
			}
			// put in decimal separator
			out[charpos-decimalPlace]='.';
		}
	}

}

void Int16ToChar(int16_t nr, char * out)
{
	uint16_t pos=10000;
	uint16_t cntr=0,charpos=0;
	uint16_t firstDigit = 0;
	uint16_t interm_nr;
	if (nr==0)
	{
		out[charpos++]=0x30;
	}
	else
	{
		if (((uint16_t)nr & 0x8000) != 0)
		{
			interm_nr = (nr ^ 0xFFFF) + 1;
			nr++; 
			out[charpos++] = '-';
		}
		else
		{
			interm_nr = nr;
		}
		while (pos > 0)
		{
			cntr=0;
			while (interm_nr >= pos)
			{
				interm_nr -= pos;
				cntr++;
			}
			if (cntr > 0 || firstDigit > 0)
			{
				out[charpos++] = cntr + 0x30;
				firstDigit = 1;
			}
			pos /= 10;
		}
	}
	out[charpos]=0;
}

/**
 * @brief converts a uint32_t to a string
 * 
 * @param nr the number to convert
 * @param out the character array,must be initialized and of length 11 minimum
 */
void UInt32ToChar(uint32_t nr, char * out)
{
	uint32_t pos=1000000000;
	uint32_t cntr=0,charpos=0;
	uint32_t firstDigit = 0;
	if (nr==0)
	{
		out[charpos++]=0x30;
	}
	else
	{
		while (pos > 0)
		{
			cntr=0;
			while (nr >= pos)
			{
				nr -= pos;
				cntr++;
			}
			if (cntr > 0 || firstDigit > 0)
			{
				out[charpos++] = cntr + 0x30;
				firstDigit = 1;
			}
			pos /= 10;
		}
	}
	out[charpos]=0;
}

/**
 * @brief converts a 32bit unsigned int to a hex representation, fox example UInt32ToHex(123,nrbfr)
 * would put "0x7b" in nrbfr
 * @param val the unsigned int value to convert
 * @param nrbfr the buffer to hold the string representation, must be at least 11 chars in size
 */
void UInt32ToHex(uint32_t val,char* nrbfr)
{
	uint32_t cval = val;
	uint32_t nibbleval;
	uint8_t charcnt=0;
	if (val == 0)
	{
		nrbfr[0] = '0';
		nrbfr[1] = 'x';
		nrbfr[2] = '0';
		charcnt = 3;
	}
	else
	{
		nrbfr[0] = '0';
		nrbfr[1] = 'x';
		charcnt = 2;
		while (charcnt < 10)
		{
			// shift out left, discard leading zeros
			nibbleval = (cval >> 28) & 0xF;
			if (nibbleval < 10 ) 
			{
				nrbfr[charcnt++] = nibbleval + 0x30;
			}
			else
			{
				nrbfr[charcnt++] = nibbleval + 0x61 - 10;
			}
			cval = cval <<  4;
		}
	}
	nrbfr[charcnt] = 0;
}



/**
 * @brief add zeros on the left side on a string representing an integer number, used by toPercentChar
 * 
 * @param minlength the minimum length the string should have
 * @param nr the array containing the string representation of the integer number, note the extra space must be allocated
 */
void fillWithLeadingZeros(uint8_t minlength,char * nr)
{
	uint8_t nrlen=0;
	while(nr[nrlen] != 0)
	{
		nrlen++;
	}
	if (nrlen < minlength)
	{
		for(uint8_t c2=0;c2<minlength-nrlen;c2++)
		{
			for(uint8_t c3=nrlen+c2;c3>c2;c3--)
			{
				nr[c3]=nr[c3-1];
			}
			nr[c2]=0x30;
		}
		nr[minlength] = 0;
	}
}

/**
 * @brief checks if one string start with another
 * 
 * @param ptrn the pattern to check
 * @param target the string for which should be checked if it start with ptrn
 * @return 1 if target start with ptrn, 0 otherwise
 */
uint8_t startsWith(char* ptrn,const char* target)
{
	uint8_t cnt=0,isEqual=1;
	while (ptrn[cnt]>0 && target[cnt]>0 && isEqual > 0)
	{
		if (ptrn[cnt] != target[cnt])
		{
			isEqual=0;
		}
		cnt++;
	}
	return isEqual;
}

/**
 * @brief converts the string to uppercase letters
 * 
 * @param str 
 */
void toUpper(char * str,char endchar)
{
	uint16_t c=0;
	uint8_t endcharFound=0;
	while((uint8_t)*(str+c) != 0 && endcharFound == 0)
	{
		if ((uint8_t)*(str+c) >=97 && (uint8_t)*(str+c) <=122)
		{
			*((uint8_t*)str+c)=(uint8_t)*(str+c) - 32;
		}
		c++;
		if(endchar > 0)
		{
			endcharFound = *(str+c) == endchar;
		}
	}
}

/**
 * @brief convert a string to an uint8_t, doesn't do any validity check on the string
 * 
 * @param chr the string convert
 * @return the string as a uint8_t
 */
uint8_t toUInt8(char * chr)
{
	uint8_t res=0;
	uint8_t cnt=0;
	while (chr[cnt]!=0)
	{
		res *= 10;
		res += (chr[cnt] - 0x30);
		cnt++;
	}
	return res;
}

/**
 * @brief convert a string to an uint32_t, doesn't do any validity check on the string
 * 
 * @param chr the string convert
 * @return the string as a uint32_t
 */
uint32_t toUInt32(char * chr)
{
	uint32_t res=0;
	uint32_t cnt=0;
	while (chr[cnt]!=0)
	{
		res *= 10;
		res += (chr[cnt] - 0x30);
		cnt++;
	}
	return res;
}

/**
 * @brief convert a string to an int16_t, doesn't do any validity check on the string
 * 
 * @param chr the string convert
 * @return the string as a int16_t
 */
int16_t toInt16(char * chr)
{
	int16_t res=0;
	int16_t cnt=0;
	uint8_t hasMinus = 0;
	if(chr[cnt] == '-')
	{
		hasMinus = 1;
		cnt++;
	}
	while (chr[cnt]!=0)
	{
		res *= 10;
		res += (chr[cnt] - 0x30);
		cnt++;
	}
	if (hasMinus == 1)
	{
		res = -res;
	}
	return res;
}


int16_t toUInt16(char * chr)
{
	uint16_t res=0;
	uint16_t cnt=0;
	while (chr[cnt]!=0)
	{
		res *= 10;
		res += (chr[cnt] - 0x30);
		cnt++;
	}
	return res;
}

/**
 * @brief returns the content within the brackts of a string. Doens't do any specific validity checks. For example for "RGB(123,32,34,0-17)"
 * the function would return "123,32,34,0-17"
 * 
 * @param input the string containing exactly one set of brackett: "()"
 * @param out the content within the brackets
 */
void getBracketContent(const char* input,char * out)
{
	uint8_t cnt=0,bcnt=0;
	uint8_t idxStart=255,idxEnd=0;
	char cur = *(input+cnt);
	while(cur != 0)
	{
		if (idxStart==255)
		{
			if (cur == '(')
			{
				idxStart = cnt;
			}
		}
		else if (cur != ')')
		{
			out[bcnt]=cur;
			bcnt++;
		}
		if (idxEnd==0)
		{
			if (cur == ')')
			{
				idxEnd = cnt;
			}
		}

		cnt++;
		cur = *(input+cnt);
	}
	out[bcnt]=0;
	if (bcnt==0 || idxEnd==0)
	{
		out = 0;
	}
}

/**
 * @brief removes all whitespace characters (Tab, Space) from a string
 * 
 * @param input the string for which the whitespace should be removed, note that the operation happens in-place and that the character after the terminating 0 are not zeroed
 */
void stripWhitespaces(char * input)
{

	uint8_t c1=0, c2=0;
	while(*(input+c1+c2) !=0)
	{
		if (*(input+c1)==0x20 || *(input+c1)==0x9)
		{
			c2++;
		}
		else
		{
			c1++;
		}
		*(input+c1)=*(input+c1+c2);
	}
	if(c2 > 0)
	{
		c1++;
		*(input+c1) = 0;
	}
}

/**
 * @brief expands a range description from uint8_t values into a uint8_t array. The first number must be smaller than the seconds one. Also the string should no contains whitespaces.
 * 
 * @param stringinput the string which should be expanded
 * @param result pointer to the uint8_t array, the array itself is initialized dynamically within the function
 * @return the length of the array, 0 if no range has been input
 */
uint8_t expandRange(char * stringinput,uint8_t ** result)
{
	char nr[4];
	uint8_t lowerBound=0;
	uint8_t upperBound=0;
	uint8_t c=0,c1=0;
	int len=0;
	while(*(stringinput+c) != '-' && *(stringinput+c) != 0)
	{
		nr[c1++]=stringinput[c++];
	}
	nr[c1]=0;

	lowerBound = toUInt8(nr);
	if(stringinput[c]!=0)
	{
		c++;
		c1=0;
		while(stringinput[c] != 0)
		{
			nr[c1++]=stringinput[c++];
		}
		nr[c1]=0;
		upperBound=toUInt8(nr);
	} else
	{
		upperBound=lowerBound;
	}

	len = (int)(upperBound-lowerBound+1);
	*result = (uint8_t*)malloc(len);
	for(uint8_t c=0;c<len;c++)
	{
		*(*(result) + c) = lowerBound + c;
	}
	return len;
}

void timeToString(char * bfr,uint8_t h,uint8_t m,uint8_t s)
{

    uint8_t strPos=0;
    char nrbfr[8];
    uint8_t c;

    if (h < 10)
    {
        *(bfr + strPos++) = '0';
    }
    UInt8ToChar(h,nrbfr);
    c=0;
    while(*(nrbfr+c) != 0)
    {
        *(bfr + strPos++) = *(nrbfr + c++);
    }
    *(bfr + strPos++) = ':';
    if (m < 10)
    {
        *(bfr + strPos++) = '0';
    }
    UInt8ToChar(m,nrbfr);
    c=0;
    while(*(nrbfr+c) != 0)
    {
        *(bfr + strPos++) = *(nrbfr + c++);
    }
    *(bfr + strPos++) = ':';
    if (s < 10)
    {
        *(bfr + strPos++) = '0';
    }
    UInt8ToChar(s,nrbfr);
    c=0;
    while(*(nrbfr+c) != 0)
    {
        *(bfr + strPos++) = *(nrbfr + c++);
    }
    *(bfr + strPos) = 0;
}

void dateToString(char * bfr, uint16_t y,uint8_t month, uint8_t d)
{
	uint8_t strPos=0;
    char nrbfr[8];
    uint8_t c;
    UInt8ToChar(d,nrbfr);
	if(d<10)
	{
		*(bfr+strPos++)='0';
	}
    c=0;
    while(*(nrbfr+c) != 0)
    {
        *(bfr + strPos++) = *(nrbfr + c++);
    }
    *(bfr + strPos++) = '.';
    UInt8ToChar(month,nrbfr);
	if(month<10)
	{
		*(bfr+strPos++)='0';
	}
    c=0;
    while(*(nrbfr+c) != 0)
    {
        *(bfr + strPos++) = *(nrbfr + c++);
    }
    *(bfr + strPos++) = '.';
    UInt16ToChar(y,nrbfr);
    c=0;
    while(*(nrbfr+c) != 0)
    {
        *(bfr + strPos++) = *(nrbfr + c++);
    }
    *(bfr + strPos) = 0;
}

void dateTimeToString(char * out,uint16_t year,uint8_t month,uint8_t day,uint8_t hour,uint8_t minute,uint8_t seconds)
{
	char bfr[12];
    dateToString(bfr,year,month,day);
    uint8_t c;
    uint8_t strPos=0;
    c=0;
    while(*(bfr + c) != 0)
    {
        *(out + strPos++) = *(bfr + c++);
    }
    c=0;
    timeToString(bfr,hour,minute,seconds);
    *(out + strPos++) = ' ';
    *(out + strPos++) = ' ';
    while(*(bfr + c) != 0)
    {
        *(out + strPos++) = *(bfr + c++);
    }
     *(out + strPos) = 0;
}

uint16_t appendToString(char * appendee,char *  appender)
{
	uint16_t c=0,c2=0;
	while (*(appendee + c) != 0)
	{
		c++;
	}
	while(*(appender + c2) != 0)
	{
		*(appendee + c++) = *(appender + c2++); 
	}
	*(appendee + c) = 0;
	return c;
}
