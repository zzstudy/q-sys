//本页用于存杂项
#include "System.h"

//用于系统的各个地方互斥读写spi flash
bool Q_SpiFlashSync(FLASH_CMD cmd,u32 addr,u32 len,u8 *buf)
{
#if(QXW_PRODUCT_ID==114)
	OS_DeclareCritical();

	OS_EnterCritical();
#endif
	
	switch(cmd)
	{
		case FlashRead:
			M25P16_Fast_Read_Data(addr,len,buf);
			break;
		case FlashWrite:
			M25P16_Page_Program(addr,len,buf);
			break;
		case FlashSectorEarse:
			M25P16_Sector_Erase(addr);
			break;
		case FlashBlukEarse:
			M25P16_Bulk_Erase();
			break;
	}

#if(QXW_PRODUCT_ID==114)
	OS_ExitCritical();
#endif

	return TRUE;
}

//TimId用来选择定时器，当Sys_Timer事件触发时，IntParam参数传递的就是此id
//uS_Base 表示单位(时基)，为1时，单位是us；为100时，单位是100us；最小值1，最大值900
//Val最大值65535
//最终定时值= Val x uS_Base x 1us
//新定时设定会覆盖旧设定
//AutoReload用来设定是一次定时还是循环定时
//val和uS_Base其中任意一个为0，则停止当前定时。
void Q_TimSet(Q_TIM_ID TimId,u16 Val,u16 uS_Base, bool AutoReload)
{
	switch(TimId)
	{
		case Q_TIM1:
			Tim2_Set(Val,uS_Base,AutoReload);
			break;
		case Q_TIM2:
			Tim4_Set(Val,uS_Base,AutoReload);
			break;
		case Q_TIM3:
			Tim5_Set(Val,uS_Base,AutoReload);
			break;
		default:
			Debug("No Such Timer in Q-sys!\n\r");
	}
}

//通用的串口输入处理
//Len 字符串个数
//pStr 字符串
//将处理如下命令:
//Go [PageName] :以GotoNewPage方式切换到指定页面，参数为0和NULL
//Press [KeyNum] :模拟按键
#define UART_CMD_MAX_PARAM_NUM 4//最长参数
void Q_UartCmdHandler(u16 Len,u8 *pStr)
{
	u16 i,n;
	u8 *pCmd=NULL;
	u8 *pParam[UART_CMD_MAX_PARAM_NUM];
	
	if(Len==0)//控制字符
	{
		if(((u16 *)pStr)[0]==0x445b)
		{

		}
		else if(((u16 *)pStr)[0]==0x435b)
		{

		}
		else
		{
			Debug("CtrlCode:%x\n\r",((u16 *)pStr)[0]);
		}	
		return ;
	}

	for(n=0;n<UART_CMD_MAX_PARAM_NUM;n++)	pParam[n]=NULL;//清空参数
		
	pCmd=pStr;
	for(i=0,n=0;pStr[i];i++)//取参数
	{
		if(pStr[i]==' ')
		{
			pStr[i]=0;
			if(pStr[i+1]&&pStr[i+1]!=' ')
			{
				if(n>=UART_CMD_MAX_PARAM_NUM) break;
				pParam[n++]=&pStr[i+1];
			}
		}
	}

	Len=strlen((void *)pCmd);
	for(i=0;i<=Len;i++)//命令字符串全部转小写
	{
		if(pCmd[i]>='A' && pCmd[i]<='Z')
			pCmd[i]=pCmd[i]+32;
	}

	switch(pCmd[0])
	{
		case 'g':
			if(strcmp((void *)pCmd,"goto")==0)
			{
				if(pParam[0]&&pParam[0][0]=='?')//help
				{
					Debug("\n\r----------------------------------------------\n\r");
					Debug("Goto [PageName]\n\r");
					Debug("Goto [PageName] [IntParam] [pParam]\n\r");
					Debug("----------------------------------------------\n\r");
				}
				else if(pParam[0]&&pParam[0][0])
				{
					if(pParam[1]&&pParam[1][0])
						Q_GotoPage(GotoNewPage,pParam[0],StrToUint(pParam[1]),pParam[2]);		
					else
						Q_GotoPage(GotoNewPage,pParam[0],0,NULL);		
				}
			}
			break;
		case 'p':
			if(strcmp((void *)pCmd,"prtscreen")==0)
			{
				if(pParam[0]&&pParam[0][0]=='?')//help
				{
					Debug("\n\r----------------------------------------------\n\r");
					Debug("PrtScreen\n\r");
					Debug("----------------------------------------------\n\r");
				}
				else if(pParam[0]==NULL)
				{
					PrtScreen();
				}
			}
			break;
		case 'r':
			if(strcmp((void *)pCmd,"release")==0)
			{
				if(pParam[0]&&pParam[0][0]=='?')//help
				{
					Debug("\n\r----------------------------------------------\n\r");
					Debug("Release [KeyNum]\n\r");
					Debug("----------------------------------------------\n\r");
				}
				else if(pParam[0]&&pParam[0][0])
				{
					INPUT_EVENT KeyEvtParam;
					TOUCH_INFO  *pTouchInfo=&KeyEvtParam.Info.TouchInfo;
					u16 Idx=StrToUint(pParam[0]);

					pTouchInfo->Id=pTouchInfo->x=pTouchInfo->y=0;
					pTouchInfo->TimeStamp=OS_GetCurrentSysMs();
					KeyEvtParam.uType=Touch_Type;
					KeyEvtParam.EventType=Input_TchRelease;
					KeyEvtParam.Num=Idx;//区域索引传过去
					OS_MsgBoxSend(gInputHandler_Queue,&KeyEvtParam,100,FALSE);
				}
			}		
			break;
	}
}



