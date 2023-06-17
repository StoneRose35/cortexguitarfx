#include "graphics/gfxfont.h"

#include "graphics/bwgraphics.h"
#include "stdlib.h"
#ifdef RP2040_FEATHER
#include "romfunc.h"
#else
#include "math.h"
float fsqrt(float a)
{
    return sqrtf(a);
}

int32_t float2int(float a)
{
    return (int32_t)a;
}

float int2float(int32_t a)
{
    return (float)a;
}
float fcos(float x)
{
	return cosf(x);
}
float fsin(float x)
{
	return sinf(x);
}

#endif

extern const uint8_t oled_font_5x7[98][5];

void drawLine(float spx,float spy,float epx, float epy,BwImageType* img)
{
	float dy,dx,absdy,absdx,delta;
	int32_t idx,idy,ix,iy;
	float m;
	dy = epy-spy;
	idy = float2int(dy);
	if(dy < 0)
	{
		absdy=-dy;
	}
	else
	{
		absdy=dy;
	}
	dx = epx-spx;
	idx = float2int(dx);
	if (dx < 0)
	{
		absdx=-dx;
	}
	else
	{
		absdx=dx;
	}
	if (absdx > absdy)
	{
		m=absdy/absdx;
		if (idx>=0 && idy >= 0)
		{
			delta = 0.5f;
			ix = float2int(spx)-1;
			iy = float2int(spy);
			for (int32_t c=0;c<idx;c++)
			{
				delta += m;
				if(delta > 1.0f)
				{
					iy++;
					delta = delta - 1.0f;
				}
				ix++;
				setPixel(ix,iy,img);
			}
		}
		else if (idx < 0 && idy>=0)
		{
			delta = 0.5f;
			ix = float2int(spx)+1;
			iy = float2int(spy);
			for (int32_t c=0;c<-idx;c++)
			{
				delta += m;
				if(delta > 1.0f)
				{
					iy++;
					delta = delta - 1.0f;
				}
				ix--;
				setPixel(ix,iy,img);
			}
		}
		else if (idx>=0 && idy < 0)
		{
			delta = 0.5f;
			ix = float2int(spx)-1;
			iy = float2int(spy);
			for (int32_t c=0;c<idx;c++)
			{
				delta += m;
				if(delta > 1.0f)
				{
					iy--;
					delta = delta - 1.0f;
				}
				ix++;
				setPixel(ix,iy,img);
			}
		}
		else
		{
			delta = 0.5f;
			ix = float2int(spx)+1;
			iy = float2int(spy);
			for (int32_t c=0;c<-idx;c++)
			{
				delta += m;
				if(delta > 1.0f)
				{
					iy--;
					delta = delta - 1.0f;
				}
				ix--;
				setPixel(ix,iy,img);
			}
		}
	}
	else
	{
		m=absdx/absdy;
		if (idx>=0 && idy >=0)
		{
			delta = 0.5f;
			ix = float2int(spx);
			iy = float2int(spy)-1;
			for (int32_t c=0;c<idy;c++)
			{
				delta += m;
				if(delta > 1.0f)
				{
					ix++;
					delta = delta - 1.0f;
				}
				iy++;
				setPixel(ix,iy,img);
			}
		}
		else if (idx < 0 && idy >=0)
		{
			delta = 0.5f;
			ix = float2int(spx);
			iy = float2int(spy)-1;
			for (int32_t c=0;c<idy;c++)
			{
				delta += m;
				if(delta > 1.0f)
				{
					ix--;
					delta = delta - 1.0f;
				}
				iy++;
				setPixel(ix,iy,img);
			}
		}
		else if (idx >= 0 && idy < 0)
		{
			delta = 0.5f;
			ix = float2int(spx);
			iy = float2int(spy)+1;
			for (int32_t c=0;c<-idy;c++)
			{
				delta += m;
				if(delta > 1.0f)
				{
					ix++;
					delta = delta - 1.0f;
				}
				iy--;
				setPixel(ix,iy,img);
			}
		}
		else
		{
			delta = 0.5f;
			ix = float2int(spx);
			iy = float2int(spy)+1;
			for (int32_t c=0;c<-idy;c++)
			{
				delta += m;
				if(delta > 1.0f)
				{
					ix--;
					delta = delta - 1.0f;
				}
				iy--;
				setPixel(ix,iy,img);
			}
		}
	}
}

void drawOval(float ax,float ay,float cx,float cy,BwImageType*img)
{
	float fix,fiy,dr;
	for(int32_t ix=0;ix<img->sx;ix++)
	{
		for(int32_t iy=0;iy<img->sy;iy++)
		{
			fix = int2float(ix);
			fiy = int2float(iy);
			dr = (fix-cx)*(fix-cx)/ax/ax + (fiy-cy)*(fiy-cy)/ay/ay; 
			if (dr <= 1.0f)
			{
				setPixel(ix,iy,img);
			}
		}
	}
}

void clearOval(float ax,float ay,float cx,float cy,BwImageType*img)
{
	float fix,fiy,dr;
	for(int32_t ix=0;ix<img->sx;ix++)
	{
		for(int32_t iy=0;iy<img->sy;iy++)
		{
			fix = int2float(ix);
			fiy = int2float(iy);
			dr = (fix-ix)*(fix-ix)/ax/ax + (fiy-iy)*(fiy-iy)/ay/ay; 
			if (dr <= 1.0f)
			{
				clearPixel(ix,iy,img);
			}
		}
	}
}

void clearSquare(float spx, float spy,float epx, float epy,BwImageType* img)
{
	uint32_t dx,dy;
	dx=(uint32_t)float2int(epx-spx);
	dy=(uint32_t)float2int(epy-spy);
	int32_t px,py;
	px=float2int(spx);
	py=float2int(spy);
	for (int32_t cx = 0;cx<dx;cx++)
	{
		for(int32_t cy=0;cy<dy;cy++)
		{
			clearPixel(px + cx,py + cy,img);
		}
	}
}

void drawSquare(float spx, float spy,float epx, float epy,BwImageType* img)
{
	uint32_t dx,dy;
	dx=(uint32_t)float2int(epx-spx);
	dy=(uint32_t)float2int(epy-spy);
	int32_t px,py;
	px=float2int(spx);
	py=float2int(spy);
	for (uint8_t cx = 0;cx<dx;cx++)
	{
		for(uint8_t cy=0;cy<dy;cy++)
		{
			setPixel(px + cx,py + cy,img);
		}
	}
}

uint8_t drawChar(uint8_t px, uint8_t py, char c,BwImageType* img,const void* font)
{
	if (font==(void*)0)
	{
		return drawCharOLedFont(px,py,c,img);
	}
	else
	{
		return drawCharGFXFont(px,py,c,img,(GFXfont*)font);
	}
}

uint8_t drawCharGFXFont(uint8_t px, uint8_t py, char c,BwImageType* img,const GFXfont* font)
{
	GFXglyph* glyph;
	int8_t w,h;
	int8_t xOffset,yOffset;
	uint32_t bitArrayOffset=0, bitOffset=0;
	uint8_t bitarray=0;
	c -= (uint8_t)font->first;
	glyph = font->glyph + c;
	w = glyph->width;
	h = glyph->height;
	xOffset = glyph->xOffset;
	yOffset = glyph->yOffset;
	for (int8_t y=0;y<h;y++)
	{
		for(int8_t x=0;x<w;x++)
		{
			if ((bitOffset++ & 0x7)==0)
			{
				bitarray = *(font->bitmap + glyph->bitmapOffset + bitArrayOffset++);
			}
			if (bitarray & 0x80)
			{
				if (px + x + xOffset > 0 && 
				    px + x + xOffset < img->sx &&
					py + y + yOffset < img->sy &&
					py + y + yOffset > 0
					)
				{
					setPixel(px+x+xOffset,py+y+yOffset,img);
				}
			}
			bitarray <<= 1;
		}
	}
	return glyph->xAdvance;
}

uint8_t drawCharOLedFont(uint8_t px, uint8_t py,char c, BwImageType* img)
{   
	uint8_t bitarray; 
	for(int8_t x=0;x<5;x++)
	{
		bitarray = oled_font_5x7[c - ' '][x];
		for (int8_t y=0;y<8;y++)
		{
			if (bitarray & 0x80)
			{
				if (px + x  > 0 && 
				    px + x < img->sx &&
					py - y < img->sy &&
					py - y > 0
					)
				{
					setPixel(px+x,py-y,img);
				}
			}
			bitarray <<= 1;
		}
	}
	return 6;
}

void drawText(uint8_t px, uint8_t py,const char * txt,BwImageType* img,const void* font)
{
	uint8_t ppx,ppy;
	ppx=px;
	ppy=py;
	uint16_t c=0;
	while(*(txt + c)!=0)
	{
		if (*(txt + c) >= ' ' && *(txt + c) < 127)
		{
			ppx += drawChar(ppx,ppy,*(txt+c),img,font);
		}
		else if (*(txt + c)=='\r')
		{
			ppx=px;
		} 
		else if (*(txt + c)=='\n')
		{
			if (font!=(void*)0)
			{
				ppy += ((GFXfont*)font)->yAdvance;
			}
			else
			{
				ppy += 8;
			}
		}
		else if (*(txt + c)=='\t')
		{
			ppx += drawChar(ppx,ppy,' ',img,font);
			ppx += drawChar(ppx,ppy,' ',img,font);
			ppx += drawChar(ppx,ppy,' ',img,font);
			ppx += drawChar(ppx,ppy,' ',img,font);
		}
		c++;
	}
}

void drawImage(uint8_t px, uint8_t py,const BwImageType * img, BwImageType* imgBuffer)
{
	uint8_t pixel;
	uint8_t cxOut, cyOut;
	for(uint8_t cy=0;cy<img->sy;cy++)
	{
		for(uint8_t cx=0;cx<img->sx;cx++)
		{
			cxOut = cx + px;
			cyOut =cy + py;
			pixel= getPixel(cx,cy,img);
			if (pixel)
			{
				setPixel(cxOut,cyOut,imgBuffer);
			}
			else
			{
				clearPixel(cxOut,cyOut,imgBuffer);
			}
		}
	}
}

uint8_t getPixel(int32_t px,int32_t py,const BwImageType*img)
{
	int32_t bitindex = py & 0x7;
	int32_t pageIdx =  (py >> 3)*(img->sx) + px;
	return *(img->data + pageIdx) & (1 << (bitindex)); 
}

void setPixel(int32_t px,int32_t py,BwImageType*img)
{
	int32_t bitindex = py & 0x7;
	int32_t pageIdx =  (py >> 3)*(img->sx) + px;
	*(img->data + pageIdx) |= (1 << (bitindex));
}

void clearPixel(int32_t px,int32_t py,BwImageType*img)
{
	int32_t bitindex = py & 0x7;
	int32_t pageIdx =  (py >> 3)*(img->sx) + px;
	*(img->data + pageIdx) &= ~(1 << (bitindex));
}


void clearImage(BwImageType*img)
{
	for(uint16_t c=0;c<256;c++)
    {
        *((uint32_t*)img->data + c) = 0;
    }
}
