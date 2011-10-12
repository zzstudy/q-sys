#include "System.h"
#include "Theme.h"
#include "CtrlObjImg.h"

#define CO_Debug Debug

//***********************************变量定义*********************************************
static const PAGE_CONTROL_NUM *gpCtrlObjNum=NULL;//当前页面控件个数记录

//用于记录页面临时参数的指针数组。[0]未使用，所以要加一个元素
static void *gPageDataPtrRecord[MAX_PAGE_LAYER_NUM+1];

//图标按键替换的临时后缀存储数组
static REP_IMG_SUFX gRepImgSufx[MAX_IMG_KEY_NUM];

//文字按键替换的临时字符串指针存储数组
static u8 *gRepKeyNameCon[MAX_CHAR_KEY_NUM];

static const IMG_TCH_OBJ *gpCurImgTchCon=NULL; //当前按键区域集合
static const CHAR_TCH_OBJ *gpCurCharTchCon=NULL; //当前按键区域集合

//动态控件存储实体指针
static void *gCtrlObjPtrBuf[MAX_DYN_CTRL_OBJ_NUM];//用来存储下列控件指针的实体
//IMG_TCH_OBJ **gpDynImgTchCon=NULL; //动态图片按键集合
//CHAR_TCH_OBJ **gpDynCharTchCon=NULL;//动态文字按键集合
#ifdef QSYS_FRAME_FULL	
//YES_NO_OBJ **gpYesNoCon=NULL;
//NUM_BOX_OBJ **gpNumBoxCon=NULL;
//STR_OPT_OBJ **gpStrOptCon=NULL;
//STR_INPUT_OBJ **gpStrInputCon=NULL;
#endif

//***********************************外部引用*********************************************
extern u8 GetCurLayerNum(void);
extern const PAGE_ATTRIBUTE *GetPageByIdx(u8 PageIdx);
extern TOUCH_REGION *GetTouchInfoByIdx(u8 Idx);

//***********************************函数定义*********************************************
void ControlObjInit(void)//系统启动时的初始化
{	
	u16 i;
	
	MemSet(gPageDataPtrRecord,0,(MAX_PAGE_LAYER_NUM+1)*sizeof(void *));//清空临时页面变量保存指针

	for(i=0;i<Q_GetPageTotal();i++)
	{
		if((GetPageByIdx(i)->CtrlObjNum.ImgTchNum>MAX_IMG_KEY_NUM)
			||(GetPageByIdx(i)->CtrlObjNum.CharTchNum>MAX_CHAR_KEY_NUM))
		{//页面的按键区域超过支持数目
			Debug("%s touch region num is too much!%d %d\n\r",GetPageByIdx(i)->Name,
			GetPageByIdx(i)->CtrlObjNum.ImgTchNum,GetPageByIdx(i)->CtrlObjNum.CharTchNum);
			Q_ErrorStopScreen("!!!New page's touch region num is over MAX_IMG_KEY_NUM or MAX_CHAR_KEY_NUM");
		}

		if((GetPageByIdx(i)->CtrlObjNum.DynImgTchNum
			+GetPageByIdx(i)->CtrlObjNum.DynCharTchNum
#ifdef QSYS_FRAME_FULL	
			+GetPageByIdx(i)->CtrlObjNum.YesNoNum
			+GetPageByIdx(i)->CtrlObjNum.NumBoxNum
			+GetPageByIdx(i)->CtrlObjNum.StrOptBoxNum
			+GetPageByIdx(i)->CtrlObjNum.StrInputBoxNum
#endif
			)>MAX_DYN_CTRL_OBJ_NUM)
		{//页面的控件个数超过支持数目
			Debug("%s control object num is too much!\n\r",GetPageByIdx(i)->Name);
			Q_ErrorStopScreen("!!!Control Object num is over MAX_DYN_CTRL_OBJ_NUM!!!");
		}
	}
}

//进入新页面时复制所有控件的区域到统一的存储区
//返回区域个数
//拷贝顺序如下:
//图片按键
//文字按键
//动态图片按键
//动态文字按键
//YesNo
//NumBox
//StrOpt
//StrInput
static void CopyCtrlObjTouchReg(TOUCH_REGION *TouchRegsBuf)
{
	u8 Idx;
	u8 Cnt=0;

	IMG_TCH_OBJ **pDynImgTchCon=(void *)gCtrlObjPtrBuf; //动态图片按键集合
	CHAR_TCH_OBJ **pDynCharTchCon=(void *)&pDynImgTchCon[gpCtrlObjNum->DynImgTchNum];//动态文字按键集合
#ifdef QSYS_FRAME_FULL	
	YES_NO_OBJ **pYesNoCon=(void *)&pDynCharTchCon[gpCtrlObjNum->DynCharTchNum];
	NUM_BOX_OBJ **pNumBoxCon=(void *)&pYesNoCon[gpCtrlObjNum->YesNoNum];
	STR_OPT_OBJ **pStrOptCon=(void *)&pNumBoxCon[gpCtrlObjNum->NumBoxNum];
	STR_INPUT_OBJ **pStrInputCon=(void *)&pStrOptCon[gpCtrlObjNum->StrOptBoxNum];
#endif

	//图片按键区域
	for(Idx=0;Idx<gpCtrlObjNum->ImgTchNum;Idx++,Cnt++)
	{
		TouchRegsBuf[Cnt].x=gpCurImgTchCon[Idx].x;
		TouchRegsBuf[Cnt].y=gpCurImgTchCon[Idx].y;
		TouchRegsBuf[Cnt].w=gpCurImgTchCon[Idx].w;
		TouchRegsBuf[Cnt].h=gpCurImgTchCon[Idx].h;
		TouchRegsBuf[Cnt].ObjID=gpCurImgTchCon[Idx].ObjID;
		TouchRegsBuf[Cnt].Type=COT_ImgTch;
		TouchRegsBuf[Cnt].Index=Idx;
		TouchRegsBuf[Cnt].OptionsMask=(u8)(gpCurImgTchCon[Idx].OptionsMask&0xff);
	}
				
	//文字按键区域
	for(Idx=0;Idx<gpCtrlObjNum->CharTchNum;Idx++,Cnt++)
	{
		TouchRegsBuf[Cnt].x=gpCurCharTchCon[Idx].x;
		TouchRegsBuf[Cnt].y=gpCurCharTchCon[Idx].y;
		TouchRegsBuf[Cnt].w=gpCurCharTchCon[Idx].w;
		TouchRegsBuf[Cnt].h=gpCurCharTchCon[Idx].h;
		TouchRegsBuf[Cnt].ObjID=gpCurCharTchCon[Idx].ObjID;
		TouchRegsBuf[Cnt].Type=COT_CharTch;
		TouchRegsBuf[Cnt].Index=Idx;
		TouchRegsBuf[Cnt].OptionsMask=(u8)(gpCurCharTchCon[Idx].OptionsMask&0xff);
	}

	//动态图片按键区域
	for(Idx=0;Idx<gpCtrlObjNum->DynImgTchNum;Idx++,Cnt++)
	{
		if(pDynImgTchCon[Idx]!=NULL)
		{
			TouchRegsBuf[Cnt].x=pDynImgTchCon[Idx]->x;
			TouchRegsBuf[Cnt].y=pDynImgTchCon[Idx]->y;
			TouchRegsBuf[Cnt].w=pDynImgTchCon[Idx]->w;
			TouchRegsBuf[Cnt].h=pDynImgTchCon[Idx]->h;
			TouchRegsBuf[Cnt].ObjID=pDynImgTchCon[Idx]->ObjID;
			TouchRegsBuf[Cnt].Type=COT_DynImg;
			TouchRegsBuf[Cnt].Index=Idx;
			TouchRegsBuf[Cnt].OptionsMask=(u8)(pDynImgTchCon[Idx]->OptionsMask&0xff);
		}
	}

	//动态文字按键区域
	for(Idx=0;Idx<gpCtrlObjNum->DynCharTchNum;Idx++,Cnt++)
	{
		if(pDynCharTchCon[Idx]!=NULL)
		{
			TouchRegsBuf[Cnt].x=pDynCharTchCon[Idx]->x;
			TouchRegsBuf[Cnt].y=pDynCharTchCon[Idx]->y;
			TouchRegsBuf[Cnt].w=pDynCharTchCon[Idx]->w;
			TouchRegsBuf[Cnt].h=pDynCharTchCon[Idx]->h;
			TouchRegsBuf[Cnt].ObjID=pDynCharTchCon[Idx]->ObjID;
			TouchRegsBuf[Cnt].Type=COT_DynChar;
			TouchRegsBuf[Cnt].Index=Idx;
			TouchRegsBuf[Cnt].OptionsMask=(u8)(pDynCharTchCon[Idx]->OptionsMask&0xff);
		}
	}

#ifdef QSYS_FRAME_FULL	
	//yes no
	for(Idx=0;Idx<gpCtrlObjNum->YesNoNum;Idx++,Cnt++)
	{
		if(pYesNoCon[Idx]!=NULL)
		{
			TouchRegsBuf[Cnt].x=pYesNoCon[Idx]->x;
			TouchRegsBuf[Cnt].y=pYesNoCon[Idx]->y;
			TouchRegsBuf[Cnt].w=CO_YES_NO_W;
			TouchRegsBuf[Cnt].h=CO_YES_NO_H;
			TouchRegsBuf[Cnt].ObjID=pYesNoCon[Idx]->ObjID;
			TouchRegsBuf[Cnt].Type=COT_YesNo;
			TouchRegsBuf[Cnt].Index=Idx;
			TouchRegsBuf[Cnt].OptionsMask=(u8)(PrsMsk|RelMsk|ReVMsk);
		}
	}

	//num box
	for(Idx=0;Idx<gpCtrlObjNum->NumBoxNum;Idx++,Cnt++)
	{
		if(pNumBoxCon[Idx]!=NULL)
		{
			TouchRegsBuf[Cnt].x=pNumBoxCon[Idx]->x;
			TouchRegsBuf[Cnt].y=pNumBoxCon[Idx]->y;
			TouchRegsBuf[Cnt].w=pNumBoxCon[Idx]->w;
			TouchRegsBuf[Cnt].h=CO_NUM_H;
			TouchRegsBuf[Cnt].ObjID=pNumBoxCon[Idx]->ObjID;
			TouchRegsBuf[Cnt].Type=COT_NumBox;
			TouchRegsBuf[Cnt].Index=Idx;
			TouchRegsBuf[Cnt].OptionsMask=0;//(u8)(pNumBoxCon[Idx]->OptionsMask&0xff);
		}
	}
#if 0
	//str option
	for(Idx=0;Idx<gpCtrlObjNum->StrOptBoxNum;Idx++,Cnt++)
	{
		if(pStrOptCon[Idx]!=NULL)
		{
			TouchRegsBuf[Cnt].x=pStrOptCon[Idx]->x;
			TouchRegsBuf[Cnt].y=pStrOptCon[Idx]->y;
			TouchRegsBuf[Cnt].w=pStrOptCon[Idx]->w;
			TouchRegsBuf[Cnt].h=CO_STR_OPT_H;
			TouchRegsBuf[Cnt].ObjID=pStrOptCon[Idx]->ObjID;
			TouchRegsBuf[Cnt].Type=COT_StrOpt;
			TouchRegsBuf[Cnt].Index=Idx;
			TouchRegsBuf[Cnt].OptionsMask=(u8)(pStrOptCon[Idx]->OptionsMask&0xff);
		}
	}

	//str input
	for(Idx=0;Idx<gpCtrlObjNum->StrInputBoxNum;Idx++,Cnt++)
	{
		if(pStrInputCon[Idx]!=NULL)
		{
			TouchRegsBuf[Cnt].x=pStrInputCon[Idx]->x;
			TouchRegsBuf[Cnt].y=pStrInputCon[Idx]->y;
			TouchRegsBuf[Cnt].w=pStrInputCon[Idx]->w;
			TouchRegsBuf[Cnt].h=pStrInputCon[Idx]->h;
			TouchRegsBuf[Cnt].ObjID=pStrInputCon[Idx]->ObjID;
			TouchRegsBuf[Cnt].Type=COT_StrInput;
			TouchRegsBuf[Cnt].Index=Idx;
			TouchRegsBuf[Cnt].OptionsMask=(u8)(pStrInputCon[Idx]->OptionsMask&0xff);
		}
	}
#endif
#endif
}

//切换页面时执行的控件部分数据处理
void PageSwithcCtrlObjDataHandler(const PAGE_ATTRIBUTE *pNewPage)
{
	gpCtrlObjNum=&gpCurrentPage->CtrlObjNum;//得到控件个数结构体

	gpCurImgTchCon=gpCurrentPage->pImgTchRegCon;//切换基本触摸区域集指针
	gpCurCharTchCon=gpCurrentPage->pCharTchRegCon;
	
	if(gTouchRegionNum) Q_Free(gpTouchRegions);//收回上个页面的内存
	
	gTouchRegionNum=gpCtrlObjNum->ImgTchNum
									+gpCtrlObjNum->CharTchNum
									+gpCtrlObjNum->DynImgTchNum
									+gpCtrlObjNum->DynCharTchNum
#ifdef QSYS_FRAME_FULL	
									+gpCtrlObjNum->YesNoNum//yes or no 选项的个数
									+gpCtrlObjNum->NumBoxNum//数字框的个数
									+gpCtrlObjNum->StrOptBoxNum//字符串选项框的个数
									+gpCtrlObjNum->StrInputBoxNum//字符串输入框的个数
#endif
	;
	if(gTouchRegionNum)//申请新内存用于存放坐标信息
	{
		gpTouchRegions=Q_Mallco(gTouchRegionNum*sizeof(TOUCH_REGION));//复制触碰区域
		CopyCtrlObjTouchReg(gpTouchRegions);//将所有控件的坐标信息复制到触碰区域记录结构体，供触摸线程使用
	}
}

//通过键值来寻找索引，返回0xff表示未找到
//只支持找到的第一个!!对于一个键值多个按键的情况不支持后面的按键
static u8 FindImgTchIdx(u8 Key)
{
	u8 i=0;

	for(;i<gpCtrlObjNum->ImgTchNum;i++)
		if(gpCurImgTchCon[i].ObjID==Key) return i;

	return 0xff;
}

static u8 FindCharTchIdx(u8 Key)
{
	u8 i=0;

	for(;i<gpCtrlObjNum->CharTchNum;i++)
		if(gpCurCharTchCon[i].ObjID==Key) return i;

	return 0xff;
}

//用于显示图片触摸区域的图片
static bool ImgTchDisplay(u8 Index,INPUT_EVT_TYPE InType,bool IsDyn)
{
	IMG_TCH_OBJ *pTouchRegion;
	GUI_REGION BmpRegion;
	GUI_REGION BgRegion;
	u8 PathBuf[MAX_BMP_PATH_LEN]="";
	u8 ImgSuffix=0,RepSuffix=0;
	bool LandScapeMode=Gui_GetLandScapeMode();//存储当前横屏模式
	bool Ret=TRUE;
	COLOR_TYPE RegionColor;

	if(IsDyn)//是否是页面动态组的图标
	{
		pTouchRegion=gCtrlObjPtrBuf[Index];

		switch(InType)//检查事件类型
		{
			case Input_TchNormal:
				RegionColor=FatColor(NORMAL_REPLACE_COLOR);
				ImgSuffix=IMG_KEY_NOR_DEF_SUFFIX;
				break;
			case Input_TchPress:
				RegionColor=FatColor(PRESS_REPLACE_COLOR);
				ImgSuffix=IMG_KEY_PRS_DEF_SUFFIX;
				break;
			case Input_TchRelease:
			case Input_TchReleaseVain:
				RegionColor=FatColor(RELEASE_REPLACE_COLOR);
				ImgSuffix=IMG_KEY_REL_DEF_SUFFIX;
				break;
			default:
				return FALSE;
		}
	}
	else //页面固定按键
	{
		pTouchRegion=(void *)&gpCurImgTchCon[Index];
		if(gRepImgSufx[Index])	 RepSuffix=gRepImgSufx[Index];
		
		switch(InType)//检查事件类型
		{
			case Input_TchNormal:
				RegionColor=FatColor(NORMAL_REPLACE_COLOR);
				ImgSuffix=IMG_KEY_NOR_DEF_SUFFIX;
				break;
			case Input_TchPress:
				RegionColor=FatColor(PRESS_REPLACE_COLOR);
				ImgSuffix=IMG_KEY_PRS_DEF_SUFFIX;
				break;
			case Input_TchRelease:
			case Input_TchReleaseVain:
				RegionColor=FatColor(RELEASE_REPLACE_COLOR);
				ImgSuffix=IMG_KEY_REL_DEF_SUFFIX;
				break;
			default:
				return FALSE;
		}
	}
	
	if(pTouchRegion->ImgPathPrefix[0])//指定了路径前缀，说明需要显示按键图标
	{
		if(pTouchRegion->OptionsMask&PathMsk)//使用自定义路径，只需叠加主题路径
		{
			if(strlen((void *)Q_GetNowThemePath())+strlen((void *)pTouchRegion->ImgPathPrefix)>=(MAX_BMP_PATH_LEN-7))
			{
				Debug("!!!Error:Path is too long!!!");
				return FALSE;//路径太长
			}
			if(RepSuffix) sprintf((void *)PathBuf,"%s%s%c%c",Q_GetNowThemePath(),pTouchRegion->ImgPathPrefix,RepSuffix,ImgSuffix);
			else sprintf((void *)PathBuf,"%s%s%c",Q_GetNowThemePath(),pTouchRegion->ImgPathPrefix,ImgSuffix);
		}
		else //不使用自定义路径，则叠加主题路径及页面名文件夹
		{
			if((strlen((void *)Q_GetNowThemePath())+strlen((void *)gpCurrentPage->Name)
				+strlen((void *)pTouchRegion->ImgPathPrefix))>=(MAX_BMP_PATH_LEN-7))
			{
				Debug("!!!Error:Path is too long!!!");
				return FALSE;//路径太长
			}
			if(RepSuffix) sprintf((void *)PathBuf,"%s%s/Btn/%s%c%c",Q_GetNowThemePath(),gpCurrentPage->Name,pTouchRegion->ImgPathPrefix,RepSuffix,ImgSuffix);//得到路径
			else sprintf((void *)PathBuf,"%s%s/Btn/%s%c",Q_GetNowThemePath(),gpCurrentPage->Name,pTouchRegion->ImgPathPrefix,ImgSuffix);//得到路径
		}
		
		if(pTouchRegion->OptionsMask&BinMsk)//是否采用bin文件做图标文件
			strcat((void *)PathBuf,".bin");
		else
			strcat((void *)PathBuf,".bmp");
		
		//根据显示模式计算坐标
		if(pTouchRegion->OptionsMask&LandMsk)
		{//横屏模式
			Gui_SetLandScapeMode(TRUE);
			BmpRegion.x=pTouchRegion->x-pTouchRegion->ImgX;
			BmpRegion.y=pTouchRegion->y+pTouchRegion->ImgY;
			BmpRegion.w=pTouchRegion->w-(pTouchRegion->ImgY<<1);
			BmpRegion.h=pTouchRegion->h-(pTouchRegion->ImgX<<1);
		}
		else		
		{//正常模式
			Gui_SetLandScapeMode(FALSE);
			BmpRegion.x=pTouchRegion->x+pTouchRegion->ImgX;
			BmpRegion.y=pTouchRegion->y+pTouchRegion->ImgY;
			BmpRegion.w=pTouchRegion->w-(pTouchRegion->ImgX<<1);
			BmpRegion.h=pTouchRegion->h-(pTouchRegion->ImgY<<1);
		}		
		
		BmpRegion.Color=pTouchRegion->TransColor;
		if((pTouchRegion->OptionsMask&DbgMsk)||
			((pTouchRegion->OptionsMask&BinMsk)?
			(Gui_DrawImgBin(PathBuf,&BmpRegion)==Gui_No_Such_File):
			(Gui_Draw24Bmp(PathBuf,&BmpRegion)==Gui_No_Such_File)))
		{//调试模式或者打开图标文件失败，则显示触摸区域
			BgRegion.x=pTouchRegion->x;
			BgRegion.y=pTouchRegion->y;
			BgRegion.w=pTouchRegion->w;
			BgRegion.h=pTouchRegion->h;
			BgRegion.Color=FatColor(REGION_BG_COLOR);
			Gui_FillBlock(&BgRegion);	
			BmpRegion.Color=RegionColor;
			Gui_FillBlock(&BmpRegion);

			if(pTouchRegion->Name&&pTouchRegion->Name[0])
			{
				BmpRegion.Color=FatColor(KEY_NAME_COLOR);
				BmpRegion.Space=0;
				if(pTouchRegion->OptionsMask&F16Msk)
					Gui_DrawFont(GBK16_FONT,(void *)pTouchRegion->Name,&BmpRegion);
				else if(pTouchRegion->OptionsMask&B14Msk)
					Gui_DrawFont(ASC14B_FONT,(void *)pTouchRegion->Name,&BmpRegion);
				else
					Gui_DrawFont(GBK12_FONT,(void *)pTouchRegion->Name,&BmpRegion);
			}

			Ret=FALSE;
		}		
		
		Gui_SetLandScapeMode(LandScapeMode);
	}
	else if(pTouchRegion->OptionsMask&DbgMsk)//没有指定后缀，除非开启了DbgMsk,否则不显示任何内容
	{
		//根据显示模式计算坐标
		if(pTouchRegion->OptionsMask&LandMsk)
		{//横屏模式
			Gui_SetLandScapeMode(TRUE);
		}
		else		
		{//正常模式
			Gui_SetLandScapeMode(FALSE);
		}		
		
		//显示触摸区域
		BgRegion.x=pTouchRegion->x;
		BgRegion.y=pTouchRegion->y;
		BgRegion.w=pTouchRegion->w;
		BgRegion.h=pTouchRegion->h;
		BgRegion.Color=FatColor(REGION_BG_COLOR);
		Gui_FillBlock(&BgRegion);	

		if(pTouchRegion->Name&&pTouchRegion->Name[0])//显示字符
		{
			BgRegion.x++;
			BgRegion.y++;
			BgRegion.w-=2;
			BgRegion.h-=2;
			BgRegion.Color=FatColor(KEY_NAME_COLOR);
			BgRegion.Space=0;
			if(pTouchRegion->OptionsMask&F16Msk)
				Gui_DrawFont(GBK16_FONT,(void *)pTouchRegion->Name,&BgRegion);
			else
				Gui_DrawFont(GBK12_FONT,(void *)pTouchRegion->Name,&BgRegion);
		}
	
		Gui_SetLandScapeMode(LandScapeMode);
	}

	return Ret;
}

//用于显示文字触摸区域的文字
static bool CharTchDisplay(u8 Index,INPUT_EVT_TYPE InType,bool IsDyn)
{
	const CHAR_TCH_OBJ *pTouchRegion;
	GUI_REGION CharRegion;
	GUI_REGION BgRegion;
	bool LandScapeMode=Gui_GetLandScapeMode();//存储当前横屏模式
	bool Ret=TRUE;
	u8 *pName;
	COLOR_TYPE CharColor,BgColor;

	if(IsDyn)//检查是不是动态文字按键组
	{
		pTouchRegion=gCtrlObjPtrBuf[gpCtrlObjNum->DynImgTchNum+Index];
		pName=pTouchRegion->Name;
	}
	else//是页面的固定文字按键组
	{
		pTouchRegion=&gpCurCharTchCon[Index];

		if(gRepKeyNameCon[Index]!=NULL)	pName=gRepKeyNameCon[Index];
		else	pName=pTouchRegion->Name;
	}

	switch(InType)//检查事件类型
	{
		case Input_TchNormal:
			CharColor=pTouchRegion->NormalColor;
			BgColor=pTouchRegion->NormalBG;		
			break;
		case Input_TchPress:
			CharColor=pTouchRegion->PressColor;
			BgColor=pTouchRegion->PressBG;	
			break;
		case Input_TchRelease:
		case Input_TchReleaseVain:
			CharColor=pTouchRegion->ReleaseColor;
			BgColor=pTouchRegion->ReleaseBG;	
			break;
		default:
			return FALSE;
	}
			
	if(pName!=NULL&&pName[0])//有文字显示
	{		
		//4	先根据显示模式计算坐标
		if(pTouchRegion->OptionsMask&LandMsk)
		{//横屏模式
			Gui_SetLandScapeMode(TRUE);
			//文字
			if(CharColor!=FatColor(TRANS_COR))//如果不是透明色
			{
				CharRegion.x=pTouchRegion->x-pTouchRegion->CharX;
				CharRegion.y=pTouchRegion->y+pTouchRegion->CharY;
				CharRegion.w=pTouchRegion->w-pTouchRegion->CharY;
				CharRegion.h=pTouchRegion->h-pTouchRegion->CharX;
			}

			if(pTouchRegion->OptionsMask&DbgMsk)//调试模式，将触摸区域显示出来
			{
				BgRegion.x=pTouchRegion->x;
				BgRegion.y=pTouchRegion->y;
				BgRegion.w=pTouchRegion->w;
				BgRegion.h=pTouchRegion->h;
				BgRegion.Color=FatColor(REGION_BG_COLOR);
				Gui_FillBlock(&BgRegion);
			}

			//背景
			if(BgColor!=FatColor(TRANS_COR))//如果不是透明色
			{
				if(pTouchRegion->OptionsMask&RoueMsk) //圆边
				{
					BgRegion.h=pTouchRegion->h-2*(pTouchRegion->Margin>>4);
					BgRegion.w=pTouchRegion->w-2*(pTouchRegion->Margin&0x0f)-BgRegion.h;					
					BgRegion.x=pTouchRegion->x-(pTouchRegion->Margin&0x0f);
					BgRegion.y=pTouchRegion->y+(pTouchRegion->Margin>>4)+(BgRegion.h>>1);
					BgRegion.Color=BgColor;
					Gui_FillBlock(&BgRegion);

					BgRegion.x-=(BgRegion.h>>1);
					BgRegion.w=(BgRegion.h>>1);
					BgRegion.Space=0xc3;
					Gui_DrawCircle(&BgRegion,TRUE);

					BgRegion.y=pTouchRegion->y-(BgRegion.h>>1)+pTouchRegion->w-(pTouchRegion->Margin>>4)-1;
					BgRegion.Space=0x3c;
					Gui_DrawCircle(&BgRegion,TRUE);
				}
				else
				{
					BgRegion.x=pTouchRegion->x-(pTouchRegion->Margin&0x0f);
					BgRegion.y=pTouchRegion->y+(pTouchRegion->Margin>>4);
					BgRegion.w=pTouchRegion->w-2*(pTouchRegion->Margin&0x0f);
					BgRegion.h=pTouchRegion->h-2*(pTouchRegion->Margin>>4);
					BgRegion.Color=BgColor;
					Gui_FillBlock(&BgRegion);//4	显示背景
				}
			}
		}
		else		
		{//正常模式
			Gui_SetLandScapeMode(FALSE);
			//文字
			if(CharColor!=FatColor(TRANS_COR))//如果不是透明色
			{
				CharRegion.x=pTouchRegion->x+pTouchRegion->CharX;
				CharRegion.y=pTouchRegion->y+pTouchRegion->CharY;
				CharRegion.w=pTouchRegion->w-pTouchRegion->CharX;
				CharRegion.h=pTouchRegion->h-pTouchRegion->CharY;
			}

			if(pTouchRegion->OptionsMask&DbgMsk)//调试模式，将触摸区域显示出来
			{
				BgRegion.x=pTouchRegion->x;
				BgRegion.y=pTouchRegion->y;
				BgRegion.w=pTouchRegion->w;
				BgRegion.h=pTouchRegion->h;
				BgRegion.Color=FatColor(REGION_BG_COLOR);
				Gui_FillBlock(&BgRegion);
			}
			
			//背景
			if(BgColor!=FatColor(TRANS_COR))//如果不是透明色
			{
				if(pTouchRegion->OptionsMask&RoueMsk) //圆边
				{
					BgRegion.h=pTouchRegion->h-2*(pTouchRegion->Margin&0x0f);
					BgRegion.w=pTouchRegion->w-2*(pTouchRegion->Margin>>4)-BgRegion.h;
					BgRegion.x=pTouchRegion->x+(pTouchRegion->Margin>>4)+(BgRegion.h>>1);
					BgRegion.y=pTouchRegion->y+(pTouchRegion->Margin&0x0f);
					BgRegion.Color=BgColor;
					Gui_FillBlock(&BgRegion);

					BgRegion.y+=(BgRegion.h>>1);
					BgRegion.w=(BgRegion.h>>1);
					BgRegion.Space=0xf0;
					Gui_DrawCircle(&BgRegion,TRUE);

					BgRegion.x=pTouchRegion->x-(BgRegion.h>>1)+pTouchRegion->w-(pTouchRegion->Margin>>4)-1;
					BgRegion.Space=0x0f;
					Gui_DrawCircle(&BgRegion,TRUE);
				}
				else
				{
					BgRegion.x=pTouchRegion->x+(pTouchRegion->Margin>>4);
					BgRegion.y=pTouchRegion->y+(pTouchRegion->Margin&0x0f);
					BgRegion.w=pTouchRegion->w-2*(pTouchRegion->Margin>>4);
					BgRegion.h=pTouchRegion->h-2*(pTouchRegion->Margin&0x0f);
					BgRegion.Color=BgColor;
					Gui_FillBlock(&BgRegion);//4	显示背景
				}
			}
		}		

		//4	显示文字
		if(CharColor!=FatColor(TRANS_COR))//如果不是透明色
		{
			CharRegion.Space=pTouchRegion->Space;
			CharRegion.Color=CharColor;
			if(pTouchRegion->OptionsMask&F16Msk)
				Gui_DrawFont(GBK16_FONT,(void *)pName,&CharRegion);
			else if(pTouchRegion->OptionsMask&B14Msk)
				Gui_DrawFont(ASC14B_FONT,(void *)pName,&CharRegion);
			else
				Gui_DrawFont(GBK12_FONT,(void *)pName,&CharRegion);
		}

		Gui_SetLandScapeMode(LandScapeMode);//还原屏幕模式
	}
	else if(pTouchRegion->OptionsMask&DbgMsk)//没有指定文字，除非开启了DbgMsk,否则不显示任何内容
	{		
		//4	先根据显示模式计算坐标
		if(pTouchRegion->OptionsMask&LandMsk)
		{//横屏模式
			Gui_SetLandScapeMode(TRUE);
		}
		else		
		{//正常模式
			Gui_SetLandScapeMode(FALSE);
		}		

		BgRegion.x=pTouchRegion->x;
		BgRegion.y=pTouchRegion->y;
		BgRegion.w=pTouchRegion->w;
		BgRegion.h=pTouchRegion->h;
		BgRegion.Color=FatColor(REGION_BG_COLOR);
		Gui_FillBlock(&BgRegion);
		
		Gui_SetLandScapeMode(LandScapeMode);//还原屏幕模式
	}

	return Ret;
}

//将相同键值的按键都显示
void SameOidTchDisplay(u8 ObjID,INPUT_EVT_TYPE InType)
{
	u8 i;
	IMG_TCH_OBJ **pDynImgTchCon=(void *)gCtrlObjPtrBuf; //动态图片按键集合
	CHAR_TCH_OBJ **pDynCharTchCon=(void *)&pDynImgTchCon[gpCtrlObjNum->DynImgTchNum];//动态文字按键集合

	//相同键值的文字按键
	for(i=0;i<gpCtrlObjNum->CharTchNum;i++)//找相同键值的键
	{
		if(gpCurCharTchCon[i].ObjID==ObjID)
			CharTchDisplay(i,InType,FALSE);
	}

	//相同键值的动态文字按键
	for(i=0;i<gpCtrlObjNum->DynCharTchNum;i++)
	{
		if(pDynCharTchCon[i]!=NULL)
			if(pDynCharTchCon[i]->ObjID==ObjID)
				CharTchDisplay(i,InType,TRUE);
	}
	
	//相同键值的图像按键
	for(i=0;i<gpCtrlObjNum->ImgTchNum;i++)//找相同键值的键
	{
		if(gpCurImgTchCon[i].ObjID==ObjID)
			ImgTchDisplay(i,InType,FALSE);
	}

	//相同键值的动态图像按键
	for(i=0;i<gpCtrlObjNum->DynImgTchNum;i++) 
	{
		if(pDynImgTchCon[i]!=NULL)
			if(pDynImgTchCon[i]->ObjID==ObjID)
				ImgTchDisplay(i,InType,TRUE);
	}
}

//有触摸输入时，处理触摸区域事件
TCH_MSG TchCtrlObjHandler(INPUT_EVT_TYPE InType,u8 Idx,TOUCH_INFO *pTouchInfo)
{
	TCH_MSG TchMsg=TM_State_OK;
	u8 TchEvtMsk=GetTouchInfoByIdx(Idx)->OptionsMask;//事件掩码
	u8 ObjID=GetTouchInfoByIdx(Idx)->ObjID;//键值

	switch(InType)//判断事件
	{
		case Input_TchPress:		
			//按下时图标变化
			SameOidTchDisplay(ObjID,InType);
			if(TchEvtMsk&PrsMsk)//需要处理
				TchMsg=gpCurrentPage->TchEvtHandler(ObjID,Tch_Press,pTouchInfo);
			break;
		case Input_TchContinue:
			//不需要检查是否触发，因为在Touch线程里面已经检查了
			TchMsg=gpCurrentPage->TchEvtHandler(ObjID,Tch_Continue,pTouchInfo);
			break;
		case Input_TchRelease:
			//按键释放改变按钮图标并发出事件
			SameOidTchDisplay(ObjID,InType);
			if(TchEvtMsk&RelMsk)//需要处理
				TchMsg=gpCurrentPage->TchEvtHandler(ObjID,Tch_Release,pTouchInfo);
			Allow_Touch_Input();
			break;
		case Input_TchReleaseVain:	
			SameOidTchDisplay(ObjID,InType);
			if(TchEvtMsk&ReVMsk)//需要处理
				TchMsg=gpCurrentPage->TchEvtHandler(ObjID,Tch_ReleaseVain,pTouchInfo);
			Allow_Touch_Input();
			break;
	}

	return TchMsg;
}

#ifdef QSYS_FRAME_FULL	
//有触摸输入时，处理yesno选框事件
//Idx为所有触摸区域集合的索引
TCH_MSG YesNoCtrlObjHandler(INPUT_EVT_TYPE InType,u8 Idx)
{
	YES_NO_OBJ *pYesNo=(void *)gCtrlObjPtrBuf[Idx-gpCtrlObjNum->ImgTchNum-gpCtrlObjNum->CharTchNum];
	GUI_REGION DrawRegion;
	TCH_MSG TchMsg=TM_State_OK;
	u8 ObjID=GetTouchInfoByIdx(Idx)->ObjID;//键值

	DrawRegion.x=pYesNo->x;
	DrawRegion.y=pYesNo->y;
	DrawRegion.w=CO_YES_NO_W;
	DrawRegion.h=CO_YES_NO_H;
	DrawRegion.Color=CO_YES_NO_TRAN_COLOR;
			
	switch(InType)//判断事件
	{
		case Input_TchPress:		//按下时图标变化
			Gui_DrawImgArray(gCtrlObj_OnOff,&DrawRegion);
			break;
		case Input_TchRelease://按键释放改变按钮图标并发出事件
			pYesNo->DefVal=(bool)!pYesNo->DefVal;
			if(pYesNo->DefVal==FALSE)	Gui_DrawImgArray(gCtrlObj_Off,&DrawRegion);
			else 	Gui_DrawImgArray(gCtrlObj_On,&DrawRegion);
			
			TchMsg=gpCurrentPage->YesNoHandler(ObjID,pYesNo->DefVal);
			Allow_Touch_Input();
			break;
		case Input_TchReleaseVain:	
			if(pYesNo->DefVal==FALSE)	Gui_DrawImgArray(gCtrlObj_Off,&DrawRegion);
			else 	Gui_DrawImgArray(gCtrlObj_On,&DrawRegion);
			
			Allow_Touch_Input();
			break;
	}
	
	return TchMsg;
}

//有触摸输入时，处理NumBox事件
//Idx为所有触摸区域集合的索引
TCH_MSG NumBoxCtrlObjHandler(INPUT_EVT_TYPE InType,u8 Idx,TOUCH_INFO *pTouchInfo)
{
	NUM_BOX_OBJ *pNumBox=(void *)gCtrlObjPtrBuf[Idx-gpCtrlObjNum->ImgTchNum-gpCtrlObjNum->CharTchNum];

	switch(InType)
	{
		case Input_TchPress:
			break;
		case Input_TchRelease:
			Q_GotoPage(GotoSubPage,"NumCtrlObjPage",0,pNumBox);//交给NumBoxPage处理
			Allow_Touch_Input();
			break;
		case Input_TchReleaseVain:
			Allow_Touch_Input();
			break;
	}

	
	return 0;
}

//有触摸输入时，处理StrOpt事件
TCH_MSG StrOptCtrlObjHandler(INPUT_EVT_TYPE InType,u8 Idx,TOUCH_INFO *pTouchInfo)
{
	return 0;
}

TCH_MSG StrInputCtrlObjHandler(INPUT_EVT_TYPE InType,u8 Idx,TOUCH_INFO *pTouchInfo)
{
	return 0;
}
#endif

//进入子页面前，保存页面的零时数据
//要保存的数据有:
// 1.页面的动态按键组
//ps 这块看不懂不要勉强
void PushPageCtrlObjData(void)
{
	void *p;

	Debug("PushPageCtrlObjData:%d\n\r",GetCurLayerNum());

	//Debug("%d\n\r",sizeof(IMG_TCH_OBJ*)*MAX_DYNAMIC_IMG_KEY_NUM);
	//Debug("%d\n\r",sizeof(CHAR_TCH_OBJ*)*MAX_DYNAMIC_CHAR_KEY_NUM);
	//Debug("%d\n\r",sizeof(REP_IMG_SUFX)*MAX_IMG_KEY_NUM);
	//Debug("%d\n\r",sizeof(u8 *)*MAX_CHAR_KEY_NUM);
	
	p=gPageDataPtrRecord[GetCurLayerNum()]=Q_Mallco(sizeof(void*)*MAX_DYN_CTRL_OBJ_NUM
								+sizeof(REP_IMG_SUFX)*MAX_IMG_KEY_NUM+sizeof(u8 *)*MAX_CHAR_KEY_NUM);
	
	memcpy(p,gCtrlObjPtrBuf,sizeof(void*)*MAX_DYN_CTRL_OBJ_NUM);
	p=(void *)((u32)p+sizeof(void*)*MAX_DYN_CTRL_OBJ_NUM);
	memcpy(p,(void *)gRepImgSufx,sizeof(REP_IMG_SUFX)*MAX_IMG_KEY_NUM);
	p=(void *)((u32)p+sizeof(REP_IMG_SUFX)*MAX_IMG_KEY_NUM);
	memcpy(p,(void *)gRepKeyNameCon,sizeof(u8 *)*MAX_CHAR_KEY_NUM);
}

//弹出页面数据
void PopPageCtrlObjData(bool DeleteAll)
{	
	if(DeleteAll)
	{
		u8 i;

		Debug("PopPageCtrlObjData Delete All\n\r");
		for(i=0;i<=MAX_PAGE_LAYER_NUM;i++)
			if(gPageDataPtrRecord[i]!=NULL) 
				Q_Free(gPageDataPtrRecord[i]);
	}
	else if(gPageDataPtrRecord[GetCurLayerNum()]!=NULL)
	{
		void *p=gPageDataPtrRecord[GetCurLayerNum()];

		Debug("PopPageCtrlObjData:%d\n\r",GetCurLayerNum());
		
		memcpy((void *)gCtrlObjPtrBuf,p,sizeof(void*)*MAX_DYN_CTRL_OBJ_NUM);
		p=(void *)((u32)p+sizeof(void*)*MAX_DYN_CTRL_OBJ_NUM);
		memcpy((void *)gRepImgSufx,p,sizeof(REP_IMG_SUFX)*MAX_IMG_KEY_NUM);
		p=(void *)((u32)p+sizeof(REP_IMG_SUFX)*MAX_IMG_KEY_NUM);
		memcpy((void *)gRepKeyNameCon,p,sizeof(u8 *)*MAX_CHAR_KEY_NUM);

		Q_Free(gPageDataPtrRecord[GetCurLayerNum()]);
		gPageDataPtrRecord[GetCurLayerNum()]=NULL;
	}
}

//清除页面数据
void CleanPageCtrlObjData(void)
{
	u8 i;

	Debug("CleanPageCtrlObjData\n\r");
	
	//清空控件
	for(i=0;i<MAX_DYN_CTRL_OBJ_NUM;i++) gCtrlObjPtrBuf[i]=NULL;

	//清空临时替换图标的后缀存储
	for(i=0;i<MAX_IMG_KEY_NUM;i++)
		gRepImgSufx[i]=0;
	for(i=0;i<MAX_CHAR_KEY_NUM;i++)
		gRepKeyNameCon[i]=NULL;
}

//激活当前页面下的触摸区域集
SYS_MSG CurrPageCtrlObjInit(INPUT_EVT_TYPE EventType,int IntParam,void *pInfoParam)
{
	IMG_TCH_OBJ **pDynImgTchCon=(void *)gCtrlObjPtrBuf; //动态图片按键集合
	CHAR_TCH_OBJ **pDynCharTchCon=(void *)&pDynImgTchCon[gpCtrlObjNum->DynImgTchNum];//动态文字按键集合
	SYS_MSG SysMsg=SM_State_Faile;
	u8 Index;
	
	CO_Debug("%s : %s\n\r",__FUNCTION__,gpCurrentPage->Name);

	//4 	绘制文字触摸区域
	for(Index=0;Index<gpCtrlObjNum->CharTchNum;Index++)
	{
		CharTchDisplay(Index,Input_TchNormal,FALSE);
	}

	//4 	绘制动态文字触摸区域
	for(Index=0;Index<gpCtrlObjNum->DynCharTchNum;Index++)
	{//显示图标
		if(pDynCharTchCon[Index]!=NULL)
			CharTchDisplay(Index,Input_TchNormal,TRUE);
	}

	//4 	绘制图片触摸区域
	for(Index=0;Index<gpCtrlObjNum->ImgTchNum;Index++)
	{//显示图标
		ImgTchDisplay(Index,Input_TchNormal,FALSE);
	}

	//4 	绘制动态图片触摸区域
	for(Index=0;Index<gpCtrlObjNum->DynImgTchNum;Index++)
	{//显示图标
		if(pDynImgTchCon[Index]!=NULL)
			ImgTchDisplay(Index,Input_TchNormal,TRUE);
	}

	//触发事件
	switch(EventType)
	{
		case Input_GotoSubPage://进入子页面
		case Input_SubPageTranslate://进入新的同级子页面
		case Input_GotoNewPage://进入新页面
				SysMsg=gpCurrentPage->SysEvtHandler(Sys_TouchSetOk,IntParam,pInfoParam);
			break;
		case Input_SubPageReturn://从子页面返回
				SysMsg=gpCurrentPage->SysEvtHandler(Sys_TouchSetOk_SR,IntParam,pInfoParam);
			break;
	}
	
	CO_Debug("%s end: %s\n\r",__FUNCTION__,gpCurrentPage->Name);
	return SysMsg;
}

#if 1//user api
//更改当前触摸域某个按键的显示图片
// 1.只对当前页面有效,转换页面或子页面后失效
// 2.只能指定后缀，比如原来的资源图标是"MusicN.bmp"
//    指定Suffix='T'，则图标资源变成"MusicT.bmp"
//如果Suffix=0，则恢复原始图标
void Q_ChangeImgTchImg(u8 ObjID,u8 Suffix)
{
	OS_DeclareCritical();
	
	if((ObjID=FindImgTchIdx(ObjID))==0xff) return;
	
	if(ObjID>=MAX_IMG_KEY_NUM)
	{
		Debug("%s:ObjID is error!\n\r",__FUNCTION__);
		return;
	}
	
	OS_EnterCritical();
	gRepImgSufx[ObjID]=Suffix;
	OS_ExitCritical();
}

//和Q_ChangeImgTchImg相对，读取当前的图标替换后缀值
//返回0表示是默认值
//否则返回替换的后缀
u8 Q_ReadImgTchImg(u8 ObjID)
{
	if((ObjID=FindImgTchIdx(ObjID))==0xff) return 0;
	
	if(ObjID>=MAX_IMG_KEY_NUM)
	{
		Debug("%s:ObjID is error!\n\r",__FUNCTION__);
		return 0;
	}

	return gRepImgSufx[ObjID];
}

//更改当前触摸域某个文字按键的显示文字
// 1.只对当前页面有效,转换页面或子页面后失效
//如果NewName=NULL，则恢复原始文字
void Q_ChangeCharTchName(u8 ObjID,u8 *NewName)
{
	OS_DeclareCritical();
	
	if((ObjID=FindCharTchIdx(ObjID))==0xff) return;
	
	if(ObjID>=MAX_CHAR_KEY_NUM)
	{
		Debug("%s:ObjID is error!\n\r",__FUNCTION__);
		return;
	}
	
	OS_EnterCritical();
	gRepKeyNameCon[ObjID]=NewName;
	OS_ExitCritical();
}

//和Q_ChangeCharTchName相对，读取当前的图标替换后缀值
//返回NULL表示是默认值或者key超出范围
u8 *Q_ReadCharTchName(u8 ObjID)
{
	if((ObjID=FindCharTchIdx(ObjID))==0xff) return NULL;
	
	if(ObjID>=MAX_CHAR_KEY_NUM)
	{
		Debug("%s:ObjID is error!\n\r",__FUNCTION__);
		return NULL;
	}
	
	return gRepKeyNameCon[ObjID];
}

//立刻呈现指定按键键值的图标或文字，对所有此键值的按键均有效
void Q_PresentTch(u8 ObjID,TCH_EVT Type)
{
	switch(Type)
	{
		case Tch_Normal:
			SameOidTchDisplay(ObjID,Input_TchNormal);
			break;
		case Tch_Press:		
			SameOidTchDisplay(ObjID,Input_TchPress);
			break;
		case Tch_Release:
			SameOidTchDisplay(ObjID,Input_TchRelease);
			break;
	}	
}

//用于设置新的动态图标按键
//Idx指定索引号，从1开始，不得大于page的DynImgTchNum属性值
bool Q_SetDynamicImgTch(u8 Idx,IMG_TCH_OBJ *pTchReg)
{
	IMG_TCH_OBJ **pDynImgTchCon=(void *)gCtrlObjPtrBuf; //动态图片按键集合

	if((Idx==0)||(Idx>gpCtrlObjNum->DynImgTchNum)) return FALSE;
	Idx--;

	pDynImgTchCon[Idx]=pTchReg;

	if(pTchReg!=NULL)
	{
		u8 Num=gpCtrlObjNum->ImgTchNum+gpCtrlObjNum->CharTchNum+Idx;
		gpTouchRegions[Num].x=pDynImgTchCon[Idx]->x;
		gpTouchRegions[Num].y=pDynImgTchCon[Idx]->y;
		gpTouchRegions[Num].w=pDynImgTchCon[Idx]->w;
		gpTouchRegions[Num].h=pDynImgTchCon[Idx]->h;
		gpTouchRegions[Num].ObjID=pDynImgTchCon[Idx]->ObjID;
		gpTouchRegions[Num].Type=COT_DynImg;
		gpTouchRegions[Num].Index=Idx;
		gpTouchRegions[Num].OptionsMask=(u8)(pDynImgTchCon[Idx]->OptionsMask&0xff);
		ImgTchDisplay(Idx,Input_TchNormal,TRUE);
	}
	else
	{
		MemSet(&gpTouchRegions[gpCtrlObjNum->ImgTchNum+gpCtrlObjNum->CharTchNum+Idx],0,sizeof(TOUCH_REGION));
	}

	return FALSE;
}

//用于设置新的动态文字按键
//Idx指定索引号，从1开始，不得大于page的DynCharTchNum属性值
bool Q_SetDynamicCharTch(u8 Idx,CHAR_TCH_OBJ *pTchReg)
{
	CHAR_TCH_OBJ **pDynCharTchCon=(void *)&gCtrlObjPtrBuf[gpCtrlObjNum->DynImgTchNum];//动态文字按键集合
		
	if((Idx==0)||(Idx>gpCtrlObjNum->DynCharTchNum)) return FALSE;
	Idx--;

	pDynCharTchCon[Idx]=pTchReg;

	if(pTchReg!=NULL)
	{
		u8 Num=gpCtrlObjNum->ImgTchNum+gpCtrlObjNum->CharTchNum+gpCtrlObjNum->DynImgTchNum+Idx;
		gpTouchRegions[Num].x=pDynCharTchCon[Idx]->x;
		gpTouchRegions[Num].y=pDynCharTchCon[Idx]->y;
		gpTouchRegions[Num].w=pDynCharTchCon[Idx]->w;
		gpTouchRegions[Num].h=pDynCharTchCon[Idx]->h;
		gpTouchRegions[Num].ObjID=pDynCharTchCon[Idx]->ObjID;
		gpTouchRegions[Num].Type=COT_DynChar;
		gpTouchRegions[Num].Index=Idx;
		gpTouchRegions[Num].OptionsMask=(u8)(pDynCharTchCon[Idx]->OptionsMask&0xff);
		CharTchDisplay(Idx,Input_TchNormal,TRUE);
	}
	else
	{
		MemSet(&gpTouchRegions[gpCtrlObjNum->ImgTchNum+gpCtrlObjNum->CharTchNum+gpCtrlObjNum->DynImgTchNum+Idx],0,sizeof(TOUCH_REGION));
	}
	
	return FALSE;
}

#ifdef QSYS_FRAME_FULL
//设置yes no选项，pYesNo指向的内存在调用完函数后不可注销
//一旦设置，当进入页面时，会用到此内存
//所以当页面还存在时，必须保证此内存存在
//Idx从1开始
bool Q_SetYesNo(u8 Idx,YES_NO_OBJ *pYesNo)
{
	YES_NO_OBJ **pYesNoCon=(void *)&gCtrlObjPtrBuf[gpCtrlObjNum->DynImgTchNum+gpCtrlObjNum->DynCharTchNum];

	if(Idx>gpCtrlObjNum->YesNoNum) return FALSE;
	Idx--;

	pYesNoCon[Idx]=pYesNo;

	if(pYesNo!=NULL)
	{
		GUI_REGION DrawRegion;
		u8 Num=gpCtrlObjNum->ImgTchNum+gpCtrlObjNum->CharTchNum
			+gpCtrlObjNum->DynImgTchNum+gpCtrlObjNum->DynCharTchNum+Idx;
		gpTouchRegions[Num].x=pYesNo->x;
		gpTouchRegions[Num].y=pYesNo->y;
		gpTouchRegions[Num].w=CO_YES_NO_W;
		gpTouchRegions[Num].h=CO_YES_NO_H;
		gpTouchRegions[Num].ObjID=pYesNo->ObjID;
		gpTouchRegions[Num].Type=COT_YesNo;
		gpTouchRegions[Num].Index=Idx;
		gpTouchRegions[Num].OptionsMask=(u8)(PrsMsk|RelMsk|ReVMsk);
		
		//draw
		DrawRegion.x=pYesNo->x;
		DrawRegion.y=pYesNo->y;
		DrawRegion.w=CO_YES_NO_W;
		DrawRegion.h=CO_YES_NO_H;
		DrawRegion.Color=CO_YES_NO_TRAN_COLOR;
		if(pYesNo->DefVal==FALSE)
			Gui_DrawImgArray(gCtrlObj_Off,&DrawRegion);
		else 
			Gui_DrawImgArray(gCtrlObj_On,&DrawRegion);
	}
	else
	{//清空触摸区域记录
		MemSet(&gpTouchRegions[gpCtrlObjNum->ImgTchNum+gpCtrlObjNum->CharTchNum
			+gpCtrlObjNum->DynImgTchNum+gpCtrlObjNum->DynCharTchNum+Idx],0,sizeof(TOUCH_REGION));
	}
	
	return FALSE;
}

//设置num box选项，pNumBox指向的内存在调用完函数后不可注销
//一旦设置，当进入页面时，会用到此内存
//所以当页面还存在时，必须保证此内存存在
//Idx从1开始
bool Q_SetNumCtrlObj(u8 Idx,NUM_BOX_OBJ *pNumBox)
{
	NUM_BOX_OBJ **pNumBoxCon=(void *)&gCtrlObjPtrBuf[gpCtrlObjNum->DynImgTchNum+gpCtrlObjNum->DynCharTchNum
																								+gpCtrlObjNum->YesNoNum];
	if(Idx>gpCtrlObjNum->NumBoxNum) return FALSE;
	Idx--;
	
	pNumBoxCon[Idx]=pNumBox;

	if(pNumBox!=NULL)
	{
		GUI_REGION DrawRegion;
		u8 Num=gpCtrlObjNum->ImgTchNum+gpCtrlObjNum->CharTchNum
			+gpCtrlObjNum->DynImgTchNum+gpCtrlObjNum->DynCharTchNum
			+gpCtrlObjNum->YesNoNum+Idx;
		gpTouchRegions[Num].x=pNumBox->x;
		gpTouchRegions[Num].y=pNumBox->y;
		gpTouchRegions[Num].w=pNumBox->w;
		gpTouchRegions[Num].h=CO_NUM_H;
		gpTouchRegions[Num].ObjID=pNumBox->ObjID;
		gpTouchRegions[Num].Type=COT_NumBox;
		gpTouchRegions[Num].Index=Idx;
		gpTouchRegions[Num].OptionsMask=(u8)(PrsMsk|RelMsk|ReVMsk);
		
		{//draw
			u8 NumStr[32];
			DrawRegion.x=pNumBox->x+CO_NUM_ARROW_W-CO_NUM_FRAME_W;
			DrawRegion.y=pNumBox->y;
			DrawRegion.w=CO_NUM_FRAME_W;
			DrawRegion.h=CO_NUM_H;
			DrawRegion.Color=CO_NUM_TRAN_COLOR;
			Gui_DrawImgArray(gCtrlObj_NumLeft,&DrawRegion);//左边框

			DrawRegion.x=pNumBox->x+CO_NUM_ARROW_W;
			DrawRegion.y=pNumBox->y;
			DrawRegion.w=CO_NUM_MIDDLE_W;
			DrawRegion.h=CO_NUM_H;
			DrawRegion.Color=CO_NUM_TRAN_COLOR;
			Gui_FillImgArray_H(gCtrlObj_NumMiddle,pNumBox->w-(CO_NUM_ARROW_W<<1),&DrawRegion);	

			DrawRegion.x=pNumBox->x+pNumBox->w-CO_NUM_ARROW_W;
			DrawRegion.y=pNumBox->y;
			DrawRegion.w=CO_NUM_FRAME_W;
			DrawRegion.h=CO_NUM_H;
			DrawRegion.Color=CO_NUM_TRAN_COLOR;
			Gui_DrawImgArray(gCtrlObj_NumRight,&DrawRegion);//右边框
			
			sprintf((void *)NumStr,"%d",pNumBox->Value);
			DrawRegion.x=pNumBox->x+((pNumBox->w-strlen((void *)NumStr)*CO_NUM_FONT_W)>>1);
			DrawRegion.y=pNumBox->y+3;
			DrawRegion.w=pNumBox->w-(CO_NUM_ARROW_W<<1);
			DrawRegion.h=CO_NUM_H;
			DrawRegion.Color=CO_NUM_FONT_COLOR;
			DrawRegion.Space=CO_NUM_FONT_SPACE;
			Gui_DrawFont(CO_NUM_FONT_STYLE,NumStr,&DrawRegion);
		}
	}
	else
	{//清空触摸区域记录
		MemSet(&gpTouchRegions[gpCtrlObjNum->ImgTchNum+gpCtrlObjNum->CharTchNum
			+gpCtrlObjNum->DynImgTchNum+gpCtrlObjNum->DynCharTchNum
			+gpCtrlObjNum->YesNoNum+Idx],0,sizeof(TOUCH_REGION));
	}
	
	return FALSE;
}


#endif

#endif



