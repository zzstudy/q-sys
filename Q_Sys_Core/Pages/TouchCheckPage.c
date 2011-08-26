/**************** Q-SYS *******************
 * PageName : TouchCheckPage
 * Author : ChenYu
 * Version : 2.0
 * Base Q-Sys Version : 2.0
 * Description : touch check
 * WebSite : Www.Q-ShareWe.Com
 ***************** Q-SYS *******************/

/*
触摸屏校验2.0的修改如下：
1.修改了一处bug
2.将动态按键改为了静态按键并调整了位置
3.增加了强行退出的功能（点击home键）
4.增加了校验结果的显示
5.抽出了一组共八个宏方便用户定义校验范围
*/

#include "User.h"
#include "TouchCheckPage.h"
//修改以下8个宏可设置最大校验区间
#define    X_RATIO_MIN  0.9	 
#define    X_RATIO_MAX  1.1
#define    Y_RATIO_MIN  0.9
#define    Y_RATIO_MAX  1.1
#define    X_OFFSET_MIN (-7)
#define    X_OFFSET_MAX 7
#define    Y_OFFSET_MIN (-7)
#define    Y_OFFSET_MAX 7
#define    Bg_X        0
#define    Bg_Y        0
#define    Bg_W        240
#define    Bg_H        320
#define	   LeftUp_X	   20
#define	   LeftUp_Y	   20
#define	   RightUp_X   220
#define	   RightUp_Y   20
#define	   LeftDown_X  20
#define	   LeftDown_Y  300
#define	   RightDown_X 220
#define	   RightDown_Y 300
#define	   MsgBox_X		37
#define	   MsgBox_Y		100
#define	   MsgBox_W		168
#define	   MsgBox_H		16
extern float X_RATIO;
extern float Y_RATIO;
extern s32 X_OFFSET;
extern s32 Y_OFFSET;
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam);
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam);
static TCH_MSG TouchEventHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo);
enum{
	ExtiKeyDown=EXTI_KEY_VALUE_START,//系统默认将外部中断按键发送到第一个键值
	ExtiKeyUp,
	ExtiKeyEnter,
	LU,
	LD,
	RU,
	RD,
	MsgBox,
    HomeKV,
    MailKV,
    RingKV,
    CallKV
};
static const IMG_TCH_OBJ ImgTchRegCon[]={
	{"",LU,RelMsk,LeftUp_X-19,LeftUp_Y-19,38,38,0,0,"",FatColor(NO_TRANS)},
    {"",RU,RelMsk,RightUp_X-19,RightUp_Y-19,38,38,0,0,"",FatColor(NO_TRANS)},
    {"",LD,RelMsk,LeftDown_X-19,LeftDown_Y-19,38,38,0,0,"",FatColor(NO_TRANS)},
    {"",RD,RelMsk,RightDown_X-19,RightDown_Y-19,38,38,0,0,"",FatColor(NO_TRANS)},
	{"",MsgBox,RelMsk,MsgBox_X,MsgBox_Y,MsgBox_W,MsgBox_H,0,0,"",FatColor(NO_TRANS)},
    {"",HomeKV,RelMsk,0,320,60,30,0,0,"",FatColor(NO_TRANS)},
    {"",MailKV,RelMsk,0,320,60,30,0,0,"",FatColor(NO_TRANS)},
    {"",RingKV,RelMsk,0,320,60,30,0,0,"",FatColor(NO_TRANS)},
    {"",CallKV,RelMsk,0,320,60,30,0,0,"",FatColor(NO_TRANS)},
};
const PAGE_ATTRIBUTE TouchCheckPage={
    "TouchCheckPage",
    "ChenYu",
    "touch check",
    NORMAL_PAGE,
    0,
    {
		sizeof(ImgTchRegCon)/sizeof(IMG_TCH_OBJ), //size of touch region array
		0,//sizeof(CharTchRegCon)/sizeof(CHAR_TCH_OBJ), //size of touch region array,
	},
    ImgTchRegCon,
    NULL,
    SystemEventHandler,
	PeripheralsHandler,
	Bit(Perip_KeyRelease),
    TouchEventHandler,
};
typedef struct{
	bool lu_flag;
	bool ru_flag;
	bool ld_flag;
	bool rd_flag;
	u32 lu_x;
	u32 lu_y;
	u32 ru_x;
	u32 ru_y;
	u32 ld_x;
	u32 ld_y;
	u32 rd_x;
	u32 rd_y;
	float x_ratio;
	float y_ratio;
	s32 x_offset;
	s32 y_offset;
	GUI_REGION DrawRegion;
	bool OkFlag;
}TOUCH_CHK_PAGE_VARS;
TOUCH_CHK_PAGE_VARS *gVars;
void Init_TouchChkPageVars(int IntParam,void *pParam){
	X_RATIO=1.0;
	Y_RATIO=1.0;
	X_OFFSET=0.0;
	Y_OFFSET=0.0;
	gVars->lu_flag=FALSE;
	gVars->ru_flag=FALSE;
	gVars->ld_flag=FALSE;
	gVars->rd_flag=FALSE;
	gVars->lu_x=LeftUp_X;
	gVars->lu_y=LeftUp_Y;
	gVars->ru_x=RightUp_X;
	gVars->ru_y=RightUp_Y;
	gVars->ld_x=LeftDown_X;
	gVars->ld_y=LeftDown_Y;
	gVars->rd_x=RightDown_X;
	gVars->rd_y=RightDown_Y;
	gVars->x_ratio =1;
	gVars->y_ratio =1;
	gVars->x_offset=0;
	gVars->y_offset=0;
	gVars->DrawRegion.x=MsgBox_X;
	gVars->DrawRegion.y=MsgBox_Y;
	gVars->DrawRegion.w=MsgBox_W;
	gVars->DrawRegion.h=MsgBox_H;
	gVars->DrawRegion.Color=FatColor(0xffffff);
	gVars->DrawRegion.Space=0x50;
	gVars->OkFlag=FALSE;
}
static void Draw_Bg(void){
    GUI_REGION DrawRegion;
    DrawRegion.x=Bg_X;
    DrawRegion.y=Bg_Y;
    DrawRegion.w=Bg_W;
    DrawRegion.h=Bg_H;
    DrawRegion.Color=FatColor(0x5a5a5a);
    Gui_FillBlock(&DrawRegion);
}
static void Draw_Focus(u32 x,u32 y,bool IfClick){
	if(IfClick==FALSE){
		Gui_DrawLine(x-8, y-8,x+8, y+8,FatColor(0x00ff00));
		Gui_DrawLine(x-8, y+8,x+8, y-8,FatColor(0x00ff00));
	}
	else{
		Gui_DrawLine(x-8, y-8,x+8, y+8,FatColor(0xff0000));
		Gui_DrawLine(x-8, y+8,x+8, y-8,FatColor(0xff0000));
	}
}

static void Draw_Result(void){
	GUI_REGION DrawRegion;
	char charbuf[32];
	DrawRegion.x=MsgBox_X;
    DrawRegion.y=MsgBox_Y+MsgBox_H+32;
    DrawRegion.w=MsgBox_W;
    DrawRegion.h=MsgBox_H;
	DrawRegion.Color=FatColor(0x0000ff);
	DrawRegion.Space=0x00;
	sprintf(charbuf,"X_RATIO =%.3f\n\r",gVars->x_ratio);
	Gui_DrawFont(GBK16_FONT,(void *)charbuf,&DrawRegion);
	DrawRegion.y+=18;
	sprintf(charbuf,"Y_RATIO =%.3f\n\r",gVars->y_ratio);
	Gui_DrawFont(GBK16_FONT,(void *)charbuf,&DrawRegion);
	DrawRegion.y+=18;
	sprintf(charbuf,"X_OFFSET=%d\n\r",gVars->x_offset);
	Gui_DrawFont(GBK16_FONT,(void *)charbuf,&DrawRegion);
	DrawRegion.y+=18;
	sprintf(charbuf,"Y_OFFSET=%d\n\r",gVars->y_offset);
	Gui_DrawFont(GBK16_FONT,(void *)charbuf,&DrawRegion);
}

static bool Calculate(void){
	float x_ratio1;
	float x_ratio2;
	float y_ratio1;
	float y_ratio2;
	s32   x_offset1;
	s32   x_offset2;
	s32   x_offset3;
	s32   x_offset4;
	s32   y_offset1;
	s32   y_offset2;
	s32   y_offset3;
	s32   y_offset4;
	
	x_ratio1 =(float)(gVars->ru_x-gVars->lu_x)/(float)(RightUp_X-LeftUp_X);
	x_ratio2 =(float)(gVars->rd_x-gVars->ld_x)/(float)(RightDown_X-LeftDown_X);
	
	y_ratio1 =(float)(gVars->ld_y-gVars->lu_y)/(float)(LeftDown_Y-LeftUp_Y);
	y_ratio2 =(float)(gVars->rd_y-gVars->ru_y)/(float)(RightDown_Y-RightUp_Y);
	
	x_offset1=LeftUp_X-(s32)(gVars->x_ratio*(float)gVars->lu_x);
	x_offset2=LeftDown_X-(s32)(gVars->x_ratio*(float)gVars->ld_x);
	x_offset3=RightUp_X-(s32)(gVars->x_ratio*(float)gVars->ru_x);
	x_offset4=RightDown_X-(s32)(gVars->x_ratio*(float)gVars->rd_x);
	
	y_offset1=LeftUp_Y-(s32)(gVars->y_ratio*(float)gVars->lu_y);
	y_offset2=LeftDown_Y-(s32)(gVars->y_ratio*(float)gVars->ld_y);
	y_offset3=RightUp_Y-(s32)(gVars->y_ratio*(float)gVars->ru_y);
	y_offset4=RightDown_Y-(s32)(gVars->y_ratio*(float)gVars->rd_y);
	
	gVars->x_ratio=(x_ratio1+x_ratio2)/(float)2;
	gVars->y_ratio=(y_ratio1+y_ratio2)/(float)2;
	gVars->x_offset=(x_offset1+x_offset2+x_offset3+x_offset4)/4;
	gVars->y_offset=(y_offset1+y_offset2+y_offset3+y_offset4)/4;
	
	if( gVars->x_ratio>X_RATIO_MAX || gVars->x_ratio<X_RATIO_MIN || gVars->y_ratio>Y_RATIO_MAX || gVars->y_ratio<Y_RATIO_MIN )
		return FALSE;
		
	if( gVars->x_offset>X_OFFSET_MAX || gVars->x_offset<X_OFFSET_MIN || gVars->y_offset>Y_OFFSET_MAX || gVars->y_offset<Y_OFFSET_MIN )
		return FALSE;
	
	return TRUE;
}
static void JudgeAndCal(void){
	gVars->DrawRegion.Color=FatColor(0x5a5a5a);
	Gui_FillBlock(&gVars->DrawRegion);
	gVars->DrawRegion.Color=FatColor(0xffffff);
	Gui_DrawFont(GBK16_FONT,"   点此重校",&gVars->DrawRegion);
	if( gVars->lu_flag==TRUE && gVars->ld_flag==TRUE && gVars->ru_flag==TRUE && gVars->rd_flag==TRUE ){
		if(Calculate()==TRUE){
			gVars->DrawRegion.Color=FatColor(0x5a5a5a);
			Gui_FillBlock(&gVars->DrawRegion);
			gVars->DrawRegion.Color=FatColor(0x00ff00);
			Gui_DrawFont(GBK16_FONT,"校验完成点此退出",&gVars->DrawRegion);
			Draw_Result();
			gVars->OkFlag=TRUE;
		}else{
			gVars->DrawRegion.Color=FatColor(0x5a5a5a);
			Gui_FillBlock(&gVars->DrawRegion);
			gVars->DrawRegion.Color=FatColor(0x00ff00);
			Gui_DrawFont(GBK16_FONT,"校验失败点此重校",&gVars->DrawRegion);
			Draw_Result();
			gVars->OkFlag=FALSE;
		}
	}
}
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent,int IntParam,void *pSysParam){
    switch(SysEvent){
        case Sys_PreGotoPage:{
        }break;
        case Sys_PageInit:{
			gVars=(TOUCH_CHK_PAGE_VARS *)Q_PageMallco(sizeof(TOUCH_CHK_PAGE_VARS));			
			 if(gVars==0){
				Q_ErrorStopScreen("gVars malloc fail !\n\r");
			 }
			Init_TouchChkPageVars(IntParam,pSysParam);
			Draw_Bg();
			Draw_Focus(LeftUp_X,LeftUp_Y,FALSE);
			Draw_Focus(LeftDown_X,LeftDown_Y,FALSE);
			Draw_Focus(RightUp_X,RightUp_Y,FALSE);
			Draw_Focus(RightDown_X,RightDown_Y,FALSE);
			Gui_DrawFont(GBK16_FONT,"请点击十字叉中心",&gVars->DrawRegion);
        }break;
        case Sys_SubPageReturn:{
        }break;
        case Sys_PageClean:{
			Q_PageFree(gVars);
        }break;
    }
    return 0;
}

static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam)
{
	switch(PeripEvent)
	{
		case Perip_KeyRelease:
			switch(IntParam){
				case ExtiKeyEnter:
					PrtScreen();
					break;
				case ExtiKeyUp:
					break;
				case ExtiKeyDown:
					break; 
			}break;
	}
	
	return 0;
}

static TCH_MSG TouchEventHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo){
    switch(Key){
		case LU:{
			if(gVars->lu_flag==FALSE){
				gVars->lu_flag=TRUE;
				Draw_Focus(LeftUp_X,LeftUp_Y,TRUE);
				gVars->lu_x=pTouchInfo->x;
				gVars->lu_y=pTouchInfo->y;				
				JudgeAndCal();
			}
		}break;
		case LD:{
			if(gVars->ld_flag==FALSE){
				gVars->ld_flag=TRUE;
				Draw_Focus(LeftDown_X,LeftDown_Y,TRUE);
				gVars->ld_x=pTouchInfo->x;
				gVars->ld_y=pTouchInfo->y;
				JudgeAndCal();
			}
		}break;
		case RU:{
			if(gVars->ru_flag==FALSE){
				gVars->ru_flag=TRUE;
				Draw_Focus(RightUp_X,RightUp_Y,TRUE);
				gVars->ru_x=pTouchInfo->x;
				gVars->ru_y=pTouchInfo->y;
				JudgeAndCal();
			}
		}break;
		case RD:{
			if(gVars->rd_flag==FALSE){
				gVars->rd_flag=TRUE;
				Draw_Focus(RightDown_X,RightDown_Y,TRUE);
				gVars->rd_x=pTouchInfo->x;
				gVars->rd_y=pTouchInfo->y;
				JudgeAndCal();
			}
		}break;
		case MsgBox:{
			if(gVars->OkFlag==TRUE){
				Debug("gVars->lu_x=%d\n\r",gVars->lu_x);
				Debug("gVars->lu_y=%d\n\r",gVars->lu_y);
				Debug("gVars->ld_x=%d\n\r",gVars->ld_x);
				Debug("gVars->ld_y=%d\n\r",gVars->ld_y);
				Debug("gVars->ru_x=%d\n\r",gVars->ru_x);
				Debug("gVars->ru_y=%d\n\r",gVars->ru_y);
				Debug("gVars->rd_x=%d\n\r",gVars->rd_x);
				Debug("gVars->rd_y=%d\n\r",gVars->rd_y);
				X_RATIO=gVars->x_ratio;
				Y_RATIO=gVars->y_ratio;
				X_OFFSET=gVars->x_offset;
				Y_OFFSET=gVars->y_offset;
				Debug("X_RATIO=%.3f\n\r",X_RATIO);
				Debug("Y_RATIO=%.3f\n\r",Y_RATIO);
				Debug("X_OFFSET=%d\n\r",X_OFFSET);
				Debug("Y_OFFSET=%d\n\r",Y_OFFSET);
				Q_GotoPage(GotoNewPage,"AppListPage",-1,NULL);
			}
			else{
				Init_TouchChkPageVars(0,NULL);
				Draw_Bg();
				Draw_Focus(LeftUp_X,LeftUp_Y,FALSE);
				Draw_Focus(LeftDown_X,LeftDown_Y,FALSE);
				Draw_Focus(RightUp_X,RightUp_Y,FALSE);
				Draw_Focus(RightDown_X,RightDown_Y,FALSE);
				Gui_DrawFont(GBK16_FONT,"请点击十字叉中心",&gVars->DrawRegion);
			}
		}break;
        case HomeKV:{
			Q_GotoPage(GotoNewPage,"AppListPage",-1,NULL);	
        }break;
        case MailKV:{
        }break;
        case RingKV:{
        }break;
        case CallKV:{
        }break;
    }
    return 0;
}
