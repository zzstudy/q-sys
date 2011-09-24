#ifndef QSYS_Q_SHELL_H  
#define QSYS_Q_SHELL_H

#define USE_Q_SHELL_FUNCTION 1//选择是否使用Q_Shell组件 1:使用 0:不使用

#if USE_Q_SHELL_FUNCTION
/*-----------------------------------------------------------------------
简述： 注册对象的记录类型
注  ：
	  1.每个注册的对象唯一的映射到一个记录
	  2.记录是放在ROM中的，不占用RAM
	  3.函数和变量的记录信息被放在两个不同的段中
	    函数的注册信息放在qShellFunTab段中，由QSH_FUN_REG生成
		变量的注册信息放在qShellVarTab段中，由QSH_VAR_REG生成
------------------------------------------------------------------------*/
typedef const struct{
	const char*		name;		//函数或变量名
	const char*		desc;		//函数或变量描述
	void *          addr;	    //函数或变量地址
}QSH_RECORD;

/*-----------------------------------------------------------------------
宏功能：将一个函数注册到qShellFunTab段中
入参    name 函数名   (无需加引号，下同)
		desc 函数描述
返回  ：无
使用举例:
unsigned int CheckStackUsed(char *task,unsigned char ifdisplay)
{
	return stack_usedbytes;
}
QSH_FUN_REG(CheckStackUsed,unsigned int CheckStackUsed(char *task,unsigned char ifdisplay))		
------------------------------------------------------------------------*/
#define QSH_FUN_REG(name, desc)					                                           \
const   char  qsh_fun_##__FILE__##_name##_name[]  = #name;				                   \
const   char  qsh_fun_##__FILE__##_name##_desc[]  = #desc;						           \
QSH_RECORD qsh_fun_##__FILE__##_name##_record  __attribute__((section("qShellFunTab"))) =  \
{							                                                               \
	qsh_fun_##__FILE__##_name##_name,	                                                   \
	qsh_fun_##__FILE__##_name##_desc,	                                                   \
	(void *)&name		                                                                   \
};

/*-----------------------------------------------------------------------
宏功能：将一个变量注册到qShellVarTab段中
入参    name 变量名   (无需加引号，下同)
		desc 变量描述
返回    无
使用举例:
unsigned int stack_usedbytes;
QSH_VAR_REG(stack_usedbytes,unsigned int stack_usedbytes)		
------------------------------------------------------------------------*/
#define QSH_VAR_REG(name, desc)					                                           \
const   char  qsh_var_##__FILE__##_name##_name[] = #name;				                   \
const   char  qsh_var_##__FILE__##_name##_desc[] = #desc;				                   \
QSH_RECORD qsh_var_##__FILE__##_name##_record  __attribute__((section("qShellVarTab"))) =  \
{							                                                               \
	qsh_var_##__FILE__##_name##_name,	                                                   \
	qsh_var_##__FILE__##_name##_desc,	                                                   \
	(void *)&name		                                                                   \
};

/*-----------------------------------------------------------------------
函数功能：shell初始化 包括初始化shell函数和初始化shell变量
入参：无
返回：无
注：  使用shell前必须先调用此函数初始化
------------------------------------------------------------------------*/
void Q_Shell_Init(void);

/*-----------------------------------------------------------------------
函数功能：执行命令
入参    
        ifctrl 0:接收到控制字符
			  !0:接收到命令字符串
		cmd    命令字符串
返回    ：成功 1
		 失败 0  
------------------------------------------------------------------------*/
unsigned int Q_Sh_CmdHandler(unsigned int ifctrl,char *cmd);

#else
#define QSH_FUN_REG(name, desc)
#define QSH_VAR_REG(name, desc)
void Q_Shell_Init(void);
unsigned int Q_Sh_CmdHandler(unsigned int ifctrl,char *cmd);
#endif

#endif
