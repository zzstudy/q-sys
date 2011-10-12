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
	LeftArrowKV=USER_KEY_VALUE_START,
	RightArrowKV,
	NumFrameKV,
	
	Key0KV,
	Key1KV,
	Key2KV,
	Key3KV,
	Key4KV,
	Key5KV,
	Key6KV,
	Key7KV,
	Key8KV,
	Key9KV,
	KeyMinusKv,//+-
	KeyDotKV,//.
	KeyCancleKV,//X
	KeyOkKV,//O
}NumCtrlObjPage_KEY_NAME;

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

//定义页面或者应用的属性集
const PAGE_ATTRIBUTE NumCtrlObjPage={
	"NumCtrlObjPage",
	"Author",
	"Page Description",
	POP_PAGE,
	0,//

	{
		0,//size of touch region array
		0,//size of touch region array,
		3,
		sizeof(NumTchRegCon)/sizeof(CHAR_TCH_OBJ),//小键盘上的动态按键个数
	},
	NULL, 
	NULL,
	
	SystemEventHandler,
	PeripheralsHandler,
	Bit(Perip_KeyPress)|Bit(Perip_KeyRelease)|Bit(Perip_UartInput),
	TouchEventHandler,
};

//-----------------------本页自定义变量声明-----------------------
//功能选项
#define TOUCH_REGION_MARGIN 4//margin可增加可点击区域范围
#define KEY_BOARD_BG_COLOR FatColor(0xffffff)
//动态按键模板
const IMG_TCH_OBJ ArrowTchTmp={"",0,PrsMsk|RelMsk|ReVMsk,0,0,CO_NUM_ARROW_W,CO_NUM_H,0,0,"",FatColor(NO_TRANS)};
const IMG_TCH_OBJ NumTchTmp={"",0,RelMsk,0,0,0,CO_NUM_H,0,0,"",FatColor(NO_TRANS)};
const GUI_REGION KeyBoardRegionTmp={0,0,176,56,0,FatColor(NO_TRANS)};

typedef struct{
	NUM_CTRL_OBJ_TYPE HandlerType;//控件类型
	void *ObjHandler;//控件实体指针
	s32 DefValue;//控件默认值
	u16 LeftDispBuf[CO_NUM_ARROW_W*CO_NUM_H];//控件左箭头缓存
	u16 RightDispBuf[CO_NUM_ARROW_W*CO_NUM_H];//控件右箭头缓存
	IMG_TCH_OBJ LeftArrowTch;//左箭头动态按键实体
	IMG_TCH_OBJ RightArrowTch;//右箭头动态按键实体
	IMG_TCH_OBJ NumTch;//中间数字框动态按键实体
	CHAR_TCH_OBJ KeyBoard[sizeof(NumTchRegCon)/sizeof(CHAR_TCH_OBJ)];//小键盘实体
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
//根据要求绘制Num控件的框体和左右箭头及数字
static void DrawNumCtrlObj(NUM_BOX_OBJ *pNumBoxObj,DRAW_NL_ACT Left,DRAW_NL_ACT Middle,DRAW_NL_ACT Right)
{
	GUI_REGION DrawRegion;
	u8 NumStr[32];
	
	if((pNumBoxObj->Type==NCOT_NumList)||(pNumBoxObj->Type==NCOT_NumEnum)||(pNumBoxObj->Type==NCOT_NumBox))
	{
		if(Left != DNA_Null)//左边部分
		{
			DrawRegion.y=pNumBoxObj->y;
			DrawRegion.h=CO_NUM_H;
			DrawRegion.Color=CO_NUM_TRAN_COLOR;
		}
		if(Left == DNA_Normal)
		{
			DrawRegion.x=pNumBoxObj->x+CO_NUM_ARROW_W-CO_NUM_FRAME_W;
			DrawRegion.w=CO_NUM_FRAME_W;
			Gui_DrawImgArray(gCtrlObj_NumLeft,&DrawRegion);//左边框
		}
		else if(Left == DNA_HiLight)
		{
			DrawRegion.x=pNumBoxObj->x+CO_NUM_ARROW_W-CO_NUM_FRAME_W;
			DrawRegion.w=CO_NUM_FRAME_W;
			Gui_DrawImgArray(gCtrlObj_NumLeftH,&DrawRegion);//左边框
		}
		else if(Left == DNA_NormalArrow)
		{
			DrawRegion.x=pNumBoxObj->x;
			DrawRegion.w=CO_NUM_ARROW_W;
			DrawRegion.Color=CO_NUM_TRAN_COLOR;
			Gui_DrawImgArray(gCtrlObj_NumLeftArrow,&DrawRegion);//左箭头绘画
		}
		else if(Left == DNA_HiLightArrow)
		{
			DrawRegion.x=pNumBoxObj->x;
			DrawRegion.w=CO_NUM_ARROW_W;
			Gui_DrawImgArray(gCtrlObj_NumLeftArrowH,&DrawRegion);//左箭头绘画
		}

		if(Middle != DNA_Null)//中间框部分
		{
			DrawRegion.x=pNumBoxObj->x+CO_NUM_ARROW_W;
			DrawRegion.y=pNumBoxObj->y;
			DrawRegion.w=CO_NUM_MIDDLE_W;
			DrawRegion.h=CO_NUM_H;
			DrawRegion.Color=CO_NUM_TRAN_COLOR;
		}
		if(Middle == DNA_Normal)
			Gui_FillImgArray_H(gCtrlObj_NumMiddle,pNumBoxObj->w-(CO_NUM_ARROW_W<<1),&DrawRegion);	//高亮框
		else if(Middle == DNA_HiLight)
			Gui_FillImgArray_H(gCtrlObj_NumMiddleH,pNumBoxObj->w-(CO_NUM_ARROW_W<<1),&DrawRegion);	//框

		if(Middle != DNA_Null)//中间数字部分
		{
			sprintf((void *)NumStr,"%d",pNumBoxObj->Value);
			DrawRegion.x=pNumBoxObj->x+((pNumBoxObj->w-strlen((void *)NumStr)*CO_NUM_FONT_W)>>1);
			DrawRegion.y=pNumBoxObj->y+3;
			DrawRegion.w=pNumBoxObj->w-(CO_NUM_ARROW_W<<1);
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
			DrawRegion.x=pNumBoxObj->x+pNumBoxObj->w-CO_NUM_ARROW_W;
			DrawRegion.y=pNumBoxObj->y;
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

//根据num box控件信息算出小键盘显示位置
void CalculateKeyBoardPosition(NUM_BOX_OBJ *pNumBoxObj, GUI_REGION *pDrawReg)
{
	u16 x,y;

	x=pNumBoxObj->x+(pNumBoxObj->w>>1);
	y=pNumBoxObj->y+(CO_NUM_H>>1);

	MemCpy(pDrawReg,&KeyBoardRegionTmp,sizeof(GUI_REGION));//复制模板内容
	
	if(x<(LCD_WIDTH>>1))//左对齐
	{
		pDrawReg->x=pNumBoxObj->x;
	}
	else//右对齐
	{
		pDrawReg->x=pNumBoxObj->x+pNumBoxObj->w-pDrawReg->w;
	}

	if(y<(LCD_HIGHT>>1))//画到下方
	{
		pDrawReg->y=pNumBoxObj->y+CO_NUM_H+TOUCH_REGION_MARGIN;
	}
	else //画到上方
	{
		pDrawReg->y=pNumBoxObj->y-pDrawReg->h-TOUCH_REGION_MARGIN;
	}
}

//根据信息绘制小键盘并注册动态按键
void DrawAndRegKeyBoard(GUI_REGION *pDrawRegion)
{
	u8 i;

	pDrawRegion->Color=KEY_BOARD_BG_COLOR;
	Gui_FillBlock(pDrawRegion);

	for(i=0;i<sizeof(NumTchRegCon)/sizeof(CHAR_TCH_OBJ);i++)
	{
		MemCpy(&gpNbpVars->KeyBoard[i],&NumTchRegCon[i],sizeof(CHAR_TCH_OBJ));
		gpNbpVars->KeyBoard[i].x+=pDrawRegion->x;
		gpNbpVars->KeyBoard[i].y+=pDrawRegion->y;
		Q_SetDynamicCharTch(i+1,&gpNbpVars->KeyBoard[i]);//建立动态按键区域
	}
}
//-----------------------本页系统函数----------------------

//发生某些事件时，会触发的函数
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam)
{
	NUM_BOX_OBJ *pNumBoxObj=pSysParam;
	GUI_REGION DrawRegion;
	
	switch(SysEvent)
	{
		case Sys_PreGotoPage:
			if((pNumBoxObj->Type == NCOT_NumBox) &&(Q_DB_GetStatus(Status_FsInitFinish,NULL)==FALSE)) //文件系统没有挂载
			{
				Debug("File system not mount!!!\n\r");
				return SM_State_Faile|SM_NoGoto;
			}			
			return SM_State_OK;
		case Sys_PageInit:		//系统每次打开这个页面，会处理这个事件		
			gpNbpVars=Q_PageMallco(sizeof(NUM_CTRL_OBJ_PAGE_VARS));
			gpNbpVars->HandlerType=pNumBoxObj->Type;

			if((pNumBoxObj->Type == NCOT_NumList)||(pNumBoxObj->Type == NCOT_NumEnum)||(pNumBoxObj->Type == NCOT_NumBox))
			{
				if(pNumBoxObj->Type == NCOT_NumBox)
				{
					CalculateKeyBoardPosition(pNumBoxObj,&DrawRegion);//计算小键盘区域
					PrtScreenToBin("/Temp/NumBoxPS.buf",DrawRegion.x,DrawRegion.y,DrawRegion.w,DrawRegion.h);
					DrawAndRegKeyBoard(&DrawRegion);
				}
				
				DrawRegion.x=pNumBoxObj->x;
				DrawRegion.y=pNumBoxObj->y;
				DrawRegion.w=CO_NUM_ARROW_W;
				DrawRegion.h=CO_NUM_H;
				Gui_ReadRegion16Bit(gpNbpVars->LeftDispBuf,&DrawRegion);//保存旧图

				DrawRegion.x=pNumBoxObj->x+pNumBoxObj->w-CO_NUM_ARROW_W;
				DrawRegion.y=pNumBoxObj->y;
				DrawRegion.w=CO_NUM_ARROW_W;
				DrawRegion.h=CO_NUM_H;
				Gui_ReadRegion16Bit(gpNbpVars->RightDispBuf,&DrawRegion);//保存旧图

				DrawNumCtrlObj((void *)pNumBoxObj,DNA_NormalArrow,DNA_HiLight,DNA_NormalArrow);//画新图

				MemCpy(&gpNbpVars->LeftArrowTch,&ArrowTchTmp,sizeof(IMG_TCH_OBJ));//左箭头
				gpNbpVars->LeftArrowTch.ObjID=LeftArrowKV;
				gpNbpVars->LeftArrowTch.x=pNumBoxObj->x-TOUCH_REGION_MARGIN;
				gpNbpVars->LeftArrowTch.y=pNumBoxObj->y-TOUCH_REGION_MARGIN;
				gpNbpVars->LeftArrowTch.w+=(TOUCH_REGION_MARGIN<<1);
				gpNbpVars->LeftArrowTch.h+=(TOUCH_REGION_MARGIN<<1);
				Q_SetDynamicImgTch(1,&gpNbpVars->LeftArrowTch);//建立动态按键区域

				MemCpy(&gpNbpVars->RightArrowTch,&ArrowTchTmp,sizeof(IMG_TCH_OBJ));//右箭头
				gpNbpVars->RightArrowTch.ObjID=RightArrowKV;
				gpNbpVars->RightArrowTch.x=pNumBoxObj->x+pNumBoxObj->w-CO_NUM_ARROW_W-TOUCH_REGION_MARGIN;
				gpNbpVars->RightArrowTch.y=pNumBoxObj->y-TOUCH_REGION_MARGIN;
				gpNbpVars->RightArrowTch.w+=(TOUCH_REGION_MARGIN<<1);
				gpNbpVars->RightArrowTch.h+=(TOUCH_REGION_MARGIN<<1);
				Q_SetDynamicImgTch(2,&gpNbpVars->RightArrowTch);//建立动态按键区域

				MemCpy(&gpNbpVars->NumTch,&NumTchTmp,sizeof(IMG_TCH_OBJ));//数字框模板
				gpNbpVars->NumTch.ObjID=NumFrameKV;
				gpNbpVars->NumTch.x=pNumBoxObj->x+CO_NUM_ARROW_W+TOUCH_REGION_MARGIN;
				gpNbpVars->NumTch.y=DrawRegion.y;
				gpNbpVars->NumTch.w=pNumBoxObj->w-(CO_NUM_ARROW_W<<1)-(TOUCH_REGION_MARGIN<<1);
				Q_SetDynamicImgTch(3,&gpNbpVars->NumTch);//建立动态按键区域

				gpNbpVars->ObjHandler=pNumBoxObj;//记录控件指针			
				gpNbpVars->DefValue=pNumBoxObj->Value;
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
				if((gpNbpVars->HandlerType==NCOT_NumList)||(gpNbpVars->HandlerType==NCOT_NumEnum)||(gpNbpVars->HandlerType==NCOT_NumBox))
				{
					pNumBoxObj=(void *)gpNbpVars->ObjHandler;
					
					DrawRegion.x=pNumBoxObj->x;
					DrawRegion.y=pNumBoxObj->y;
					DrawRegion.w=CO_NUM_ARROW_W;
					DrawRegion.h=CO_NUM_H;
					DrawRegion.Color=FatColor(NO_TRANS);
					Gui_DrawImgArray((void *)gpNbpVars->LeftDispBuf,&DrawRegion);//还原

					DrawRegion.x=pNumBoxObj->x+pNumBoxObj->w-CO_NUM_ARROW_W;
					DrawRegion.y=pNumBoxObj->y;
					DrawRegion.w=CO_NUM_ARROW_W;
					DrawRegion.h=CO_NUM_H;
					DrawRegion.Color=FatColor(NO_TRANS);
					Gui_DrawImgArray((void *)gpNbpVars->RightDispBuf,&DrawRegion);//还原

					if(gpNbpVars->DefValue == pNumBoxObj->Value)
						DrawNumCtrlObj((void *)pNumBoxObj,DNA_Normal,DNA_Normal,DNA_Normal);//未改变原有值
					else
						DrawNumCtrlObj((void *)pNumBoxObj,DNA_HiLight,DNA_HiLight,DNA_HiLight);//改变了原有值

					if(gpNbpVars->HandlerType==NCOT_NumBox)
					{
						pNumBoxObj=(void *)gpNbpVars->ObjHandler;
						CalculateKeyBoardPosition(pNumBoxObj,&DrawRegion);//计算小键盘区域
						Gui_DrawImgBin("/Temp/NumBoxPS.buf",&DrawRegion);
					}
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
		case LeftArrowKV://-
			if(InEvent == Tch_Press)
			{
				NUM_BOX_OBJ *pNumBox=(void *)gpNbpVars->ObjHandler;
				DrawNumCtrlObj(pNumBox,DNA_HiLightArrow,DNA_Null,DNA_Null);//画新图
			}
			else
			{
				NUM_BOX_OBJ *pNumBox=(void *)gpNbpVars->ObjHandler;
				NUM_LIST_OBJ *pNumList=(void *)gpNbpVars->ObjHandler;
				NUM_ENUM_OBJ *pNumEnum=(void *)gpNbpVars->ObjHandler;
				if(InEvent == Tch_Release)
				{
					if(pNumBox->Type == NCOT_NumBox)
					{
						pNumBox->Value-=1;
					}
					else if(pNumList->Type == NCOT_NumList)
					{
						if((pNumList->Value-pNumList->Step)>=pNumList->Min)
							pNumList->Value-=pNumList->Step;
						else pNumList->Value=pNumList->Max;
					}
					else if(pNumEnum->Type == NCOT_NumEnum)
					{
						if((pNumEnum->Idx-1)>=0) pNumEnum->Idx--;
						else pNumEnum->Idx=(pNumEnum->Total-1);
						pNumEnum->Value=pNumEnum->pEnumList[pNumEnum->Idx];			
					}
				}
				DrawNumCtrlObj(pNumBox,DNA_NormalArrow,DNA_HiLight,DNA_Null);//画新图
			}
			break;
		case RightArrowKV://+
			if(InEvent == Tch_Press)
			{
				NUM_BOX_OBJ *pNumBox=(void *)gpNbpVars->ObjHandler;
				DrawNumCtrlObj(pNumBox,DNA_Null,DNA_Null,DNA_HiLightArrow);//画新图
			}
			else
			{					
				NUM_BOX_OBJ *pNumBox=(void *)gpNbpVars->ObjHandler;
				NUM_LIST_OBJ *pNumList=(void *)gpNbpVars->ObjHandler;
				NUM_ENUM_OBJ *pNumEnum=(void *)gpNbpVars->ObjHandler;
				if(InEvent == Tch_Release)
				{
					if(pNumBox->Type == NCOT_NumBox)
					{
						pNumBox->Value+=1;
					}
					else if(pNumList->Type == NCOT_NumList)
					{
						if((pNumList->Value+pNumList->Step)<=pNumList->Max)
							pNumList->Value+=pNumList->Step;
						else pNumList->Value=pNumList->Min;
					}
					else if(pNumEnum->Type == NCOT_NumEnum)
					{
						if((pNumEnum->Idx+1)<pNumEnum->Num) pNumEnum->Idx++;
						else pNumEnum->Idx=0;
						pNumEnum->Value=pNumEnum->pEnumList[pNumEnum->Idx];		
					}
				}
				DrawNumCtrlObj(pNumBox,DNA_Null,DNA_HiLight,DNA_NormalArrow);//画新图
			}
			break;
		case Key0KV:case Key1KV:case Key2KV:case Key3KV:case Key4KV:
		case Key5KV:case Key6KV:case Key7KV:case Key8KV:case Key9KV:
			{
				NUM_BOX_OBJ *pNumBox=(void *)gpNbpVars->ObjHandler;
				Key-=Key0KV;
				pNumBox->Value=pNumBox->Value*10+Key;
				
				DrawNumCtrlObj(pNumBox,DNA_Null,DNA_HiLight,DNA_Null);//画新图
			}
			break;
		case KeyMinusKv://+-
			{
				NUM_BOX_OBJ *pNumBox=(void *)gpNbpVars->ObjHandler;
				pNumBox->Value=0-pNumBox->Value;
				DrawNumCtrlObj(pNumBox,DNA_Null,DNA_HiLight,DNA_Null);//画新图
			}
			break;
		case KeyDotKV://.
			break;
		case KeyCancleKV://X
			{
				NUM_BOX_OBJ *pNumBox=(void *)gpNbpVars->ObjHandler;
				pNumBox->Value/=10;
				DrawNumCtrlObj(pNumBox,DNA_Null,DNA_HiLight,DNA_Null);//画新图
			}
			break;
		case KeyOkKV://O
		case NumFrameKV://点击窗体本身表示返回
			Q_GotoPage(SubPageReturn,"",0,gpNbpVars->ObjHandler);//返回前一个页面
			break;
			
		default:
			//需要响应的事件未定义
			Debug("%s TouchEventHandler:This Touch Event Handler case unfinish! Key:%d\n\r",Q_GetCurrPageName(),Key);
			///while(1);
	}
	
	return 0;
}



