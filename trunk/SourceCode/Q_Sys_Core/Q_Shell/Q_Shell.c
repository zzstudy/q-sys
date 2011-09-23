#include "Q_Shell.h"
#include "string.h"
#include "stdio.h"

#if USE_Q_SHELL_FUNCTION
/*命令类型枚举*/
typedef enum{
	QSCT_CALL_FUN,    //调用函数
	QSCT_GET_VAR,     //查看变量值
	QSCT_SET_VAR,     //给变量赋值
	QSCT_GET_BY_ADDR, //获得指定地址处的变量或寄存器的值(32位)	
	QSCT_SET_BY_ADDR, //赋予指定地址处的变量或寄存器指定值(32位)	
	QSCT_LIST_FUN,    //列出所有可被访问的函数的描述信息
	QSCT_LIST_VAR,    //列出所有可被访问的变量的描述信息
	QSCT_ERROR        //无效命令类型
}Q_SH_CALL_TYPE;

/* 命令信息 */
#define QSH_FUN_PARA_MAX_NUM 	     4                //函数最多只可接受4个参数
#define QSH_FUN_PARA_TYPE_NOSTRING   0
#define QSH_FUN_PARA_TYPE_STRING     1
typedef struct{
	char             *CmdStr;                         //命令字符串的地址
	unsigned char     ParaStart[QSH_FUN_PARA_MAX_NUM];//函数各个参数的起始位置
	unsigned char     ParaEnd[QSH_FUN_PARA_MAX_NUM];  //函数各个参数的结束位置
	unsigned char	  ParaTypes[QSH_FUN_PARA_MAX_NUM];//函数各个参数的类型
	unsigned char     ParaNum;                        //函数参数个数
	unsigned char     CallStart;                      //调用名起始位置
	unsigned char     CallEnd;                        //调用名结束位置
	Q_SH_CALL_TYPE    CallType;                       //命令类型
}Q_SH_MSG_DEF;
static Q_SH_MSG_DEF Q_Sh_Msg;//占用20字节RAM

/*--------------------------------------------------- 
函数功能：截取字符串
入参
		base   母字符串起始地址
		start  待截字符串起始位置偏移量
		end    待截字符串结束位置偏移量
返回    截取的字符串首地址
 ---------------------------------------------------*/
static char *Q_Sh_StrCut(char *Base,unsigned int Start,unsigned int End)
{
	Base[End+1]=0;
	return (char *)((unsigned int)Base+Start);
}

/*--------------------------------------------------- 
函数功能：分析命令字符串，并将分析结果放入全局变量Q_Sh_Msg中
入参
		cmdstr   :命令字符串地址
返回    成功 1
		失败 0
注      被分析的命令字符串不允许包含空格
 ---------------------------------------------------*/
static unsigned int Q_Sh_AnalyCmdStr(char *CmdStr)
{
	unsigned int Index=0;
	unsigned int TmpPos=0;
	unsigned int ParamNum=0;
	Q_Sh_Msg.CmdStr=CmdStr;
	Q_Sh_Msg.CallStart=0;
	/* 调用名分离*/
	while(1)
	{
		if( CmdStr[Index]<33||CmdStr[Index]>126 )//非法字符侦测
		{
			Q_Sh_Msg.CallType=QSCT_ERROR;
			return 0;
		}
		if(CmdStr[Index]=='(')
		{
			if(Index==0)
			{
				Q_Sh_Msg.CallType=QSCT_ERROR;
				return 0;
			}
			Q_Sh_Msg.CallEnd=(Index-1);
			Index++;
			break;
		}
		Index++;
	}
	/* 参数分离*/
	TmpPos=Index;
	while(1)
	{
		if( CmdStr[Index]<33||CmdStr[Index]>126 )//非法字符侦测
		{
			Q_Sh_Msg.CallType=QSCT_ERROR;
			return 0;
		}
		if(CmdStr[Index]==',')
		{
			if(Index==TmpPos)
			{
				Q_Sh_Msg.CallType=QSCT_ERROR;
				return 0;
			}
			if(CmdStr[Index-1]=='\"')
			{
				Q_Sh_Msg.ParaTypes[ParamNum]=QSH_FUN_PARA_TYPE_STRING;
				Q_Sh_Msg.ParaStart[ParamNum]=TmpPos+1;
				Q_Sh_Msg.ParaEnd[ParamNum]=Index-2;
			}
			else
			{
				Q_Sh_Msg.ParaTypes[ParamNum]=QSH_FUN_PARA_TYPE_NOSTRING;
				Q_Sh_Msg.ParaStart[ParamNum]=TmpPos;
				Q_Sh_Msg.ParaEnd[ParamNum]=Index-1;
			}
			ParamNum++;
			TmpPos=Index+1;
		}
		if(CmdStr[Index]==')')
		{
			if(Index==Q_Sh_Msg.CallEnd+2)//无参数
			{
				Q_Sh_Msg.ParaNum=0;
				break;
			}
			if(Index==TmpPos)
			{
				Q_Sh_Msg.CallType=QSCT_ERROR;
				return 0;
			}
			if(CmdStr[Index-1]=='\"')
			{
				Q_Sh_Msg.ParaTypes[ParamNum]=QSH_FUN_PARA_TYPE_STRING;
				Q_Sh_Msg.ParaStart[ParamNum]=TmpPos+1;
				Q_Sh_Msg.ParaEnd[ParamNum]=Index-2;
			}
			else
			{
				Q_Sh_Msg.ParaTypes[ParamNum]=QSH_FUN_PARA_TYPE_NOSTRING;
				Q_Sh_Msg.ParaStart[ParamNum]=TmpPos;
				Q_Sh_Msg.ParaEnd[ParamNum]=Index-1;
			}
			ParamNum++;
			Q_Sh_Msg.ParaNum=ParamNum;
			break;
		}
		Index++;
	}
	/*  分析调用类型*/
	if( strcmp(Q_Sh_StrCut(Q_Sh_Msg.CmdStr,Q_Sh_Msg.CallStart,Q_Sh_Msg.CallEnd),"lf")==0 )
		Q_Sh_Msg.CallType=QSCT_LIST_FUN;//显示函数列表
	else if(strcmp(Q_Sh_StrCut(Q_Sh_Msg.CmdStr,Q_Sh_Msg.CallStart,Q_Sh_Msg.CallEnd),"lv")==0)
		Q_Sh_Msg.CallType=QSCT_LIST_VAR;//显示变量列表
	else if(strcmp(Q_Sh_StrCut(Q_Sh_Msg.CmdStr,Q_Sh_Msg.CallStart,Q_Sh_Msg.CallEnd),"get")==0)
		Q_Sh_Msg.CallType=QSCT_GET_VAR;//查看变量值
	else if(strcmp(Q_Sh_StrCut(Q_Sh_Msg.CmdStr,Q_Sh_Msg.CallStart,Q_Sh_Msg.CallEnd),"set")==0)
		Q_Sh_Msg.CallType=QSCT_SET_VAR;//设置变量值
	else if(strcmp(Q_Sh_StrCut(Q_Sh_Msg.CmdStr,Q_Sh_Msg.CallStart,Q_Sh_Msg.CallEnd),"read")==0)
		Q_Sh_Msg.CallType=QSCT_GET_BY_ADDR;//查看指定地址内存字的值
	else if(strcmp(Q_Sh_StrCut(Q_Sh_Msg.CmdStr,Q_Sh_Msg.CallStart,Q_Sh_Msg.CallEnd),"write")==0)
		Q_Sh_Msg.CallType=QSCT_SET_BY_ADDR;//设置指定地址内存字的值
	else
		Q_Sh_Msg.CallType=QSCT_CALL_FUN;//调用函数
	return 1;
}

/*-----------------------------------------------------------------------
由链接器赋值，不占据实际内存
------------------------------------------------------------------------*/
extern int QShellFunTab$$Base;	 //qShellFunTab段首地址
extern int QShellFunTab$$Limit;	 //qShellFunTab段尾地址
extern int QShellVarTab$$Base;	 //qShellVarTab段首地址
extern int QShellVarTab$$Limit;	 //qShellVarTab段尾地址

/*-----------------------------------------------------------------------
记录shell表段范围信息
------------------------------------------------------------------------*/
static QSH_RECORD *qsh_fun_table_begin = (void *)0; //记录qShellFunTab段首地址
static QSH_RECORD *qsh_fun_table_end   = (void *)0; //记录qShellFunTab段尾地址
static QSH_RECORD *qsh_var_table_begin = (void *)0; //记录qShellVarTab段首地址
static QSH_RECORD *qsh_var_table_end   = (void *)0; //记录qShellVarTab段尾地址

/*-----------------------------------------------------------------------
简述：统一的函数类型
注  ：使用这种类型可以接受不同个数的参数
------------------------------------------------------------------------*/
typedef unsigned int (*QSH_FUNC_TYPE)();
/*-----------------------------------------------------------------------
简述：统一的变量类型,只支持u32类型的变量
------------------------------------------------------------------------*/
typedef unsigned int QSH_VAR_TYPE;

/*-----------------------------------------------------------------------
函数功能：查询指定函数的记录
入参    ：函数名
返回    ：成功 对应的记录
		  失败 0
注      ：记录中包含的信息有：
		  函数名name，函数描述desc，函数地址addr 
------------------------------------------------------------------------*/
static QSH_RECORD* Q_Sh_FunFind(char* Name)
{
	QSH_RECORD* Index;
	for (Index = qsh_fun_table_begin; Index < qsh_fun_table_end; Index ++)
	{
		if (strcmp(Index->name, Name) == 0)
			return Index;
	}
	return (void *)0;
}

/*-----------------------------------------------------------------------
函数功能：查询指定变量的记录
入参    ：变量名
返回    ：成功 对应的记录
		  失败 0
注      ：记录中包含的信息有：变量名name，变量描述desc，变量地址addr
------------------------------------------------------------------------*/
static QSH_RECORD* Q_Sh_VarFind(char* Name)
{
	QSH_RECORD* Index;
	for (Index = qsh_var_table_begin; Index < qsh_var_table_end; Index ++)
	{
		if (strcmp(Index->name, Name) == 0)
			return Index;
	}
	return (void *)0;
}

/*-----------------------------------------------------------------------
函数功能：列出所有注册到QShellFunTab段中的函数
入参    ：无
返回    ：无
------------------------------------------------------------------------*/
static void Q_Sh_ListFun(void)
{
	QSH_RECORD* Index;
	for (Index = qsh_fun_table_begin; Index < qsh_fun_table_end; Index++)
	{
		printf("%s\r\n",Index->desc);
	}
}

/*-----------------------------------------------------------------------
函数功能：列出所有注册到QShellVarTab段中的变量
入参    ：无
返回    ：无
------------------------------------------------------------------------*/
static void Q_Sh_ListVar(void)
{
	QSH_RECORD* Index;
	for (Index = qsh_var_table_begin; Index < qsh_var_table_end; Index++)
	{
		printf("%s\r\n",Index->desc);
	}
}

/*--------------------------------------------------- 
函数功能：计算m的n次方
 ---------------------------------------------------*/
static unsigned int Q_Sh_Pow(unsigned int m,unsigned int n)
{
	unsigned int Result=1;	 
	while(n--)Result*=m;    
	return Result;
}

/*--------------------------------------------------------------	    
函数功能：把字符串转为数字
         支持16进制,支持大小写
         不支持负数 
入参：
		str:数字字符串指针
		res:转换完的结果存放地址.
返回值: 成功             0
        数据格式错误     1
	    16进制位数为0    2
	    起始格式错误.    3
	    十进制位数为0    4
--------------------------------------------------------------*/
static unsigned int Q_Sh_Str2num(char*Str,unsigned int *Res)
{
	unsigned int t;
	unsigned int bNum=0;	//数字的位数
	char *p;		  
	unsigned int HexDec=10;//默认为十进制数据
	p=Str;
	*Res=0;//清零.
	while(1)//全部转换为小写字母
	{
		if((*p>='A'&&*p<='F')||(*p=='X'))
			*p=*p+0x20;
		else if(*p=='\0')break;
		p++;
	}
	p=Str;
	while(1)
	{
		if((*p<='9'&&*p>='0')||(*p<='f'&&*p>='a')||(*p=='x'&&bNum==1))//参数合法
		{
			if(*p>='a')HexDec=16;	//字符串中存在字母,为16进制格式.
			bNum++;					//位数增加.
		}else if(*p=='\0')break;	//碰到结束符,退出.
		else return 1;				//不全是十进制或者16进制数据.
		p++; 
	} 
	p=Str;			    //重新定位到字符串开始的地址.
	if(HexDec==16)		//16进制数据
	{
		if(bNum<3)return 2;			//位数小于3，直接退出.因为0X就占了2个,如果0X后面不跟数据,则该数据非法.
		if(*p=='0' && (*(p+1)=='x'))//必须以'0x'开头.
		{
			p+=2;	//偏移到数据起始地址.
			bNum-=2;//减去偏移量	 
		}else return 3;//起始头的格式不对
	}else if(bNum==0)return 4;//位数为0，直接退出.	  
	while(1)
	{
		if(bNum)bNum--;
		if(*p<='9'&&*p>='0')t=*p-'0';	//得到数字的值
		else t=*p-'a'+10;				//得到A~F对应的值	    
		*Res+=t*Q_Sh_Pow(HexDec,bNum);		   
		p++;
		if(*p=='\0')break;//数据都查完了.	
	}
	return 0;//成功转换
}	

//以下为对外函数
/*-----------------------------------------------------------------------
函数功能：初始化
入参：无
返回：无
------------------------------------------------------------------------*/
void Q_Shell_Init(void)
{	
	qsh_fun_table_begin = (QSH_RECORD*) &QShellFunTab$$Base;
	qsh_fun_table_end   = (QSH_RECORD*) &QShellFunTab$$Limit;
	qsh_var_table_begin = (QSH_RECORD*) &QShellVarTab$$Base;
	qsh_var_table_end   = (QSH_RECORD*) &QShellVarTab$$Limit;
}

/*-----------------------------------------------------------------------
函数功能：执行命令
入参    
        ifctrl 0:接收到控制字符
			  !0:接收到命令字符串
		cmd    命令字符串
返回    ：成功 1
		 失败 0  
------------------------------------------------------------------------*/
unsigned int Q_Sh_CmdHandler(unsigned int IfCtrl,char *Cmd)
{
	QSH_RECORD *pRecord;
	unsigned int ret=1;
	unsigned int fun_para[QSH_FUN_PARA_MAX_NUM];
	unsigned int fun_resu;
	unsigned int var_set;
	unsigned int var_addr;
	unsigned int i;
	if(IfCtrl==0)//控制字符
	{
		if(((unsigned short *)Cmd)[0]==0x445b)
		{

		}
		else if(((unsigned short *)Cmd)[0]==0x435b)
		{

		}
		else
		{
			printf("CtrlCode:%x\n\r",((unsigned short *)Cmd)[0]);
		}	
		return 0;
	}
	Q_Sh_AnalyCmdStr(Cmd);
	switch(Q_Sh_Msg.CallType)//分类执行
	{
		case QSCT_ERROR://命令解析失败
		{
			printf("illegal Cmd string\r\n");
			ret=0;
		}
		break;
		
		case QSCT_CALL_FUN://调用函数
		{
			if(Q_Sh_Msg.ParaNum>4)//函数参数不能大于四个
			{	
				printf("fun's para can't over 4!!!\r\n");
				ret=0;
				break;
			}
			//在QShellFunTab段中查找变量的记录
			pRecord=Q_Sh_FunFind(Q_Sh_StrCut(Q_Sh_Msg.CmdStr,Q_Sh_Msg.CallStart,Q_Sh_Msg.CallEnd));
			if( pRecord == 0 )//若没找到对应的记录则说明输入的函数没有注册
			{
				printf("the fun have not been regist\r\n");
				ret=0;
				break;
			}
			for(i=0;i<Q_Sh_Msg.ParaNum;i++)//得到参数值
			{
				if(Q_Sh_Msg.ParaTypes[i]==QSH_FUN_PARA_TYPE_STRING)//若参数为字符串类型
				{
					fun_para[i]=(unsigned int)Q_Sh_StrCut(Q_Sh_Msg.CmdStr,Q_Sh_Msg.ParaStart[i],Q_Sh_Msg.ParaEnd[i]);					
				}
				else//若参数为数值类型
				{
					if(Q_Sh_Str2num(Q_Sh_StrCut(Q_Sh_Msg.CmdStr,Q_Sh_Msg.ParaStart[i],Q_Sh_Msg.ParaEnd[i]),&(fun_para[i]))!=0)
					{
						printf("num str format error\r\n");
						ret=0;
						break;
					}
				}
			}
			fun_resu=((QSH_FUNC_TYPE)(pRecord->addr))(fun_para[0],fun_para[1],fun_para[2],fun_para[3]);
			printf("return %d\r\n",fun_resu);
		}
		break;
		
		case QSCT_GET_VAR://查看变量值
		{
			if(Q_Sh_Msg.ParaNum!=1)//检查参数是否为1个，若不是则说明命令字符串格式不对
			{	
				printf("Cmd get must have only one para!!!\r\n");
				ret=0;
				break;
			}
			if(Q_Sh_Msg.ParaTypes[0]==QSH_FUN_PARA_TYPE_STRING)//第一个参数的类型不能是字符串
			{
				printf("Cmd get's para can not be string\r\n");
				ret=0;
				break;
			}
			pRecord=Q_Sh_VarFind(Q_Sh_StrCut(Q_Sh_Msg.CmdStr,Q_Sh_Msg.ParaStart[0],Q_Sh_Msg.ParaEnd[0]));//在QShellVarTab段中查找变量的记录
			if( pRecord == 0 )//若没找到对应的记录则说明输入的变量名没有注册
			{
				printf("the var have not been regist\r\n");
				ret=0;
				break;
			}
			printf("%s=%d\r\n",pRecord->name,*(QSH_VAR_TYPE *)(pRecord->addr));//打印变量值 
		}
		break;

		case QSCT_SET_VAR://给变量赋值
		{
			if(Q_Sh_Msg.ParaNum!=2)//检查参数是否为2个，若不是则说明命令字符串格式不对
			{	
				printf("Cmd set must have two para!!!\r\n");
				ret=0;
				break;
			}
			pRecord=Q_Sh_VarFind(Q_Sh_StrCut(Q_Sh_Msg.CmdStr,Q_Sh_Msg.ParaStart[0],Q_Sh_Msg.ParaEnd[0]));//在QShellVarTab段中查找变量的记录
			if( pRecord == 0 )//若没找到对应的记录则说明输入的变量名没有注册
			{
				printf("the var have not been regist\r\n");
				ret=0;
				break;
			}
			if(Q_Sh_Msg.ParaTypes[1]==QSH_FUN_PARA_TYPE_STRING)//第二个参数的类型不能是字符串
			{
				printf("can't set var string type\r\n");
				ret=0;
				break;
			}
			if(Q_Sh_Str2num(Q_Sh_StrCut(Q_Sh_Msg.CmdStr,Q_Sh_Msg.ParaStart[1],Q_Sh_Msg.ParaEnd[1]),&var_set)!=0)//获得设置值
			{
				printf("num str format error\r\n");
				ret=0;
				break;
			}
			*(QSH_VAR_TYPE *)(pRecord->addr)=var_set;//给指定变量赋值	
			printf("%s=%d\r\n",pRecord->name,*(QSH_VAR_TYPE *)(pRecord->addr)); //回显变量值			
		}
		break;
		
		
		case QSCT_GET_BY_ADDR://获得指定地址处的变量或寄存器的值(32位)	
		{
			if(Q_Sh_Msg.ParaNum!=1)//检查参数是否为1个，若不是则说明命令字符串格式不对
			{	
				printf("Cmd read must have only one para!!!\r\n");
				ret=0;
				break;
			}
			if(Q_Sh_Msg.ParaTypes[0]==QSH_FUN_PARA_TYPE_STRING)//第一个参数的类型不能是字符串
			{
				printf("Cmd read's para can not be string\r\n");
				ret=0;
				break;
			}
			if(Q_Sh_Str2num(Q_Sh_StrCut(Q_Sh_Msg.CmdStr,Q_Sh_Msg.ParaStart[0],Q_Sh_Msg.ParaEnd[0]),&var_addr)!=0)//获得地址值
			{
				printf("num str format error\r\n");
				ret=0;
				break;
			}
			printf("*(0x%x)=0x%x\r\n",var_addr,*(QSH_VAR_TYPE *)var_addr);//打印变量值
		}
		break;
		
		case QSCT_SET_BY_ADDR://赋予指定地址处的变量或寄存器指定值(32位)	
		{
			if(Q_Sh_Msg.ParaNum!=2)//检查参数是否为2个，若不是则说明命令字符串格式不对
			{	
				printf("Cmd write must have two para!!!\r\n");
				ret=0;
				break;
			}
			if(Q_Sh_Msg.ParaTypes[0]==QSH_FUN_PARA_TYPE_STRING)//第一个参数的类型不能是字符串
			{
				printf("Cmd write's first para can not be string\r\n");
				ret=0;
				break;
			}
			if(Q_Sh_Msg.ParaTypes[1]==QSH_FUN_PARA_TYPE_STRING)//第二个参数的类型不能是字符串
			{
				printf("Cmd write's second para can not be string\r\n");
				ret=0;
				break;
			}
			if(Q_Sh_Str2num(Q_Sh_StrCut(Q_Sh_Msg.CmdStr,Q_Sh_Msg.ParaStart[0],Q_Sh_Msg.ParaEnd[0]),&var_addr)!=0)//获得地址值
			{
				printf("num str format error\r\n");
				ret=0;
				break;
			}
			if(Q_Sh_Str2num(Q_Sh_StrCut(Q_Sh_Msg.CmdStr,Q_Sh_Msg.ParaStart[1],Q_Sh_Msg.ParaEnd[1]),&var_set)!=0)//获得设置值
			{
				printf("num str format error\r\n");
				ret=0;
				break;
			}
			*(QSH_VAR_TYPE *)var_addr=var_set;
			printf("*(0x%x)=0x%x\r\n",var_addr,*(QSH_VAR_TYPE *)var_addr);//回显变量值
		}
		break;
		
		case QSCT_LIST_FUN://列出所有可被shell访问的函数的描述信息
		{
			Q_Sh_ListFun();
		}
		break;
		
		case QSCT_LIST_VAR://列出所有可被shell访问的变量的描述信息
		{
			Q_Sh_ListVar();
		}
		break;
		
		default:
		{	
			printf("shell error\r\n");
			ret=0;
		}
	}
	return ret;
}
#else
void Q_Shell_Init(void)
{}

unsigned int Q_Sh_CmdHandler(unsigned int IfCtrl,char *Cmd)
{
	return 1;
}
#endif
