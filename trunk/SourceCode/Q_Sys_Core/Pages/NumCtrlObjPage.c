/**************** Q-SYS *******************
 * PageName : NumCtrlObjPage
 * Author : YourName
 * Version : 
 * Base Q-Sys Version :
 * Description :
 * WebSite : Www.Q-ShareWe.Com
 ***************** Q-SYS *******************/
 
#include "User.h"
#include "Theme.h"
#include "NumCtrlObjPage.h"
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
	RightArrowKV,
	NumBoxKV,
	
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
}NumCtrlObjPage_KEY_NAME;

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
const PAGE_ATTRIBUTE NumCtrlObjPage={
	"NumCtrlObjPage",
	"Author",
	"Page Description",
	POP_PAGE,
	0,//

	{
		sizeof(ImgTchRegCon)/sizeof(IMG_TCH_OBJ), //size of touch region array
		0,//sizeof(NumTchRegCon)/sizeof(CHAR_TCH_OBJ), //size of touch region array,
		3,
	},
	ImgTchRegCon, //touch region array
	NumTchRegCon,
	
	SystemEventHandler,
	PeripheralsHandler,
	Bit(Perip_KeyPress)|Bit(Perip_KeyRelease)|Bit(Perip_UartInput),
	TouchEventHandler,
};

//-----------------------本页自定义变量声明-----------------------
//功能选项
#define TOUCH_REGION_MARGIN 4//margin可增加可点击区域范围

//动态按键模板
const IMG_TCH_OBJ ArrowTchTmp={"",0,PrsMsk|RelMsk|ReVMsk,0,0,CO_NUM_ARROW_W,CO_NUM_H,0,0,"",FatColor(NO_TRANS)};
const IMG_TCH_OBJ NumTchTmp={"",0,RelMsk,0,0,0,CO_NUM_H,0,0,"",FatColor(NO_TRANS)};

typedef struct{
	NUM_BOX_TYPE HandlerType;//控件类型
	void *ObjHandler;//控件实体指针
	s32 DefValue;//控件默认值
	u16 LeftDispBuf[CO_NUM_ARROW_W*CO_NUM_H];//控件左箭头缓存
	u16 RightDispBuf[CO_NUM_ARROW_W*CO_NUM_H];//控件右箭头缓存
	IMG_TCH_OBJ LeftArrowTch;//左箭头动态按键实体
	IMG_TCH_OBJ RightArrowTch;//右箭头动态按键实体
	IMG_TCH_OBJ NumTch;//中间数字框动态按键实体
	GUI_REGION NumKeySaveReg;//小键盘范围
}NUM_CTRL_OBJ_PAGE_VARS;
static NUM_CTRL_OBJ_PAGE_VARS *gpNbpVars;//只需要定义一个指针，减少全局变量的使用

//-----------------------本页自定义函数-----------------------
typedef enum{
	DNA_Null=0,//维持原样
	DNA_Normal,//普通
	DNA_HiLight,//高亮
	DNA_NormalArrow,//普通箭头
	DNA_HiLightArrow//高亮箭头
}DRAW_NL_ACT;
//根据要求绘制Num控件
static void DrawNumCtrlObj(NUM_BOX_OBJ *pNumBoxObj,DRAW_NL_ACT Left,DRAW_NL_ACT Middle,DRAW_NL_ACT Right)
{
	NUM_LIST_OBJ *pNumListObj;
	NUM_ENUM_OBJ *pNumEnumObj;
	GUI_REGION DrawRegion;
	u8 NumStr[32];

	pNumListObj=(void *)pNumBoxObj;
	pNumEnumObj=(void *)pNumBoxObj;
	
	if(pNumBoxObj->Type==NBT_NumBox)
	{
	}
	else 	if((pNumListObj->Type==NBT_NumList)||(pNumEnumObj->Type==NBT_NumEnum))
	{
		if(Left != DNA_Null)//左边部分
		{
			DrawRegion.y=pNumListObj->y;
			DrawRegion.h=CO_NUM_H;
			DrawRegion.Color=CO_NUM_TRAN_COLOR;
		}
		if(Left == DNA_Normal)
		{
			DrawRegion.x=pNumListObj->x+CO_NUM_ARROW_W-CO_NUM_FRAME_W;
			DrawRegion.w=CO_NUM_FRAME_W;
			Gui_DrawImgArray(gCtrlObj_NumLeft,&DrawRegion);//左边框
		}
		else if(Left == DNA_HiLight)
		{
			DrawRegion.x=pNumListObj->x+CO_NUM_ARROW_W-CO_NUM_FRAME_W;
			DrawRegion.w=CO_NUM_FRAME_W;
			Gui_DrawImgArray(gCtrlObj_NumLeftH,&DrawRegion);//左边框
		}
		else if(Left == DNA_NormalArrow)
		{
			DrawRegion.x=pNumListObj->x;
			DrawRegion.w=CO_NUM_ARROW_W;
			DrawRegion.Color=CO_NUM_TRAN_COLOR;
			Gui_DrawImgArray(gCtrlObj_NumLeftArrow,&DrawRegion);//左箭头绘画
		}
		else if(Left == DNA_HiLightArrow)
		{
			DrawRegion.x=pNumListObj->x;
			DrawRegion.w=CO_NUM_ARROW_W;
			Gui_DrawImgArray(gCtrlObj_NumLeftArrowH,&DrawRegion);//左箭头绘画
		}

		if(Middle != DNA_Null)//中间框部分
		{
			DrawRegion.x=pNumListObj->x+CO_NUM_ARROW_W;
			DrawRegion.y=pNumListObj->y;
			DrawRegion.w=CO_NUM_MIDDLE_W;
			DrawRegion.h=CO_NUM_H;
			DrawRegion.Color=CO_NUM_TRAN_COLOR;
		}
		if(Middle == DNA_Normal)
			Gui_FillImgArray_H(gCtrlObj_NumMiddle,pNumListObj->w-(CO_NUM_ARROW_W<<1),&DrawRegion);	//高亮框
		else if(Middle == DNA_HiLight)
			Gui_FillImgArray_H(gCtrlObj_NumMiddleH,pNumListObj->w-(CO_NUM_ARROW_W<<1),&DrawRegion);	//框

		if(Middle != DNA_Null)//中间数字部分
		{
			sprintf((void *)NumStr,"%d",pNumListObj->Value);
			DrawRegion.x=pNumListObj->x+((pNumListObj->w-strlen((void *)NumStr)*CO_NUM_FONT_W)>>1);
			DrawRegion.y=pNumListObj->y+3;
			DrawRegion.w=pNumListObj->w-(CO_NUM_ARROW_W<<1);
			DrawRegion.h=CO_NUM_H;
			DrawRegion.Space=CO_NUM_FONT_SPACE;		
		}
		if(Middle == DNA_Normal)
		{
			DrawRegion.Color=CO_NUM_FONT_COLOR;
			Gui_DrawFont(CO_NUM_FONT_STYLE,NumStr,&DrawRegion);//数字
		}
		else if(Middle == DNA_HiLight)
		{
			DrawRegion.Color=CO_NUM_FONT_COLOR_H;
			Gui_DrawFont(CO_NUM_FONT_STYLE,NumStr,&DrawRegion);//数字
		}
		
		if(Right != DNA_Null)//右边部分
		{
			DrawRegion.x=pNumListObj->x+pNumListObj->w-CO_NUM_ARROW_W;
			DrawRegion.y=pNumListObj->y;
			DrawRegion.h=CO_NUM_H;
			DrawRegion.Color=CO_NUM_TRAN_COLOR;
		}
		if(Right == DNA_Normal)
		{
			DrawRegion.w=CO_NUM_FRAME_W;
			Gui_DrawImgArray(gCtrlObj_NumRight,&DrawRegion);//右边框
		}
		else if(Right == DNA_HiLight)
		{
			DrawRegion.w=CO_NUM_FRAME_W;
			Gui_DrawImgArray(gCtrlObj_NumRightH,&DrawRegion);//右边框
		}
		else if(Right == DNA_NormalArrow)
		{
			DrawRegion.w=CO_NUM_ARROW_W;
			Gui_DrawImgArray(gCtrlObj_NumRightArrow,&DrawRegion);//右箭头绘画
		}
		else if(Right == DNA_HiLightArrow)
		{
			DrawRegion.w=CO_NUM_ARROW_W;
			Gui_DrawImgArray(gCtrlObj_NumRightArrowH,&DrawRegion);//右箭头绘画			
		}
	}
}
//-----------------------本页系统函数----------------------

//发生某些事件时，会触发的函数
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam)
{
	NUM_BOX_OBJ *pNumBoxObj=pSysParam;
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
			return SM_State_OK;
		case Sys_PageInit:		//系统每次打开这个页面，会处理这个事件		
			pNumListObj=pSysParam;
			pNumEnumObj=pSysParam;
			gpNbpVars=Q_PageMallco(sizeof(NUM_CTRL_OBJ_PAGE_VARS));
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

				gpNbpVars->ObjHandler=pNumBoxObj;
				gpNbpVars->DefValue=pNumBoxObj->Value;
			}
			else if((pNumListObj->Type == NBT_NumList)||(pNumEnumObj->Type == NBT_NumEnum))
			{
				DrawRegion.x=pNumListObj->x;
				DrawRegion.y=pNumListObj->y;
				DrawRegion.w=CO_NUM_ARROW_W;
				DrawRegion.h=CO_NUM_H;
				Gui_ReadRegion16Bit(gpNbpVars->LeftDispBuf,&DrawRegion);//保存旧图

				DrawRegion.x=pNumListObj->x+pNumListObj->w-CO_NUM_ARROW_W;
				DrawRegion.y=pNumListObj->y;
				DrawRegion.w=CO_NUM_ARROW_W;
				DrawRegion.h=CO_NUM_H;
				Gui_ReadRegion16Bit(gpNbpVars->RightDispBuf,&DrawRegion);//保存旧图

				DrawNumCtrlObj((void *)pNumListObj,DNA_NormalArrow,DNA_HiLight,DNA_NormalArrow);//画新图

				MemCpy(&gpNbpVars->LeftArrowTch,&ArrowTchTmp,sizeof(IMG_TCH_OBJ));//左箭头
				gpNbpVars->LeftArrowTch.ObjID=LeftArrowKV;
				gpNbpVars->LeftArrowTch.x=pNumListObj->x-TOUCH_REGION_MARGIN;
				gpNbpVars->LeftArrowTch.y=pNumListObj->y-TOUCH_REGION_MARGIN;
				gpNbpVars->LeftArrowTch.w+=(TOUCH_REGION_MARGIN<<1);
				gpNbpVars->LeftArrowTch.h+=(TOUCH_REGION_MARGIN<<1);
				Q_SetDynamicImgTch(1,&gpNbpVars->LeftArrowTch);//建立动态按键区域

				MemCpy(&gpNbpVars->RightArrowTch,&ArrowTchTmp,sizeof(IMG_TCH_OBJ));//右箭头
				gpNbpVars->RightArrowTch.ObjID=RightArrowKV;
				gpNbpVars->RightArrowTch.x=pNumListObj->x+pNumListObj->w-CO_NUM_ARROW_W-TOUCH_REGION_MARGIN;
				gpNbpVars->RightArrowTch.y=pNumListObj->y-TOUCH_REGION_MARGIN;
				gpNbpVars->RightArrowTch.w+=(TOUCH_REGION_MARGIN<<1);
				gpNbpVars->RightArrowTch.h+=(TOUCH_REGION_MARGIN<<1);
				Q_SetDynamicImgTch(2,&gpNbpVars->RightArrowTch);//建立动态按键区域

				MemCpy(&gpNbpVars->NumTch,&NumTchTmp,sizeof(IMG_TCH_OBJ));//数字框模板
				gpNbpVars->NumTch.ObjID=NumBoxKV;
				gpNbpVars->NumTch.x=pNumListObj->x+CO_NUM_ARROW_W+TOUCH_REGION_MARGIN;
				gpNbpVars->NumTch.y=DrawRegion.y;
				gpNbpVars->NumTch.w=pNumListObj->w-(CO_NUM_ARROW_W<<1)-(TOUCH_REGION_MARGIN<<1);
				Q_SetDynamicImgTch(3,&gpNbpVars->NumTch);//建立动态按键区域

				gpNbpVars->ObjHandler=pNumListObj;//记录控件指针			
				gpNbpVars->DefValue=pNumListObj->Value;
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
				else if((gpNbpVars->HandlerType==NBT_NumList)||(gpNbpVars->HandlerType==NBT_NumEnum))
				{
					pNumListObj=(void *)gpNbpVars->ObjHandler;
					
					DrawRegion.x=pNumListObj->x;
					DrawRegion.y=pNumListObj->y;
					DrawRegion.w=CO_NUM_ARROW_W;
					DrawRegion.h=CO_NUM_H;
					DrawRegion.Color=CO_NUM_TRAN_COLOR;
					Gui_DrawImgArray((void *)gpNbpVars->LeftDispBuf,&DrawRegion);//还原

					DrawRegion.x=pNumListObj->x+pNumListObj->w-CO_NUM_ARROW_W;
					DrawRegion.y=pNumListObj->y;
					DrawRegion.w=CO_NUM_ARROW_W;
					DrawRegion.h=CO_NUM_H;
					DrawRegion.Color=CO_NUM_TRAN_COLOR;
					Gui_DrawImgArray((void *)gpNbpVars->RightDispBuf,&DrawRegion);//还原

					if(gpNbpVars->DefValue == pNumListObj->Value)
						DrawNumCtrlObj((void *)pNumListObj,DNA_Normal,DNA_Normal,DNA_Normal);//未改变原有值
					else
						DrawNumCtrlObj((void *)pNumListObj,DNA_HiLight,DNA_HiLight,DNA_HiLight);//改变了原有值
				}
				else
				{
				}

				Q_PageFree(gpNbpVars);
			}
			return SM_State_OK;
		default:
			//需要响应的事件未定义
			Debug("%s SystemEventHandler:This System Event Handler case unfinish! SysEvent:%d\n\r",Q_GetCurrPageName(),SysEvent);
			//while(1);
	}
	
	return SM_State_OK;
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
		case LeftArrowKV:
			if(InEvent == Tch_Press)
			{
				NUM_LIST_OBJ *pNumList=(void *)gpNbpVars->ObjHandler;
				DrawNumCtrlObj((void *)pNumList,DNA_HiLightArrow,DNA_Null,DNA_Null);//画新图
			}
			else
			{
				NUM_LIST_OBJ *pNumList=(void *)gpNbpVars->ObjHandler;
				NUM_ENUM_OBJ *pNumEnum=(void *)gpNbpVars->ObjHandler;
				if(InEvent == Tch_Release)
				{
					if(pNumList->Type == NBT_NumList)
					{
						if((pNumList->Value-pNumList->Step)>=pNumList->Min)
							pNumList->Value-=pNumList->Step;
						else pNumList->Value=pNumList->Max;
					}
					else if(pNumEnum->Type == NBT_NumEnum)
					{
						if((pNumEnum->Idx-1)>=0) pNumEnum->Idx--;
						else pNumEnum->Idx=(pNumEnum->Total-1);
						pNumEnum->Value=pNumEnum->pEnumList[pNumEnum->Idx];			
					}
				}
				DrawNumCtrlObj((void *)pNumList,DNA_NormalArrow,DNA_HiLight,DNA_Null);//画新图
			}
			break;
		case RightArrowKV:
			if(InEvent == Tch_Press)
			{
				NUM_LIST_OBJ *pNumList=(void *)gpNbpVars->ObjHandler;
				DrawNumCtrlObj((void *)pNumList,DNA_Null,DNA_Null,DNA_HiLightArrow);//画新图
			}
			else
			{					
				NUM_LIST_OBJ *pNumList=(void *)gpNbpVars->ObjHandler;
				NUM_ENUM_OBJ *pNumEnum=(void *)gpNbpVars->ObjHandler;
				if(InEvent == Tch_Release)
				{
					if(pNumList->Type == NBT_NumList)
					{
						if((pNumList->Value+pNumList->Step)<=pNumList->Max)
							pNumList->Value+=pNumList->Step;
						else pNumList->Value=pNumList->Min;
					}
					else if(pNumEnum->Type == NBT_NumEnum)
					{
						if((pNumEnum->Idx+1)<pNumEnum->Num) pNumEnum->Idx++;
						else pNumEnum->Idx=0;
						pNumEnum->Value=pNumEnum->pEnumList[pNumEnum->Idx];		
					}
				}
				DrawNumCtrlObj((void *)pNumList,DNA_Null,DNA_HiLight,DNA_NormalArrow);//画新图
			}
			break;
		case NumBoxKV:
			Q_GotoPage(SubPageReturn,"",0,NULL);//返回前一个页面
			break;
			
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



