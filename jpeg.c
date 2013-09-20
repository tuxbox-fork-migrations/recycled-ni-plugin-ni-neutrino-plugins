/*
 * $Id: jpeg.c,v 1.1 2009/12/19 19:42:49 rhabarber1848 Exp $
 *
 * tuxwetter - d-box2 linux project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>

#define FH_ERROR_OK 0
#define FH_ERROR_FILE 1		/* read/access error */
#define FH_ERROR_FORMAT 2	/* file format error */
#define FH_ERROR_MALLOC 3	/* error during malloc */

struct r_jpeg_error_mgr
{
	struct jpeg_error_mgr pub;
	jmp_buf envbuffer;
};


int fh_jpeg_id(const char *name)
{
//	dbout("fh_jpeg_id {\n");
	int fd;
	unsigned char id[10];
	fd=open(name,O_RDONLY); if(fd==-1) return(0);
	read(fd,id,10);
	close(fd);
//	 dbout("fh_jpeg_id }\n");
	if(id[6]=='J' && id[7]=='F' && id[8]=='I' && id[9]=='F')	return(1);
	if(id[0]==0xff && id[1]==0xd8 && id[2]==0xff) return(1);
	return(0);
}


void jpeg_cb_error_exit(j_common_ptr cinfo)
{
//	dbout("jpeg_cd_error_exit {\n");
	struct r_jpeg_error_mgr *mptr;
	mptr=(struct r_jpeg_error_mgr*) cinfo->err;
	(*cinfo->err->output_message) (cinfo);
	longjmp(mptr->envbuffer,1);
//	 dbout("jpeg_cd_error_exit }\n");
}

int fh_jpeg_load(const char *filename,unsigned char *buffer,int x,int y)
{
//	dbout("fh_jpeg_load (%d/%d) {\n",x,y);
	struct jpeg_decompress_struct cinfo;
	struct jpeg_decompress_struct *ciptr;
	struct r_jpeg_error_mgr emgr;
	unsigned char *bp;
	int px,c;
	FILE *fh;
	JSAMPLE *lb;

	ciptr=&cinfo;
	if(!(fh=fopen(filename,"rb"))) return(FH_ERROR_FILE);
	ciptr->err=jpeg_std_error(&emgr.pub);
	emgr.pub.error_exit=jpeg_cb_error_exit;
	if(setjmp(emgr.envbuffer)==1)
	{
		// FATAL ERROR - Free the object and return...
		jpeg_destroy_decompress(ciptr);
		fclose(fh);
//	dbout("fh_jpeg_load } - FATAL ERROR\n");
		return(FH_ERROR_FORMAT);
	}

	jpeg_create_decompress(ciptr);
	jpeg_stdio_src(ciptr,fh);
	jpeg_read_header(ciptr,TRUE);
	ciptr->out_color_space=JCS_RGB;
	if(x==(int)ciptr->image_width)
		ciptr->scale_denom=1;
	else if(abs(x*2 - ciptr->image_width) < 2)
		ciptr->scale_denom=2;
	else if(abs(x*4 - ciptr->image_width) < 4)
		ciptr->scale_denom=4;
	else if(abs(x*8 - ciptr->image_width) < 8)
		ciptr->scale_denom=8;
	else
		ciptr->scale_denom=1;

	jpeg_start_decompress(ciptr);

	px=ciptr->output_width;
	c=ciptr->output_components;


	if(c==3)
	{
		lb=(JSAMPLE*)(*ciptr->mem->alloc_small)((j_common_ptr) ciptr,JPOOL_PERMANENT,c*px);
		bp=buffer;
		while(ciptr->output_scanline < ciptr->output_height)
		{
			jpeg_read_scanlines(ciptr, &lb, 1);
			memcpy(bp,lb,px*c);
			bp+=px*c;
		}                 

	}
	jpeg_finish_decompress(ciptr);
	jpeg_destroy_decompress(ciptr);
	fclose(fh);
//	 dbout("fh_jpeg_load }\n");
	return(FH_ERROR_OK);
}

int fh_jpeg_getsize(const char *filename,int *x,int *y, int wanted_width, int wanted_height)
{
//	dbout("fh_jpeg_getsize {\n");
	struct jpeg_decompress_struct cinfo;
	struct jpeg_decompress_struct *ciptr;
	struct r_jpeg_error_mgr emgr;

	int px,py;
	FILE *fh;
	
	ciptr=&cinfo;
	if(!(fh=fopen(filename,"rb"))) return(FH_ERROR_FILE);

	ciptr->err=jpeg_std_error(&emgr.pub);
	emgr.pub.error_exit=jpeg_cb_error_exit;
	if(setjmp(emgr.envbuffer)==1)
	{
		// FATAL ERROR - Free the object and return...
		jpeg_destroy_decompress(ciptr);
		fclose(fh);
//	dbout("fh_jpeg_getsize } - FATAL ERROR\n");
		return(FH_ERROR_FORMAT);
	}

	jpeg_create_decompress(ciptr);
	jpeg_stdio_src(ciptr,fh);
	jpeg_read_header(ciptr,TRUE);
	ciptr->out_color_space=JCS_RGB;
	// should be more flexible...
	if((int)ciptr->image_width/8 >= wanted_width ||
      (int)ciptr->image_height/8 >= wanted_height)
		ciptr->scale_denom=8;
	else if((int)ciptr->image_width/4 >= wanted_width ||
      (int)ciptr->image_height/4 >= wanted_height)
		ciptr->scale_denom=4;
	else if((int)ciptr->image_width/2 >= wanted_width ||
           (int)ciptr->image_height/2 >= wanted_height)
		ciptr->scale_denom=2;
	else
		ciptr->scale_denom=1;

	jpeg_start_decompress(ciptr);
	px=ciptr->output_width; py=ciptr->output_height;
	*x=px; *y=py;
//	jpeg_finish_decompress(ciptr);
	jpeg_destroy_decompress(ciptr);
	fclose(fh);
//	 dbout("fh_jpeg_getsize }\n");
	return(FH_ERROR_OK);
}

