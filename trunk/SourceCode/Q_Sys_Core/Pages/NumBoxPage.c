/**************** Q-SYS *******************
 * PageName : NumBoxPage
 * Author : YourName
 * Version : 
 * Base Q-Sys Version :
 * Description :
 * WebSite : Www.Q-ShareWe.Com
 ***************** Q-SYS *******************/
 
#include "User.h"
#include "Theme.h"
#include "NumBoxPage.h"
#include "CtrlObjImg.h"

//函数声明
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam);
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam);
static TCH_MSG TouchEventHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo);

//-----------------------本页系统变量及声明-----------------------
//定义页面按键需要用到的枚举，类似于有序唯一的宏定义
typedef enum
{
	ExtiKeyDown=EXTI_KEY_VALUE_START,//系统默认将外部中断按键发送到第一个键值
	ExtiKeyUp,
	ExtiKeyEnter,
	
	//后面的硬件版本将支持更多外部中断按键或者无线键盘，
	//所以触摸键值从USER_KEY_VALUE_START开始，将前面的键值都留下来
	BackKV=USER_KEY_VALUE_START,
	LeftArrowKV,
	DotKV,
	RightArrowKV,
	DoneKV,

	Key1KV,
	Key2KV,
	Key3KV,
	Key4KV,
	Key5KV,
	Key6KV,
	Key7KV,
	Key8KV,
	Key9KV,
	Key0KV,
	KeyMinusKv,
	KeyDotKV,
	KeyCancleKV,
	KeyOkKV,
	KeyPreKV,
	KeyNextKV,

	HomeKV,
	MessageKV,
	MusicKV,
	PepoleKV,
}NumBoxPage_KEY_NAME;

//定义页面或应用的触摸区域集，相当于定义按键
//支持的最大触摸区域个数为MAX_TOUCH_REGION_NUM
//系统显示和触摸的所有坐标系均以屏幕左上角为原点(x 0,y 0)，右下角为(x 320,y 240)
static const IMG_TCH_OBJ ImgTchRegCon[]={
	//KeyName,ObjID,OptionMask,Tch_x,Tch_y,Tch_w,Tch_h,Img_x,Img_y,BmpPathPrefix,NormalSuf,PressSuf,ReleaseSuf,TransColor},
	//液晶屏下面非显示区域的四个键
	{"",HomeKV,RelMsk,0,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MessageKV,RelMsk,60,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MusicKV,RelMsk,120,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",PepoleKV,RelMsk,180,320,60,30,0,0,"",FatColor(NO_TRANS)},
};

static const CHAR_TCH_OBJ NumTchRegCon[]={
	//KeyName,ObjID,OptionMask,Tch_x,Tch_y,Tch_w,Tch_h,
		//Char_x,Char_y,MarginXY,SpaceXY,NormalColor,NormalBG,PressColor,PressBG,ReleaseColor,ReleaseBG},
	{"1",Key1KV,B14Msk|RelMsk,4,4,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"2",Key2KV,B14Msk|RelMsk,28,4,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"3",Key3KV,B14Msk|RelMsk,52,4,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"4",Key4KV,B14Msk|RelMsk,76,4,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"5",Key5KV,B14Msk|RelMsk,100,4,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"-",KeyMinusKv,B14Msk|RelMsk,124,4,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"X",KeyCancleKV,B14Msk|RelMsk,148,4,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},

	{"6",Key6KV,B14Msk|RelMsk,4,28,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"7",Key7KV,B14Msk|RelMsk,28,28,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"8",Key8KV,B14Msk|RelMsk,52,28,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"9",Key9KV,B14Msk|RelMsk,76,28,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"0",Key0KV,B14Msk|RelMsk,100,28,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{".",KeyDotKV,B14Msk|RelMsk,124,28,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"O",KeyOkKV,B14Msk|RelMsk,148,28,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
};

static const CHAR_TCH_OBJ ArrowTchRegCon[]={
	//KeyName,ObjID,OptionMask,Tch_x,Tch_y,Tch_w,Tch_h,
		//Char_x,Char_y,MarginXY,SpaceXY,NormalColor,NormalBG,PressColor,PressBG,ReleaseColor,ReleaseBG},
	{"<",Key1KV,B14Msk|RelMsk,4,4,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{">",Key2KV,B14Msk|RelMsk,28,4,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
};

//定义页面或者应用的属性集
const PAGE_ATTRIBUTE NumBoxPage={
	"NumBoxPage",
	"Author",
	"Page Description",
	NORMAL_PAGE,
	0,//

	{
		sizeof(ImgTchRegCon)/sizeof(IMG_TCH_OBJ), //size of touch region array
		0//sizeof(NumTchRegCon)/sizeof(CHAR_TCH_OBJ), //size of touch region array,
	},
	ImgTchRegCon, //touch region array
	NumTchRegCon,
	
	SystemEventHandler,
	PeripheralsHandler,
	Bit(Perip_KeyPress)|Bit(Perip_KeyRelease)|Bit(Perip_UartInput),
	TouchEventHandler,
};

//-----------------------本页自定义变量声明-----------------------
typedef struct{
	NUM_BOX_TYPE HandlerType;
	u8 ObjHandler[32];// 成员个数必须大于num obj存储需要的最大字节数
	u16 LeftDispBuf[CO_NUM_BOX_ARROW_W*CO_NUM_BOX_H];
	u16 RightDispBuf[CO_NUM_BOX_ARROW_W*CO_NUM_BOX_H];
	
}NUM_BOX_PAGE_VARS;
static NUM_BOX_PAGE_VARS *gpNbpVars;//只需要定义一个指针，减少全局变量的使用

//-----------------------本页自定义函数-----------------------

//-----------------------本页系统函数----------------------

//发生某些事件时，会触发的函数
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam)
{
	NUM_BOX_OBJ *pNumBoxObj;
	NUM_LIST_OBJ *pNumListObj;
	NUM_ENUM_OBJ *pNumEnumObj;
	GUI_REGION DrawRegion;
	
	switch(SysEvent)
	{
		case Sys_PreGotoPage:
			if((pNumBoxObj->Type == NBT_NumBox) &&(Q_DB_GetStatus(Status_FsInitFinish,NULL)==FALSE)) //文件系统没有挂载
			{
				Debug("File system not mount!!!\n\r");
				return SM_State_Faile|SM_NoGoto;
			}			
			return SM_NoPageClean;
		case Sys_PageInit:		//系统每次打开这个页面，会处理这个事件		
			pNumBoxObj=pSysParam;
			pNumListObj=pSysParam;
			pNumEnumObj=pSysParam;
			gpNbpVars=Q_PageMallco(sizeof(NUM_BOX_PAGE_VARS));
			gpNbpVars->HandlerType=pNumBoxObj->Type;
			if(pNumBoxObj->Type == NBT_NumBox)
			{
				PrtScreenToBin("/NumBoxPS.buf",0,0,176,56);
				DrawRegion.x=0;
				DrawRegion.y=0;
				DrawRegion.w=176;
				DrawRegion.h=56;
				DrawRegion.Color=FatColor(0xffffff);
				Gui_FillBlock(&DrawRegion);

				MemCpy(gpNbpVars->ObjHandler,pNumBoxObj,sizeof(NUM_BOX_OBJ));
			}
			else if(pNumListObj->Type == NBT_NumList)
			{
				DrawRegion.x=pNumListObj->x;
				DrawRegion.y=pNumListObj->y;
				DrawRegion.w=CO_NUM_BOX_ARROW_W;
				DrawRegion.h=CO_NUM_BOX_H;
				Gui_ReadRegion16Bit(gpNbpVars->LeftDispBuf,&DrawRegion);

				DrawRegion.x=pNumListObj->x+pNumListObj->w-CO_NUM_BOX_ARROW_W;
				DrawRegion.y=pNumListObj->y;
				DrawRegion.w=CO_NUM_BOX_ARROW_W;
				DrawRegion.h=CO_NUM_BOX_H;
				Gui_ReadRegion16Bit(gpNbpVars->RightDispBuf,&DrawRegion);

				DrawRegion.x=pNumListObj->x;
				DrawRegion.y=pNumListObj->y;
				DrawRegion.w=CO_NUM_BOX_ARROW_W;
				DrawRegion.h=CO_NUM_BOX_H;
				DrawRegion.Color=CO_NUM_BOX_TRAN_COLOR;
				Gui_DrawImgArray(gCtrlObj_NumBoxLeftArrow,&DrawRegion);

				DrawRegion.x=pNumListObj->x+pNumListObj->w-CO_NUM_BOX_ARROW_W;
				DrawRegion.y=pNumListObj->y;
				DrawRegion.w=CO_NUM_BOX_ARROW_W;
				DrawRegion.h=CO_NUM_BOX_H;
				DrawRegion.Color=CO_NUM_BOX_TRAN_COLOR;
				Gui_DrawImgArray(gCtrlObj_NumBoxRightArrow,&DrawRegion);

				MemCpy(gpNbpVars->ObjHandler,pNumListObj,sizeof(NUM_LIST_OBJ));
			}
			else if(pNumEnumObj->Type == NBT_NumEnum)
			{

			}
			else
			{
			}
			break;
		case Sys_TouchSetOk:
		case Sys_TouchSetOk_SR:

			break;
		case Sys_PageClean:
			if(gpNbpVars!=NULL)
			{
				if(gpNbpVars->HandlerType==NBT_NumBox)
				{
					pNumBoxObj=(void *)gpNbpVars->ObjHandler;
					
					DrawRegion.x=0;
					DrawRegion.y=0;
					DrawRegion.w=176;
					DrawRegion.h=56;
					DrawRegion.Color=FatColor(NO_TRANS);
					Gui_DrawImgBin("/NumBoxPS.buf",&DrawRegion);
				}
				else if(gpNbpVars->HandlerType==NBT_NumList)
				{
					pNumListObj=(void *)gpNbpVars->ObjHandler;
					
					DrawRegion.x=pNumListObj->x;
					DrawRegion.y=pNumListObj->y;
					DrawRegion.w=CO_NUM_BOX_ARROW_W;
					DrawRegion.h=CO_NUM_BOX_H;
					DrawRegion.Color=CO_NUM_BOX_TRAN_COLOR;
					Gui_DrawImgArray((void *)gpNbpVars->LeftDispBuf,&DrawRegion);

					DrawRegion.x=pNumListObj->x+pNumListObj->w-CO_NUM_BOX_ARROW_W;
					DrawRegion.y=pNumListObj->y;
					DrawRegion.w=CO_NUM_BOX_ARROW_W;
					DrawRegion.h=CO_NUM_BOX_H;
					DrawRegion.Color=CO_NUM_BOX_TRAN_COLOR;
					Gui_DrawImgArray((void *)gpNbpVars->RightDispBuf,&DrawRegion);
				}
				else if(gpNbpVars->HandlerType==NBT_NumEnum)
				{

				}
				else
				{
				}
			}
			return SM_NoPageInit |SM_NoTouchInit;
		default:
			//需要响应的事件未定义
			Debug("%s SystemEventHandler:This System Event Handler case unfinish! SysEvent:%d\n\r",Q_GetCurrPageName(),SysEvent);
			//while(1);
	}
	
	return 0;
}

static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam)
{
	switch(PeripEvent)
	{
		case Perip_KeyPress:
			if(IntParam==ExtiKeyEnter)
			{
			}
			else if(IntParam==ExtiKeyUp)
			{
			}
			else if(IntParam==ExtiKeyDown)
			{
			}
			break;
		case Perip_KeyRelease:
			if(IntParam==ExtiKeyEnter)
			{PrtScreen();
			}
			else if(IntParam==ExtiKeyUp)
			{
			}
			else if(IntParam==ExtiKeyDown)
			{
			}
			break;
		case Perip_UartInput:
			if((IntParam>>16)==1)//串口1
			{
				Q_Sh_CmdHandler(IntParam&0xffff,pParam);
			}
			break;
	}

	return 0;
}

//当使用者按下本页TouchRegionSet里定义的按键时，会触发这个函数里的对应事件
static TCH_MSG TouchEventHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo)
{		
	//GUI_REGION DrawRegion;
	
	switch(Key)
	{
		case HomeKV:
			Q_GotoPage(SubPageReturn,"",0,NULL);//返回前一个页面
			break;
		case MessageKV:
		case MusicKV:
		case PepoleKV:
			break;
		default:
			//需要响应的事件未定义
			Debug("%s TouchEventHandler:This Touch Event Handler case unfinish! Key:%d\n\r",Q_GetCurrPageName(),Key);
			///while(1);
	}
	
	return 0;
}



