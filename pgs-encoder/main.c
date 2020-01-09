/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
 * Copyright (C) Koichi Akabe 2009 <mail@vbkaisetsu.com>
 * 
 * main.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * main.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <png.h>
#include <stdlib.h>
#include <unistd.h>

int matchchar(char f[], char q[], int p)
{
	int c;
	c = 0;
	while(1)
	{
		if(q[c] == 0)
			return(1);
		if(f[p + c] != q[c])
			return(0);
		c++;
	}
}

void getabsolutepath(char rp[], char ap[])
{
	int i;
	i = 0;
	int pl;
	pl = 0;
	char pathsymbol[4];
	getcwd(ap, 512);
	while(1)
	{
		if(ap[pl] == 0x00)
		{
			break;
		}
		pl++;
	}
	if(ap[pl - 1] != 0x2f)
	{
		ap[pl] = 0x2f;
		pl++;
	}
	while(1)
	{
		sprintf(pathsymbol, "../");
		if(matchchar(rp, pathsymbol, i))
		{
			i += 3;
			pl--;
			while(1)
			{
				if(ap[pl - 1] == 0x2f)
				{
					break;
				}
				pl--;
			}
			continue;
		}
		sprintf(pathsymbol, "./");
		if(matchchar(rp, pathsymbol, i))
		{
			i += 2;
			continue;
		}
		sprintf(pathsymbol, "/");
		if(matchchar(rp, pathsymbol, i))
		{
			pl = 0;
		}
		break;
	}
	while(rp[i] != 0x00)
	{
		ap[pl] = rp[i];
		i++;
		pl++;
	}
	ap[pl] = 0x00;
}

void longtobyte(long l, char *c1, char *c2, char *c3, char *c4)
{
	if(l >= 0)
	{
		*c1 = floor((double)l / 16777216);
		*c2 = floor((double)(l - (*c1) * 16777216) / 65536);
		*c3 = floor((double)(l - (*c1) * 16777216 - (*c2) * 65536) / 256);
		*c4 = l - (*c1) * 16777216 - (*c2) * 65536 - (*c3) * 256;
	}
	else
	{
		signed long c5, c6, c7, c8;
		c5 = floor((double)l / 16777216);
		c6 = floor((double)(l - c5 * 16777216) / 65536);
		c7 = floor((double)(l - c5 * 16777216 - c6 * 65536) / 256);
		c8 = (l - c5 * 16777216 - c6 * 65536 - c7 * 256);
		*c1 = c5;
		*c2 = c6;
		*c3 = c7;
		*c4 = c8;
	}
}

void inttobyte(int l, char *c1, char *c2)
{
	*c1 = floor((double)l / 256);
	*c2 = l - (*c1) * 256;
}

void help()
{
	printf("Syntax: pgssup [options] <xmlfile> <outputfile>\n");
	printf("\n");
	printf("Options\n");
	printf(" -s <WxH>        Size of Video frame (default: 1920x1080)\n");
	printf("\n");
	printf("XML structure:\n");
	printf("\n");
	printf("FOR EXAMPLE\n");
	printf("<pgssup defaultoffset=\"0,920\">\n");
	printf("    <subtitle starttime=\"00:00:54.384\" endtime=\"00:00:56.932\" image=\"/home/hoge/sub00000.png\" />\n");
	printf("    <subtitle starttime=\"00:00:59.837\" endtime=\"00:01:01.411\" offset=\"1000,50\" image=\"/home/hoge/sub00001.png\" />\n");
	printf("    <subtitle starttime=\"00:01:10.734\" endtime=\"00:01:12.638\" view=\"forced\" image=\"/home/hoge/sub00002.png\" />\n");
	printf("</pgssup>\n");
	printf("\n");
	printf("defaultoffset: Default offset of subtitles. If the offset of subtitles is null, this value will set to the subtitle. (default=0,0)\n");
	printf("offset:        the position of subtitle on the display.\n");
	printf("view:          Subtitles will be forced to display if this property is \"forced\".\n");
	printf("image:         This image should have less than 256 colors.\n");
	printf("\n");
}

int main(int argc, char *argv[])
{
	printf("==============================================\n");
	printf("| PGSSUP version 0.1                         |\n");
	printf("| Koichi Akabe 2009 <mail@vbkaisetsu.com>    |\n");
	printf("| Last modified: 2009-01-13 (YYYY-MM-DD)     |\n");
	printf("| License: GNU Lesser General Public License |\n");
	printf("==============================================\n");
	printf("\n");
	int i, j, k;
	int width, height;
	width = 1920;
	height = 1080;
	char xmlpath[512];
	char outpath[512];
	
	for(i = 1; i < argc - 2; i++)
	{
		if(argv[i][0] == 0x2d)
		{
			if(strcmp(argv[i], "-s") == 0)
			{
				i++;
				sscanf(argv[i], "%dx%d", &width, &height);
				if(width == 0 || height == 0)
				{
					printf("Error: video size failed\n");
					return(0);
				}
			}
			else if(strcmp(argv[i], "-h") == 0)
			{
				help();
				return(0);
			}
			else
			{
				printf("Error: unknown option: %s\n", argv[i]);
				return(0);
			}
		}
	}
	if(argc == 1)
	{
		help();
		return(0);
	}
	if(argc == 2)
	{
		if(strcmp(argv[1], "-h") == 0)
			help();
		else
			printf("Error: syntax error\n");
		return(0);
	}
	sprintf(xmlpath, "%s", argv[argc - 2]);
	sprintf(outpath, "%s", argv[argc - 1]);
	char path[512];
	long fsize;
	FILE *fp;
	getabsolutepath(xmlpath, path);
	fp = fopen(path, "r");
	if (fp == NULL)
	{
		printf("Error: xml file \"%s\" opening faild\n", path);
		return(0);
	}
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	char xml[fsize + 32];
	fseek(fp, 0, SEEK_SET);
	for(i = 0; i < fsize; i++)
	{
		xml[i] = (char)fgetc(fp);
	}
	fclose(fp);
	
	char defaultoffsetc[16];
	char starttimec[16384][16];
	char endtimec[16384][16];
	char offsetc[16384][16];
	char viewc[16384][16];
	char imagec[16384][256];
	int tagquo, tagtype;
	tagquo = 0;
	tagtype = 0;
	
	int h1, m1, s1, ms1, h2, m2, s2, ms2;
	long starttime;
	long endtime;
	char pngfile[128];
	long time1, time2, time3, time4;
	char b1, b2, b3, b4;
	char supdata[67000];
	int c;
	c = 0;
	char *writer;
	int targetpoint;
	char dummy[65536];
	writer = dummy;
	targetpoint = 0;
	defaultoffsetc[0] = 0;
	for(i = 0; i < 16384; i++)
	{
		starttimec[i][0] = 0;
		endtimec[i][0] = 0;
		offsetc[i][0] = 0;
		viewc[i][0] = 0;
		imagec[i][0] = 0;
	}
	char q[32];
	int knowntag;
	int jumpquo;
	jumpquo = 0;
	knowntag = 0;
	printf("Info: parsing xml ...\n");
	for(i = 0; i < fsize; i++)
	{
		if(xml[i] == 0x3c && tagquo == 0)
		{
			knowntag = 0;
			tagquo = 1;
			sprintf(q, "<pgssup");
			if(matchchar(xml, q, i) && (xml[i + 7] == 0x20 || xml[i + 7] == 0x3e))
			{
				if(tagtype == 0)
				{
					tagtype = 1;
					knowntag = 1;
				}
				else
				{
					printf("Error: the pgssup tag should be in the root\n");
					return(0);
				}
			}
			sprintf(q, "<subtitle");
			if(matchchar(xml, q, i) && (xml[i + 9] == 0x20 || xml[i + 9] == 0x3e))
			{
				if(tagtype == 1)
				{
					tagtype = 2;
					knowntag = 1;
				}
				else
				{
					printf("Error: the subtitle tag should be in the pgssup tag\n");
					return(0);
				}
				c++;
			}
			sprintf(q, "</subtitle>");
			if(matchchar(xml, q, i))
			{
				if(tagtype == 2)
				{
					tagtype = 1;
					knowntag = 1;
				}
				else
				{
					printf("Error: XML syntax is wrong\n");
					return(0);
				}
			}
			sprintf(q, "</pgssup>");
			if(matchchar(xml, q, i))
			{
				if(tagtype == 1)
				{
					tagtype = 0;
					knowntag = 1;
				}
				else
				{
					printf("Error: XML syntax is wrong\n");
					return(0);
				}
			}
			if(!knowntag)
			{
                // NOTE (magiruuvelvet): quick patch to not die on XML comments
                // printf("Error: unknown tag\n");
                // return(0);
			}
		}
		if(xml[i - 1] == 0x22 && tagquo == 1 && jumpquo == 0)
		{
			tagquo = 2;
			sprintf(q, " defaultoffset=");
			if(tagtype == 1 && matchchar(xml, q, i - 16))
			{
				targetpoint = 0;
				writer = defaultoffsetc;
			}
			sprintf(q, " starttime=");
			if(tagtype == 2 && matchchar(xml, q, i - 12))
			{
				targetpoint = 0;
				writer = starttimec[c - 1];
			}
			sprintf(q, " endtime=");
			if(tagtype == 2 && matchchar(xml, q, i - 10))
			{
				targetpoint = 0;
				writer = endtimec[c - 1];
			}
			sprintf(q, " offset=");
			if(tagtype == 2 && matchchar(xml, q, i - 9))
			{
				targetpoint = 0;
				writer = offsetc[c - 1];
			}
			sprintf(q, " view=");
			if(tagtype == 2 && matchchar(xml, q, i - 7))
			{
				targetpoint = 0;
				writer = viewc[c - 1];
			}
			sprintf(q, " image=");
			if(tagtype == 2 && matchchar(xml, q, i - 8))
			{
				targetpoint = 0;
				writer = imagec[c - 1];
			}
		}
		if(jumpquo == 1)
			jumpquo = 0;
		if(xml[i] == 0x22 && tagquo == 2)
		{
			tagquo = 1;
			writer[targetpoint] = 0;
			writer = dummy;
			jumpquo = 1;
		}
		if(xml[i] == 0x3e && tagquo == 1)
		{
			tagquo = 0;
			if(xml[i - 1] == 0x2f)
			{
				tagtype--;
			}
		}
		writer[targetpoint] = xml[i];
		targetpoint++;
	}
	if(tagtype != 0)
	{
		printf("Error: XML is not valid\n");
		return(0);
	}
	if(tagquo != 0)
	{
		printf("Error: XML is not valid\n");
		return(0);
	}
	printf("Info: loading xml completed\n");
	printf("\n");
	png_structp png_ptr;
	png_infop info_ptr;
	png_infop end_info;
	unsigned char pngheader[16];
	int is_png;
	unsigned char palette_r[256];
	unsigned char palette_g[256];
	unsigned char palette_b[256];
	unsigned char palette_a[256];
	int palette_c;
	png_uint_32 png_h, png_w;
	int colortype, bit_depth;
	int x, y;
	int offsetx, offsety, onoff;
	FILE *pngf;
	int supt;
	int writtenbyte;
	signed char Cr, Cb;
	unsigned char Y;
	int prevpalette;
	int colorseqcount;
	int bmplengthtarget;
	int bmplength;
	int doffsetx, doffsety;
	sscanf(defaultoffsetc, "%d,%d", &doffsetx, &doffsety);
	getabsolutepath(outpath, path);
	fp = fopen(path, "wb");
	int aflag;
	aflag = 0;
	if (fp == NULL)
	{
		printf("Error: the sup file \"%s\" could not be opened\n", path);
		return(0);
	}
	for(i = 0; i < c; i++)
	{
		supt = 0;
		palette_c = 0;
		sscanf(starttimec[i], "%2d:%2d:%2d.%3d", &h1, &m1, &s1, &ms1);
		sscanf(endtimec[i], "%2d:%2d:%2d.%3d", &h2, &m2, &s2, &ms2);
		sscanf(imagec[i], "%s", pngfile);
		sscanf(offsetc[i], "%d,%d", &offsetx, &offsety);
		if(offsetc[i][0] == 0x00)
		{
			offsetx = doffsetx;
			offsety = doffsety;
		}
		sprintf(q, "forced");
		if(matchchar(viewc[i], q, 0))
		{
			onoff = 1;
			printf("Info: including subtitle %d... (%d:%02d:%02d.%03d - %d:%02d:%02d.%03d forced)\n", i + 1, h1, m1, s1, ms1, h2, m2, s2, ms2);
		}
		else
		{
			onoff = 0;
			printf("Info: including subtitle %d... (%d:%02d:%02d.%03d - %d:%02d:%02d.%03d)\n", i + 1, h1, m1, s1, ms1, h2, m2, s2, ms2);
		}
		
		starttime = h1 * 3600000 + m1 * 60000 + s1 * 1000 + ms1;
		endtime = h2 * 3600000 + m2 * 60000 + s2 * 1000 + ms2;
        starttime *= 90; //.090;
		time1 = starttime - 5832;
		time2 = starttime - 90;
		time3 = starttime - 5643;
        endtime *= 90; //.090;
		time4 = endtime - 90;
		
		// 0x16
		supdata[supt] = 0x50;
		supdata[supt + 1] = 0x47;
		longtobyte(starttime, &b1, &b2, &b3, &b4);
		supdata[supt + 2] = b1;
		supdata[supt + 3] = b2;
		supdata[supt + 4] = b3;
		supdata[supt + 5] = b4;
		longtobyte(time1, &b1, &b2, &b3, &b4);
		supdata[supt + 6] = b1;
		supdata[supt + 7] = b2;
		supdata[supt + 8] = b3;
		supdata[supt + 9] = b4;
		supdata[supt + 10] = 0x16;
		supdata[supt + 11] = 0x00;
		supdata[supt + 12] = 0x13;
		inttobyte(width, &b1, &b2);
		supdata[supt + 13] = b1;
		supdata[supt + 14] = b2;
		inttobyte(height, &b1, &b2);
		supdata[supt + 15] = b1;
		supdata[supt + 16] = b2;
		supdata[supt + 17] = 0x40;
		supdata[supt + 18] = 0x00;
		supdata[supt + 19] = 0x00;
		supdata[supt + 20] = 0x80;
		supdata[supt + 21] = 0x00;
		supdata[supt + 22] = 0x00;
		supdata[supt + 23] = 0x01;
		supdata[supt + 24] = 0x00;
		supdata[supt + 25] = 0x00;
		supdata[supt + 26] = 0x00;
		if(onoff != 0)
			supdata[supt + 27] = 0x40;
		else
			supdata[supt + 27] = 0x00;
		inttobyte(offsetx, &b1, &b2);
		supdata[supt + 28] = b1;
		supdata[supt + 29] = b2;
		inttobyte(offsety, &b1, &b2);
		supdata[supt + 30] = b1;
		supdata[supt + 31] = b2;
		supt += 32;

		// PNG
		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png_ptr)
		{
			printf("Error: file \"%s\" could not be opened\n", pngfile);
			return(0);
		}
		info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr)
		{
			png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
			printf("Error: file \"%s\" could not be opened\n", pngfile);
			return(0);
		}
		end_info = png_create_info_struct(png_ptr);
		if (!end_info)
		{
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			printf("Error: file \"%s\" could not be opened\n", pngfile);
			return(0);
		}
		getabsolutepath(pngfile, path);
		pngf = fopen(path, "rb");
		if(pngf == NULL)
		{
			printf("Error: file \"%s\" could not be opened\n", path);
			return(0);
		}
		if(fread(pngheader, 1, 8, pngf) < 8)
		{
			printf("Error: file \"%s\" is not PNG format\n", path);
			return(0);
		}
		is_png = !png_sig_cmp(pngheader, 0, 8);
		if(!is_png)
		{
			printf("Error: file \"%s\" is not PNG format\n", path);
			return(0);
		}
		png_init_io(png_ptr, pngf);
		png_set_sig_bytes(png_ptr, 8); 
		png_read_info(png_ptr, info_ptr);
		png_get_IHDR(png_ptr, info_ptr, &png_w, &png_h, &bit_depth, &colortype, NULL, NULL, NULL);
		if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		    png_set_expand(png_ptr);
		if(colortype == PNG_COLOR_TYPE_PALETTE)
		    png_set_expand(png_ptr);
		if(colortype == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		    png_set_expand(png_ptr);
		if(bit_depth > 8)
		    png_set_strip_16(png_ptr);
		if(colortype == PNG_COLOR_TYPE_GRAY)
		    png_set_gray_to_rgb(png_ptr);
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
			png_set_tRNS_to_alpha(png_ptr);
		if(colortype == PNG_COLOR_TYPE_RGB || colortype == PNG_COLOR_TYPE_GRAY)
		{
			png_set_add_alpha(png_ptr, 255, PNG_FILLER_AFTER);
		}
		unsigned char *bmbuff;
		unsigned char **pixel;
		bmbuff = (unsigned char *)malloc(png_w * png_h * 4);
		pixel = (unsigned char **)malloc(png_w * png_h * 4);
		for(j = 0; j < png_h; j++)
			pixel[j] = bmbuff + png_w * 4 * j;
		png_read_update_info(png_ptr, info_ptr);
		png_read_image(png_ptr, pixel);
		
		png_read_end(png_ptr, end_info);
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		
		// 0x17
		supdata[supt] = 0x50;
		supdata[supt + 1] = 0x47;
		longtobyte(time2, &b1, &b2, &b3, &b4);
		supdata[supt + 2] = b1;
		supdata[supt + 3] = b2;
		supdata[supt + 4] = b3;
		supdata[supt + 5] = b4;
		longtobyte(time1, &b1, &b2, &b3, &b4);
		supdata[supt + 6] = b1;
		supdata[supt + 7] = b2;
		supdata[supt + 8] = b3;
		supdata[supt + 9] = b4;
		supdata[supt + 10] = 0x17;
		supdata[supt + 11] = 0x00;
		supdata[supt + 12] = 0x0A;
		supdata[supt + 13] = 0x01;
		supdata[supt + 14] = 0x00;
		inttobyte(offsetx, &b1, &b2);
		supdata[supt + 15] = b1;
		supdata[supt + 16] = b2;
		inttobyte(offsety, &b1, &b2);
		supdata[supt + 17] = b1;
		supdata[supt + 18] = b2;
		inttobyte(png_w, &b1, &b2);
		supdata[supt + 19] = b1;
		supdata[supt + 20] = b2;
		inttobyte(png_h, &b1, &b2);
		supdata[supt + 21] = b1;
		supdata[supt + 22] = b2;
		supt += 23;
		palette_c = 0;
		aflag = 0;
		palette_c = 0;
		// creating color palette
		for(j = 0; j < 256; j++)
		{
			palette_r[j] = 0;
			palette_g[j] = 0;
			palette_b[j] = 0;
			palette_a[j] = 0;
		}
		for(y = 0; y < png_h; y++)
		{
			for(x = 0; x < png_w; x++)
			{
				if(pixel[y][x * 4 + 3] == 0)
				{
					aflag = 1;
					continue;
				}
				for(j = 0; j < palette_c; j++)
				{
					if(
					   palette_r[j] == pixel[y][x * 4] &&
					   palette_g[j] == pixel[y][x * 4 + 1] &&
					   palette_b[j] == pixel[y][x * 4 + 2] &&
					   palette_a[j] == pixel[y][x * 4 + 3]
					   )
						break;
				}
				if(j == palette_c)
				{
					if(palette_c == 256)
					{
						printf("Error: the png file \"%s\" has more than 256 colors\n", pngfile);
						return(0);
					}
					palette_r[palette_c] = pixel[y][x * 4];
					palette_g[palette_c] = pixel[y][x * 4 + 1];
					palette_b[palette_c] = pixel[y][x * 4 + 2];
					palette_a[palette_c] = pixel[y][x * 4 + 3];
					palette_c++;
				}
			}
		}
		if(aflag == 1 && palette_c == 256)
		{
			printf("Error: the png file \"%s\" has more than 256 colors\n", pngfile);
			return(0);
		}
		printf("Info: the png file \"%s\" has %d color(s); size: %dx%d\n", pngfile, palette_c, (int)png_w, (int)png_h);
		// 0x14
		supdata[supt] = 0x50;
		supdata[supt + 1] = 0x47;
		longtobyte(time1, &b1, &b2, &b3, &b4);
		supdata[supt + 2] = b1;
		supdata[supt + 3] = b2;
		supdata[supt + 4] = b3;
		supdata[supt + 5] = b4;
		supdata[supt + 6] = 0x00;
		supdata[supt + 7] = 0x00;
		supdata[supt + 8] = 0x00;
		supdata[supt + 9] = 0x00;
		supdata[supt + 10] = 0x14;
		inttobyte(palette_c * 5 + 2, &b1, &b2);
		supdata[supt + 11] = b1;
		supdata[supt + 12] = b2;
		supdata[supt + 13] = 0x00;
		supdata[supt + 14] = 0x00;
		supt += 15;
		for(j = 0; j < palette_c; j++)
		{
			// RGB -> YCrCb
			Y = 0.299 * (double)palette_r[j] + 0.587 * (double)palette_g[j] + 0.114 * (double)palette_b[j];
			Cr = 0.5 * (double)palette_r[j] - 0.419 * (double)palette_g[j] - 0.081 * (double)palette_b[j];
			Cb = -0.169 * (double)palette_r[j] - 0.332 * (double)palette_g[j] + 0.5 * (double)palette_b[j];
			supdata[supt] = j;
			supdata[supt + 1] = Y;
			supdata[supt + 2] = Cr + 128;
			supdata[supt + 3] = Cb + 128;
			supdata[supt + 4] = palette_a[j];
			supt += 5;
		}
		
		// 0x15
		supdata[supt] = 0x50;
		supdata[supt + 1] = 0x47;
		longtobyte(time3, &b1, &b2, &b3, &b4);
		supdata[supt + 2] = b1;
		supdata[supt + 3] = b2;
		supdata[supt + 4] = b3;
		supdata[supt + 5] = b4;
		longtobyte(time1, &b1, &b2, &b3, &b4);
		supdata[supt + 6] = b1;
		supdata[supt + 7] = b2;
		supdata[supt + 8] = b3;
		supdata[supt + 9] = b4;
		supdata[supt + 10] = 0x15;
		bmplengthtarget = supt + 11;
		supdata[supt + 13] = 0x00;
		supdata[supt + 14] = 0x00;
		supdata[supt + 15] = 0x00;
		supdata[supt + 16] = 0xc0;
		supdata[supt + 17] = 0x00;
		inttobyte(png_w, &b1, &b2);
		supdata[supt + 20] = b1;
		supdata[supt + 21] = b2;
		inttobyte(png_h, &b1, &b2);
		supdata[supt + 22] = b1;
		supdata[supt + 23] = b2;
		supt += 24;
		bmplength = supt;
		for(y = 0; y < png_h; y++)
		{
			colorseqcount = 1;
			for(j = 0; j < palette_c; j++)
			{
				if(
				   palette_r[j] == pixel[y][0] &&
				   palette_g[j] == pixel[y][1] &&
				   palette_b[j] == pixel[y][2] &&
				   palette_a[j] == pixel[y][3]
				   )
					break;
			}
			prevpalette = j;
			if(pixel[y][3] == 0)
				prevpalette = 0xff;
			for(x = 1; x < png_w; x++)
			{
				for(j = 0; j < palette_c; j++)
				{
					if(
					   palette_r[j] == pixel[y][x * 4 + 0] &&
					   palette_g[j] == pixel[y][x * 4 + 1] &&
					   palette_b[j] == pixel[y][x * 4 + 2] &&
					   palette_a[j] == pixel[y][x * 4 + 3]
					   )
						break;
				}
				if(pixel[y][x * 4 + 3] == 0)
					j = 0xff;
				if(j == prevpalette)
				{
					colorseqcount++;
					continue;
				}
				else
				{
					k = (double)colorseqcount / 256;
					if(colorseqcount >= 0x40)
					{
						if(prevpalette != 0)
						{
							supdata[supt] = 0x00;
							supdata[supt + 1] = k + 0xC0;
							supdata[supt + 2] = colorseqcount - k * 256;
							supdata[supt + 3] = prevpalette;
							supt += 4;
						}
						else
						{
							supdata[supt] = 0x00;
							supdata[supt + 1] = k + 0x40;
							supdata[supt + 2] = colorseqcount - k * 256;
							supt += 3;
						}
					}
					else
					{
						if(prevpalette != 0)
						{
							if(prevpalette <= 0x39 && colorseqcount == 1)
							{
								supdata[supt] = prevpalette;
								supt += 1;
							}
							else if(prevpalette <= 0x39 && colorseqcount == 2)
							{
								supdata[supt] = prevpalette;
								supdata[supt + 1] = prevpalette;
								supt += 2;
							}
							else
							{
								supdata[supt] = 0x00;
								supdata[supt + 1] = colorseqcount + 0x80;
								supdata[supt + 2] = prevpalette;
								supt += 3;
							}
						}
						else
						{
							supdata[supt] = 0x00;
							supdata[supt + 1] = colorseqcount;
							supt += 2;
						}
					}
					prevpalette = j;
					colorseqcount = 1;
				}
			}
			j = (double)colorseqcount / 256;
			if(colorseqcount >= 0x40)
			{
				if(prevpalette != 0)
				{
					supdata[supt] = 0x00;
					supdata[supt + 1] = j + 0xC0;
					supdata[supt + 2] = colorseqcount - j * 256;
					supdata[supt + 3] = prevpalette;
					supt += 4;
				}
				else
				{
					supdata[supt] = 0x00;
					supdata[supt + 1] = j + 0x40;
					supdata[supt + 2] = colorseqcount - j * 256;
					supt += 3;
				}
			}
			else
			{
				if(prevpalette != 0)
				{
					if(prevpalette <= 0x39 && colorseqcount == 1)
					{
						supdata[supt] = prevpalette;
						supt += 1;
					}
					else if(prevpalette <= 0x39 && colorseqcount == 2)
					{
						supdata[supt] = prevpalette;
						supdata[supt + 1] = prevpalette;
						supt += 2;
					}
					else
					{
						supdata[supt] = 0x00;
						supdata[supt + 1] = colorseqcount + 0x80;
						supdata[supt + 2] = prevpalette;
						supt += 3;
					}
				}
				else
				{
					supdata[supt] = 0x00;
					supdata[supt + 1] = colorseqcount;
					supt += 2;
				}
			}
			supdata[supt] = 0x00;
			supdata[supt + 1] = 0x00;
			supt += 2;
		}
		bmplength = supt - bmplength + 11;
		if(bmplength > 65535)
		{
			printf("Error: subtitle picture is very complicated. (%d byte)\n", bmplength);
			printf("       It should be less than 65537 bytes.\n");
			return(0);
		}
		inttobyte(bmplength, &b1, &b2);
		supdata[bmplengthtarget] = b1;
		supdata[bmplengthtarget + 1] = b2;
		inttobyte(bmplength - 7, &b1, &b2);
		supdata[bmplengthtarget + 7] = b1;
		supdata[bmplengthtarget + 8] = b2;
		// 0x80
		supdata[supt] = 0x50;
		supdata[supt + 1] = 0x47;
		longtobyte(time3, &b1, &b2, &b3, &b4);
		supdata[supt + 2] = b1;
		supdata[supt + 3] = b2;
		supdata[supt + 4] = b3;
		supdata[supt + 5] = b4;
		supdata[supt + 6] = 0x00;
		supdata[supt + 7] = 0x00;
		supdata[supt + 8] = 0x00;
		supdata[supt + 9] = 0x00;
		supdata[supt + 10] = 0x80;
		supdata[supt + 11] = 0x00;
		supdata[supt + 12] = 0x00;
		supt += 13;
		
		// 0x16
		supdata[supt] = 0x50;
		supdata[supt + 1] = 0x47;
		longtobyte(endtime, &b1, &b2, &b3, &b4);
		supdata[supt + 2] = b1;
		supdata[supt + 3] = b2;
		supdata[supt + 4] = b3;
		supdata[supt + 5] = b4;
		longtobyte(time4, &b1, &b2, &b3, &b4);
		supdata[supt + 6] = b1;
		supdata[supt + 7] = b2;
		supdata[supt + 8] = b3;
		supdata[supt + 9] = b4;
		supdata[supt + 10] = 0x16;
		supdata[supt + 11] = 0x00;
		supdata[supt + 12] = 0x0b;
		inttobyte(width, &b1, &b2);
		supdata[supt + 13] = b1;
		supdata[supt + 14] = b2;
		inttobyte(height, &b1, &b2);
		supdata[supt + 15] = b1;
		supdata[supt + 16] = b2;
		supdata[supt + 17] = 0x40;
		supdata[supt + 18] = 0x00;
		supdata[supt + 19] = 0x01;
		supdata[supt + 20] = 0x00;
		supdata[supt + 21] = 0x00;
		supdata[supt + 22] = 0x00;
		supdata[supt + 23] = 0x00;
		supt += 24;
		
		// 0x17
		supdata[supt] = 0x50;
		supdata[supt + 1] = 0x47;
		longtobyte(time4, &b1, &b2, &b3, &b4);
		supdata[supt + 2] = b1;
		supdata[supt + 3] = b2;
		supdata[supt + 4] = b3;
		supdata[supt + 5] = b4;
		supdata[supt + 6] = 0x00;
		supdata[supt + 7] = 0x00;
		supdata[supt + 8] = 0x00;
		supdata[supt + 9] = 0x00;
		supdata[supt + 10] = 0x17;
		supdata[supt + 11] = 0x00;
		supdata[supt + 12] = 0x0a;
		supdata[supt + 13] = 0x01;
		supdata[supt + 14] = 0x00;
		inttobyte(offsetx, &b1, &b2);
		supdata[supt + 15] = b1;
		supdata[supt + 16] = b2;
		inttobyte(offsety, &b1, &b2);
		supdata[supt + 17] = b1;
		supdata[supt + 18] = b2;
		inttobyte(png_w, &b1, &b2);
		supdata[supt + 19] = b1;
		supdata[supt + 20] = b2;
		inttobyte(png_h, &b1, &b2);
		supdata[supt + 21] = b1;
		supdata[supt + 22] = b2;
		supt += 23;
		
		// 0x80
		supdata[supt] = 0x50;
		supdata[supt + 1] = 0x47;
		longtobyte(time4, &b1, &b2, &b3, &b4);
		supdata[supt + 2] = b1;
		supdata[supt + 3] = b2;
		supdata[supt + 4] = b3;
		supdata[supt + 5] = b4;
		supdata[supt + 6] = 0x00;
		supdata[supt + 7] = 0x00;
		supdata[supt + 8] = 0x00;
		supdata[supt + 9] = 0x00;
		supdata[supt + 10] = 0x80;
		supdata[supt + 11] = 0x00;
		supdata[supt + 12] = 0x00;
		supt += 13;
		free(pixel);
		free(bmbuff);
		writtenbyte = fwrite(supdata, sizeof(char), supt, fp);
		printf("Info: subtitle %d was included (%d bytes, bitmap: %d bytes)...\n", i + 1, writtenbyte, bmplength - 7);
		printf("\n");
	}
	fclose(fp);
	printf("Complete !\n");
	return(0);
}


