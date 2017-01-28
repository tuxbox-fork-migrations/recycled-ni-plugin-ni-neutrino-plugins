#include <string.h>
#include <stdio.h>
#include <time.h>

#include "input.h"
#include "icons.h"
#include "text.h"
#include "io.h"
#include "gfx.h"
#include "pngw.h"


#define xbrd 	25
#define ybrd	25
#define exsz	23
#define eysz	38
#define bxsz	60
#define bysz	60
#define hsz		50
#define tys		30
#define NUM		'#'
#define ANUM	'@'
#define HEX		'^'
#define ndelay	2
#define REPEAT_TIMER 2

char rstr[512]={0},tstr[512]={0}, *format=NULL, *estr=NULL;
unsigned int kpos=0;
int epos=-1,cpos=0,cnt,first=1,hex=0;
const char kcod[10][13]={"0 _.:,;$@()#","1-+*/", "2abc�", "3def", "4ghi", "5jkl", "6mno�", "7pqrs�", "8tuv�", "9wxyz"};
const char hcod[10][13]={"0","1", "2abc", "3def", "4", "5", "6", "7", "8", "9"};
unsigned rc;
extern int radius;
extern int rcvalue;
const char INST_FILE[]="/tmp/rc.locked";
int instance=0;
int rclocked=0;
int rcvalue=0;

int get_instance(void)
{
FILE *fh;
int rval=0;

	if((fh=fopen(INST_FILE,"r"))!=NULL)
	{
		rval=fgetc(fh);
		fclose(fh);
	}
	return rval;
}

void put_instance(int pval)
{
FILE *fh;

	if(pval)
	{
		if (!rclocked) {
			rclocked=1;
			system("pzapit -lockrc > /dev/null");
		}
		if((fh=fopen(INST_FILE,"w"))!=NULL)
		{
			fputc(pval,fh);
			fclose(fh);
		}
	}
	else
	{
		remove(INST_FILE);
		system("pzapit -unlockrc > /dev/null");
	}
}

int IsAlpha(char ch)
{
	const char uml[]="������";
	return (((ch>='A')&&(ch<='Z')) || ((ch>='a')&&(ch<='z')) || strchr(uml,ch));
}

int IsNum(char ch)
{
	return ((ch>='0')&&(ch<='9'));
}

int IsInput(char ch)
{

	if((ch==NUM) || (ch==ANUM))
	{
		hex=0;
		return 1;
	}
	if (ch==HEX)
	{
		hex=1;
		return 1;
	}
	return 0;
}

void FindCode(char ch)
{

	if(!hex)
	{
		for(cpos=0; cpos<10; cpos++)
		{
			for(kpos=0; kpos<strlen(kcod[cpos]); kpos++)
			{
				if(ch==kcod[cpos][kpos])
				{
					return;
				}
			}
		}
	}
	else
	{
		for(cpos=0; cpos<10; cpos++)
		{
			for(kpos=0; kpos<strlen(hcod[cpos]); kpos++)
			{
				if(ch==hcod[cpos][kpos])
				{
					return;
				}
			}
		}
	}
}
		
void NextPos(void)
{
	do
	{
		epos++;
		if(epos>=cnt)
		{
			epos=0;
		}
	}
	while(!IsInput(format[epos]));
	FindCode(estr[epos]);
	first=1;
}

void PrevPos(void)
{
	do
	{
		epos--;
		if(epos<0)
		{
			epos=cnt-1;
		}
	}
	while(!IsInput(format[epos]));
	FindCode(estr[epos]);
	first=1;
}

void SetCode(int code)
{
	if(format[epos]==NUM)
	{
		hex=0;
		cpos=code;
		kpos=0;
		estr[epos]=kcod[cpos][kpos];
		NextPos();
	}
	else
	{
		if(format[epos]==HEX)
		{
			hex=1;
			if(strlen(hcod[code])>1)
			{
				if(!first)
				{
				 	if(cpos==code)
					{
						if(++kpos>=strlen(hcod[cpos]))
						{
							kpos=0;
						}
					}
					else
					{
						NextPos();
						cpos=code;
						kpos=0;
						first=0;
					}
					estr[epos]=hcod[cpos][kpos];
				}
				else
				{
					cpos=code;
					kpos=0;
					estr[epos]=hcod[cpos][kpos];
					first=0;
				}
			}
			else
			{
				cpos=code;
				kpos=0;
				estr[epos]=hcod[cpos][kpos];
				NextPos();
			}
		}
		else
		{
			hex=0;
			if(!first)
			{
			 	if(cpos==code)
				{
					if(++kpos>=strlen(kcod[cpos]))
					{
						kpos=0;
					}
				}
				else
				{
					NextPos();
					cpos=code;
					kpos=1;
					first=0;
				}
				estr[epos]=kcod[cpos][kpos];
			}
			else
			{
				cpos=code;
				kpos=1;
				estr[epos]=kcod[cpos][kpos];
				first=0;
			}
		}
	}
}

int ReTransform_Msg(char *msg)
{
unsigned i;
int found=0;
char *sptr=msg, *tptr=tstr;

	*tptr=0;
	while(*sptr)
	{
		found=0;
		for(i=0; i<sizeof(tc)/sizeof(tc[0]) && !found; i++)
		{
			rc=*sptr;
			if(rc==tc[i])
			{
				rc=sc[i];
				*(tptr++)='~';
				found=1;
			}
		}
		*tptr=rc;
		++sptr;
		++tptr;
	}
	*tptr=0;
	return strlen(rstr);
}

char *inputd(char *form, char *title, char *defstr, int keys, int frame, int mask, int bhelp, int cols, int tmo)
{
int iw, ih, xsize=0, ysize=0, icon_w=0, icon_h=0;
int i, j, tlen;
int exs, eys, wxs, wxw, wys, wyw, xp, yp;
char trnd[2]={0,0}, tch;
int act_key=-1, last_key=-1, run=1, ipos=0, count=0;
time_t t1, t2, tm1;
 // only for num block
const char knum[12][2]={"1","2","3","4","5","6","7","8","9"," ","0"};
const char kalp[12][6]={"+-*/","abc�","def","ghi","jkl","mno�","pqrs�","tuv�","wxyz","","_,.;"};

	epos=-1;
	cpos=0;
	kpos=0;
	first=1;
	time(&tm1);
	if(cols>25)
	{
		cols=25;
	}
	if(cols<1)
	{
		cols=1;
	}

	format=form;
	estr=strdup(form);
	cnt=strlen(form);
	tlen=i=GetStringLen(title, BIG)+10;
	j=((cnt>cols)?cols:cnt)*exsz;
	if(j>i)
	{
		i=j;
	}
	if(keys)
	{
		j=3*bxsz;
		if(j>i)
		{
			i=j;
		}
	}
	wxw=i+2*xbrd;
	wxw=(keys==1 && wxw < 265) ? 265 : wxw;

	i=(((cnt-1)/cols)+1)*eysz;
	if(keys)
	{
		i+=4*bysz;
	}
	wyw=((keys)?4:2)*ybrd+i;

	wxs=((ex-sx)-wxw)/2;
	wys=(((ey-sy)-wyw)+hsz)/2;
	exs=wxs+(wxw-((cnt>cols)?cols:cnt)*exsz)/2;
	eys=wys+ybrd;

	*estr=0;
	*rstr=0;

	j=0;
	for(i=0; i<strlen(format); i++)
	{
		tch=format[i];
		if(IsInput(tch))
		{
			if(epos==-1)
			{
				epos=i;
			}
			if(defstr && j<strlen(defstr))
			{
				estr[i]=defstr[j++];
			}
			else
			{
				estr[i]=' ';
			}
		}
		else
		{
			estr[i]=format[i];
		}
	}
	estr[i]=0;

	// icon & title
	RenderBox(wxs+6, wys-hsz+6, wxs+wxw+6, wys+wyw+6, radius, COL_SHADOW_PLUS_0);
	RenderBox(wxs, wys-hsz, wxs+wxw, wys+wyw, radius, CMC);
	RenderBox(wxs, wys-hsz, wxs+wxw, wys, radius, CMH);

	png_getsize(ICON_KEYS, &icon_w, &icon_h);
	if(icon_w > 40 || icon_h > 40)
		icon_w = icon_h = xsize = ysize = 40;
	paintIcon(ICON_KEYS, wxs+8, wys-hsz/2-icon_h/2, xsize, ysize, &iw, &ih);
	int tstart, twide;
	if(wxs+8+iw  >= wxs+wxw-8-iw-tlen ) {
		tstart = wxs+8+iw;
		twide  = wxw-8-iw;
	}
	else {
		tstart = wxs;
		twide  = wxw;
	}
	RenderString(title, tstart, wys-7, twide, CENTER, BIG, CMHT);

	int bxs=wxs+(wxw-(3*bxsz))/2;
	int bys=((wys+wyw)-2*ybrd)-4*bysz;
	if(keys == 1)
	{
		png_getsize(ICON_NUMERIC_PAD, &icon_w, &icon_h);
		paintIcon(ICON_NUMERIC_PAD, wxs+wxw/2-icon_w/2, bys+10, 0, 0, &iw, &ih);	
	}
	if(keys == 2)
	{
		for(i=0; i<11; i++)
		{
			if(i!=9) //num fields
			{
				RenderBox(bxs+(i%3)*bxsz, bys+(i/3)*bysz, bxs+((i%3)+1)*bxsz, bys+((i/3)+1)*bysz, radius, CMS);
				RenderBox(bxs+(i%3)*bxsz+2, bys+(i/3)*bysz+2, bxs+((i%3)+1)*bxsz-2, bys+((i/3)+1)*bysz-2, radius, CMC);
				RenderString(knum[i], bxs+(i%3)*bxsz, bys+(i/3)*bysz+bysz/2+8, bxsz, CENTER, MED, CMCIT);
				RenderString(kalp[i], bxs+(i%3)*bxsz, bys+(i/3)*bysz+bysz-2, bxsz, CENTER, SMALL, CMCIT);
			}
		}
	}
	if(keys)
	{
		png_getsize(ICON_BUTTON_RED, &icon_w, &icon_h);
		paintIcon(ICON_BUTTON_RED, bxs-icon_w/2, wys+wyw-ybrd-2-icon_h/2, 0, 0, &iw, &ih);
		RenderString("Gro�/Klein", bxs+icon_w/2+5,wys+wyw-ybrd+10, 3*bxsz, LEFT, SMALL, CMCIT);
		paintIcon(ICON_BUTTON_YELLOW, bxs+125-icon_w/2, wys+wyw-ybrd-2-icon_h/2, 0, 0, &iw, &ih);
		RenderString("l�schen", bxs+125+icon_w/2+5,wys+wyw-ybrd+10, 65, LEFT, SMALL, CMCIT);
	}

	while(run)
	{
		for(i=0; i<cnt; i++)
		{
			xp=i%cols;
			yp=i/cols;
			if(frame && IsInput(format[i]))
			{
				RenderBox(exs+xp*exsz, eys+5+yp*eysz, exs+(xp+1)*exsz, eys+(yp+1)*eysz, 0/*radius*/, CMS);
			}
			RenderBox(exs+xp*exsz+1, eys+5+yp*eysz+1, exs+(xp+1)*exsz-1, eys+(yp+1)*eysz-1, 0/*radius*/, (epos==i)?CMCS:CMC);

			*trnd=(mask && format[i]==NUM && IsNum(estr[i]))?'*':estr[i];
			RenderString(trnd, exs+xp*exsz+2, eys+yp*eysz+tys+7, exsz-2, CENTER, MED, (epos==i)?CMCST:(IsInput(format[i]))?CMCT:CMCIT);
		}
		memcpy(lfb, lbb, var_screeninfo.xres*var_screeninfo.yres*sizeof(uint32_t));
		
		time(&t1);
		i=-1;
		while(i==-1)
		{
			i=GetRCCode(1000);
			if (i != KEY_UP && i != KEY_DOWN && i != KEY_LEFT && i != KEY_RIGHT)
			{
				if (rcvalue == 2 && count < 3)
				{
					count++;
					i = -1;
				}
			}
			time(&t2);
			if(tmo)
			{
				//printf("t2-t1 = %f %i\n", difftime(t2, t1), tmo);
				if((t2-t1)>=tmo)
				{
					i=KEY_EXIT;
				}
			}
			if((((format[epos]!=NUM) && (format[epos]!=HEX)) || ((format[epos]==HEX)&&(strlen(hcod[cpos])>1))) && ((t2-t1)>=ndelay) && last_key>=0)
			{
				act_key=i=-2;
				NextPos();
			}
		}
		count=0;
		act_key=i;
		
		switch(act_key)
		{
			case KEY_0:
				SetCode(0);
			break;
			
			case KEY_1:
				SetCode(1);
			break;
			
			case KEY_2:
				SetCode(2);
			break;
			
			case KEY_3:
				SetCode(3);
			break;
			
			case KEY_4:
				SetCode(4);
			break;
			
			case KEY_5:
				SetCode(5);
			break;
			
			case KEY_6:
				SetCode(6);
			break;
			
			case KEY_7:
				SetCode(7);
			break;
			
			case KEY_8:
				SetCode(8);
			break;
			
			case KEY_9:
				SetCode(9);
			break;
			
			case KEY_RIGHT:
				NextPos();
				act_key=-2;
			break;
			
			case KEY_LEFT:
				PrevPos();
				act_key=-2;
			break;
			
			case KEY_VOLUMEUP:
				ipos=epos;
				while(IsInput(format[ipos+1]) && ((ipos+1)<cnt))
				{
					++ipos;
				}
				while(ipos>epos)
				{
					estr[ipos]=estr[ipos-1];
					--ipos;
				}
				estr[epos]=' ';
//				estr[epos]=((format[epos]=='#')||(format[epos]=='^'))?'0':' ';
				act_key=-1;
			break;

			case KEY_VOLUMEDOWN:
				ipos=epos+1;
				while(IsInput(format[ipos]) && (ipos<cnt))
				{
					estr[ipos-1]=estr[ipos];
					++ipos;
				}
				estr[ipos-1]=' ';
//				estr[ipos-1]=(format[ipos-1]=='#')?'0':' ';
				act_key=-1;
			break;

			case KEY_OK:
				run=0;
			break;
			
			case KEY_MUTE:
				memset(lfb, TRANSP, var_screeninfo.xres*var_screeninfo.yres*sizeof(uint32_t));
				while(GetRCCode(300)!=KEY_MUTE);
			break;

			case KEY_UP:
				if(epos>=cols)
				{
					epos-=cols;
					if(!IsInput(format[epos]))
					{
						NextPos();
					}
				}
				else
				{
					epos=cnt-1;
					if(!IsInput(format[epos]))
					{
						PrevPos();
					}
				}
				act_key=-2;
			break;
			
			case KEY_DOWN:
				if(epos<=(cnt-cols))
				{
					epos+=cols;
					if(!IsInput(format[epos]))
					{
						NextPos();
					}
				}
				else
				{
					epos=0;
					if(!IsInput(format[epos]))
					{
						NextPos();
					}
				}
				act_key=-2;
			break;
			
			case KEY_EXIT:
				free(estr);
				estr=NULL;
				*rstr=0;
				run=0;
			break;
			
			case KEY_RED:
				if(IsAlpha(estr[epos]))
				{
					estr[epos]^=0x20;
				}
				act_key=-2;
			break;
			
			case KEY_YELLOW:
				epos=-1;
				for(i=0; i<strlen(format); i++)
				{
					if(IsInput(format[i]))
					{
						if(epos==-1)
						{
							epos=i;
						}
						estr[i]=' ';
					}
				}
				act_key=-2;
			break;
			
			case KEY_HELP:
				if(bhelp)
				{
					sprintf(estr,"?");
					run=0;
				}
			break;
			
			default:
				act_key=-2;
			break;
		}
		last_key=act_key;
	}
	
	if(estr)
	{
		j=0;
		for(i=0; i<strlen(format); i++)
		{
			if(IsInput(format[i]))
			{
				rstr[j++]=estr[i];
			}
		}
		rstr[j]=0;
		free(estr);
	}
	ReTransform_Msg(rstr);
	return rstr;
}

