/*±¾ÎÄ¼þÓÃÓÚÒ»Ð©¹«ÓÃµÄÏµÍ³º¯Êý£¬¿ÉÒÔ¹©ÏµÍ³ºÍÒ³Ãæµ÷ÓÃ£¬¼´´ËÒ³º¯Êý¾ùÎª¿ÉÖØÈëº¯Êý*/

#include "User.h"


extern u16 ff_convert(u16 src,u32 dir);
//StrUtf8:utf8×Ö·û´®
//StrOem:CÓïÑÔÈÏÊ¶µÄ×Ö·û´®±àÂë
//StrOemByte:×Ö·û¸öÊý£¬strlen(StrOem)£¬±ØÐë²»Ð¡ÓÚstrlen(StrUtf8)
//return:StrOem or NULL
u8 *UTF8_To_OEM(const u8 *StrUtf8,u8 *StrOem,u16 StrOemByte)
{ 
    u8 HiByte,MiByte,LoByte;
    u16 HandleByte=0;
    u16 Tmp;
 
    while((*StrUtf8)&&(HandleByte<StrOemByte))
    {
        if (*StrUtf8 > 0x00 && *StrUtf8 <= 0x7F) //´¦Àíµ¥×Ö½ÚUTF8×Ö·û£¨Ó¢ÎÄ×ÖÄ¸¡¢Êý×Ö£©
        {
            StrOem[HandleByte++] = *StrUtf8;
        }
        else if (((*StrUtf8) & 0xE0) == 0xC0) //´¦ÀíË«×Ö½ÚUTF8×Ö·û
        {
            HiByte = *StrUtf8;
            StrUtf8++;
            LoByte = *StrUtf8;
            if ((LoByte & 0xC0) != 0x80)  //¼ì²éÊÇ·ñÎªºÏ·¨µÄUTF8×Ö·û±íÊ¾
            {
                return NULL; //Èç¹û²»ÊÇÔò±¨´í
            }
   
            Tmp =((HiByte&0x1f) << 6) + (LoByte & 0x3F);//ÏÈ×ª»»³Éunicode
            Tmp=ff_convert(Tmp,0);//ÔÙÍ¨¹ýÎÄ¼þÏµÍ³ÏÖ³ÉµÄº¯Êý×ª»¯³Éoem
            StrOem[HandleByte++] = Tmp>>8;
            StrOem[HandleByte++] = Tmp&0xff;
        }
        else if (((*StrUtf8) & 0xF0) == 0xE0) //´¦ÀíÈý×Ö½ÚUTF8×Ö·û
        {
            HiByte = *StrUtf8;
            StrUtf8++;
            MiByte = *StrUtf8;
            StrUtf8++;
            LoByte = *StrUtf8;
            if (((MiByte & 0xC0) != 0x80) || ((LoByte & 0xC0) != 0x80))
            {
                return NULL;
            }
            Tmp = ((HiByte&0x0f)<<12)+((MiByte&0x3f)<<6)+(LoByte&0x3f);//ÏÈ×ª»»³Éunicode
            Tmp=ff_convert(Tmp,0);//ÔÙÍ¨¹ýÎÄ¼þÏµÍ³ÏÖ³ÉµÄº¯Êý×ª»¯³Éoem
            StrOem[HandleByte++] = Tmp>>8;
            StrOem[HandleByte++] = Tmp&0xff;
        }
        else //¶ÔÓÚÆäËû×Ö½ÚÊýµÄUTF8×Ö·û²»½øÐÐ´¦Àí
        {
            return NULL;
        }
        
        StrUtf8 ++;
    }
    
    StrOem[HandleByte] = 0;//Ìî³ä½áÊø·û

    return (u8 *)StrOem;
}

//pStr±ØÐëÊÇÊ®½øÖÆµÄÊý×Ö£¬Èç¹û²»·ûºÏÒªÇó£¬·µ»Ø0
u32 StrToUint(u8 *pStr)         
{
	u32 i=0,sum=0;
	
	while(pStr[i])            //µ±str[i]²»Îª\0Ê±Ö´ÐÐÑ­»·
	{
		if(pStr[i]<'0'||pStr[i]>'9') return 0;//str²»ÕýÈ·
		sum=sum*10+(pStr[i]-'0');
		i++;
	}
	
	return(sum);
} 

#if 0
void MemSet(void *Dst,u8 C,u16 Byte)
{
	if(!Byte) return;

	for(Byte-=1;Byte;Byte--)
	{
		((u8 *)Dst)[Byte]=C;
	}
	((u8 *)Dst)[0]=C;
}

void MemCpy(void *Dst,const void *Src,u16 Byte)
{
	if(!Byte) return;
	
	for(Byte-=1;Byte;Byte--)
	{
		((u8 *)Dst)[Byte]=((u8 *)Src)[Byte];
	}
	((u8 *)Dst)[0]=((u8 *)Src)[0];
}
#endif

//±È½ÏÁ½¸öÄÚ´æ¿éÖÐ×Ö½ÚÊÇ·ñÏàÍ¬£¬·µ»ØTRUE±íÊ¾ÏàÍ¬£¬LenÖ¸¶¨±È½Ï³¤¶È
bool CompareBuf(u8 *Buf1,u8 *Buf2,u16 Len)
{
	u16 i;

	for(i=0;i<Len;i++)
	{
		if(Buf1[i]!=Buf2[i]) return FALSE;
	}

	return TRUE;
}

//Buf Òª´òÓ¡µÄÄÚ´æ
//Len Òª´òÓ¡µÄ×Ö½ÚÊý
//RawLen Ã¿ÐÐÒª´òÓ¡µÄ×Ö½ÚÊý
void DisplayBuf(const u8 *Buf,u16 Len,u8 RawLen)
{
	int i;
	
	for(i=0;i<Len;i++)
	{
		Debug("0x%02x ",Buf[i]);
		if(i%RawLen==(RawLen-1)) Debug("\n\r");
	}
	Debug("\n\r");
}

#if 0
//¿½±´ÎÄ¼þ
bool FileCpy(u8 *DstPath,u8 *SrcPath)
{
	u32 ReadByte,WrtByte;
	u8 *Buf=OS_Mallco(512);
	FIL *pDstFileObj=OS_Mallco(sizeof(FIL));
	FIL *pSrcFileObj=OS_Mallco(sizeof(FIL));
	bool Ret=TRUE;

	if (f_open(pSrcFileObj ,(void *)SrcPath, FA_OPEN_EXISTING | FA_READ) != FR_OK ) 
	{
		Debug("Open Src File error!\n\r");
		Ret=FALSE;
		goto CopyEnd;
	}

	if (f_open(pDstFileObj ,(void *)DstPath, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK ) 
	{
		Debug("Open Dst File error!\n\r");
		f_close(pSrcFileObj);
		Ret=FALSE;
		goto CopyEnd;
	}

	while(1)
	{
		if(f_read(pSrcFileObj,(void *)Buf, sizeof(Buf), &ReadByte)==FR_OK)
		{
			if(ReadByte==0)
			{
				f_close(pSrcFileObj);
				f_close(pDstFileObj);
				break;
			}
			
			if(f_write(pDstFileObj,(void *)Buf,ReadByte,&WrtByte)!=FR_OK)
			{
				f_close(pSrcFileObj);
				f_close(pDstFileObj);
				Ret=FALSE;
				goto CopyEnd;
			}
		}
		else
		{
			Debug("Read Src File error!\n\r");
			f_close(pSrcFileObj);
			f_close(pDstFileObj);
			Ret=FALSE;
			goto CopyEnd;
		}
	}

CopyEnd:
	OS_Free(pSrcFileObj);
	OS_Free(pDstFileObj);
	OS_Free(Buf);
	return Ret;
}
#endif

//±¾º¯Êý½«ÆÁÄ»ÏÔÊ¾´æ·Å³ÉbmpÎÄ¼þµ½SnapshotÄ¿Â¼
//Çë×¢Òâtf¿¨SnapshotÄ¿Â¼ÏÂÒª·ÅÒ»ÕÅ¿Õ°×µÄ240 320 bmpÍ¼Æ¬Blank.bmp
#define BUF_PIX_NUM	60 //Ã¿´Î¶Á60¸öÏñËØµÄÐÅÏ¢
bool PrtScreen(void)
{
	static int num=0;
	int row,column;
	GUI_REGION ReadRegion;
	u8 Buf[BUF_PIX_NUM*3];
	u8 BlankPath[]="Picture/Blank.bmp";
	u8 Path[40]="";
	FS_FILE *pSnapBmpObj; 
	bool Ret=TRUE;
	
	num++;
	sprintf((void *)Path,"Picture/Snapshot%d-%04x.bmp",num,Rand(0xffff));
	Debug(" #Print screen to image file:%s\n\r #Please wait a moment...\n\r",Path);

	
	if(FS_FileCpy((void *)BlankPath,(void *)Path)==FALSE)
	{
		Debug("File copy error!\n\r");
		Ret=FALSE;
		goto PrtScnEnd;
	}

	if ((pSnapBmpObj =FS_FOpen( (void *)Path, FA_OPEN_EXISTING | FA_WRITE)) == 0 ) 
	{
		Debug("Open bmp error!\n\r");
		Ret=FALSE;
		goto PrtScnEnd;
	}

	if(FS_FSeek(pSnapBmpObj,sizeof(BMP_INFO),FS_SEEK_SET)==-1)//ÒÆµ½Î»Í¼É«²Ê±í
	{
		Debug("Bmp fseek err!\r\n");
		Ret=FALSE;
		goto PrtScnEnd;
	}	
	
	ReadRegion.w=LCD_WIDTH/4;
	ReadRegion.h=1;
	
	for(row=LCD_HIGHT-1;row>=0;row--)
	{
		ReadRegion.y=row;

		for(column=0;column<LCD_WIDTH;column+=BUF_PIX_NUM)
		{
			ReadRegion.x=column;
			Gui_ReadRegion24Bit(Buf,&ReadRegion);
			
			if((FS_FWrite((void *)Buf,sizeof(Buf),1,pSnapBmpObj))==0)
			{
					Ret=FALSE;
					goto PrtScnEnd;
			}
		}
	}

	Debug(" #Save the screen to file success!\n\r");

PrtScnEnd:
	if(pSnapBmpObj)
		FS_FClose(pSnapBmpObj);
	return Ret;
}

//¶ÁÈ¡ÆÁÄ»µÄÄ³¿éÇøÓòµ½ÎÄ¼þ
//Ö§³ÖµÄ×î´óÇøÓò¿í¶ÈÓ¦Ð¡ÓÚPRINT_SRC_BUF_LENµÄÒ»°ë!!!
#define PRINT_SRC_BUF_LEN 4096
bool PrtScreenToBin(u8 *pNewFilePath,u16 x,u16 y,u16 w,u16 h)
{
	unsigned int WrtByte;
	u8 *Buf=OS_Mallco(PRINT_SRC_BUF_LEN);
	FS_FILE *pDstFileObj;
	GUI_REGION ReadRegion;
	u16 Rows,LastRows,Num;
	bool Ret=TRUE;

	if ((pDstFileObj=FS_FOpen((void *)pNewFilePath, FA_CREATE_ALWAYS | FA_WRITE) )== 0 ) 
	{
		Debug("Open Dst File error!\n\r");
		Ret=FALSE;
		goto PrtEnd;
	}

	//¼ÆËãÃ¿´Î¶ÁÈ¡µuÄ¸ß¶È
	Rows=(PRINT_SRC_BUF_LEN>>1)/w;
	if(Rows>h)	Rows=h;
	else if(Rows==0) //ÇøÓòÌ«´ó
	{
		Debug("Error:Save Region is too big!!!\n\r");
		Ret=FALSE;
		goto PrtEnd;
	}

	Num=h/Rows;//¼ÆËã¶ÁÈ¡´ÎÊý-1

	LastRows=h%Rows;//¼ÆËã×îºóÒ»´ÎµÄ¸ß¶È

	//¶¨ÒåÐ¡¿é
	ReadRegion.x=x;
	ReadRegion.y=y;
	ReadRegion.w=w;
	ReadRegion.h=Rows;
	WrtByte=Rows*w*2;
	
	for(;Num;Num--)
	{
		Gui_ReadRegion16Bit((void *)Buf,&ReadRegion);
		ReadRegion.y+=Rows;
		if((FS_FWrite((void *)Buf,WrtByte,1,pDstFileObj))==0)
		{
			Ret=FALSE;
			goto PrtEnd;
		}
	}
	
	//×îºóÒ»´Î
	if(LastRows)
	{
		ReadRegion.h=LastRows;
		Gui_ReadRegion16Bit((void *)Buf,&ReadRegion);
		ReadRegion.y+=Rows;
		
		if((FS_FWrite((void *)Buf,LastRows*w*2,1,pDstFileObj))==0)
		{
			Ret=FALSE;
			goto PrtEnd;
		}
	}

PrtEnd:
	if(pDstFileObj)
		FS_FClose(pDstFileObj);
	OS_Free(Buf);
	return Ret;
}

//·ÖÎöµ¥¸öºó×ºÃû
//suffixÄÚÈÝÀàËÆÓÚ".mp3"
bool CheckSuffix(u8 *Str,u8 *Suffix,u8 SuffixLen)
{
	u8 Len;
	u8 i;

	if(Suffix==NULL||(!Suffix[0])) return TRUE;
	Len=strlen((void *)Str);
	
	if(Len<=SuffixLen) return FALSE;
	for(i=SuffixLen;i;i--)
	{
		if(Str[Len-i]!=Suffix[SuffixLen-i]) return FALSE;
	}

	return TRUE;
}

//·ÖÎö¶à¸öºó×ºÃû
//SuffixStrÄÚÈÝÀàËÆÓÚ".mp3|.wav|.txt"
//Èç¹ûSuffixStrÎª¿Õ£¬Ôò±íÊ¾²»¹ýÂË
bool CheckMultiSuffix(u8 *Str,u8 *SuffixStr)
{
	u8 i,j,SuffixStrLen;

	if(SuffixStr==NULL||(!SuffixStr[0])) return TRUE;

	SuffixStrLen=strlen((void *)SuffixStr)+1;
	
	for(i=0,j=0;j<SuffixStrLen;j++)
	{
		if(SuffixStr[j]=='|') //Ç°ÃæµÄ
		{
			if(CheckSuffix(Str,&SuffixStr[i],j-i)==TRUE) return TRUE;
			i=j+1;
		}
		else if(SuffixStr[j]==0)//×îºóÒ»¸ö
		{
			return CheckSuffix(Str,&SuffixStr[i],j-i);
		}
	}		
	
	return FALSE;
}



//¼ÆËãhash33Öµ
u32 MakeHash33(u8 *pData,u32 Len)
{
	u32 hash=0;
	u32 i=0;

	for(;i<Len;i++)
	{
		hash+=pData[i];
		hash+=(hash<<5);
	}

	return hash;
}

/*Table of CRC values for high-order byte*/
const static u8 auchCRCHi[]={
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

/*Table of CRC values for low-order byte*/
const static char auchCRCLo[]={
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2,0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04,
0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8,
0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6,0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10,
0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE,0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA,0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C,
0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0,
0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62,0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE,0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C,
0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76,0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,
0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,
0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A,0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86,0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

//¼ÆËãcrc
u16 CRC16(const u8 *pData,u16 Len)
{
	u8 uchCRCHi=0xff; /*Ð£ÑéÂë¸ß×Ö½Ú³õÖµ*/
	u8 uchCRCLo=0xff;/* Ð£ÑéÂëµÍ×Ö½Ú³õÖµ*/
	u16 uIndex;       

	while(Len--)
	{
		uIndex=uchCRCHi^*pData++;
		uchCRCHi=uchCRCLo^auchCRCHi[uIndex];
		uchCRCLo=auchCRCLo[uIndex];
	}
	
	return(uchCRCHi<<8|uchCRCLo);
}

//½»»»u16µÄ¸ßµÍ×Ö½Ú
//0x1234 -> 0x3412
u16 Rev16(u16 Data)
{
	return __REV16(Data);
}

//·­×ªu32µÄ×Ö½ÚË³Ðò
//0x12345678 -> 0x78563412
u32 Rev32(u32 Data)
{
	return __REV(Data);
}

//·µ»Ø·Ç0Ëæ»úÖµ
//Èç¹ûÏëµÃµ½°üº¬0µÄËæ»úÖµ£¬ÓÒÒÆ1Î»¼´¿É¡£
u32 Rand(u32 Mask)
{
	u32 RandNum=(ADC_GetConversionValue(ADC1)*(*((volatile u32 *)0xE000E018)))&Mask;

	return (RandNum?RandNum:0x01);
}

//»ñÈ¡Ó²¼þÎ¨Ò»ID
u32 GetHwID(void)
{
	static u32 Id=0;

	if(Id) return Id;
	else
	{
		Id=MakeHash33((u8 *)0x1FFFF7E8,96);
		return Id;
	}
}



