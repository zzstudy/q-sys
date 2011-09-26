#ifndef QSYS_CONTROL_OBJ_H
#define QSYS_CONTROL_OBJ_H

typedef enum{
	COT_NULL=0,
	COT_ImgTch,
	COT_CharTch,
	COT_DynImg,
	COT_DynChar,
#ifdef QSYS_FRAME_FULL	
	COT_YesNo,
	COT_NumBox,
	COT_StrOpt,
	COT_StrInput,
#endif
}CTRL_OBJ_TYPE;

typedef struct{
	u8 ImgTchNum;//图片按键数目，大多数页面只用到了图片按钮
	u8 CharTchNum;//文字按键数目，大多数页面只用到了图片按钮，则此值定义为0即可
	u8 DynImgTchNum;//动态图片按键的个数
	u8 DynCharTchNum;//动态文字按键的个数
#ifdef QSYS_FRAME_FULL	
	u8 YesNoNum;//yes or no 选项的个数
	u8 NumBoxNum;//数字框的个数
	u8 StrOptBoxNum;//字符串选项框的个数
	u8 StrInputBoxNum;//字符串输入框的个数
#endif
}PAGE_CONTROL_NUM;//页面的动态控件个数

//4	用于页面的UserEventHandler返回值，旨在告诉系统做一些事情
#define TCH_MSG u32
//bit0-bit15 用于用户自定义返回值
#define TM_RET_MASK 0xffff
//bit16-30用于回传命令给系统，所以只支持15个回传命令
#define TM_CMD_MASK 0x7fff0000
#define TM_CMD_OFFSET 16
#define TM_TouchOff (1<<16)//在TouchEventHandler返回此值，关闭触摸响应
#define TM_TouchOn (1<<17)//在TouchEventHandler返回此值，开启触摸响应
#define TM_ExtiKeyOff (1<<18)//在TouchEventHandler返回此值，关闭外部按键输入
#define TM_ExtiKeyOn (1<<19)//在TouchEventHandler返回此值，开启外部按键输入
//bit31 用于回传状态给系统
#define TM_STATE_MASK 0x80000000
#define TM_STATE_OFFSET 31
#define TM_State_OK 0
#define TM_State_Faile 0x80000000

typedef enum {
	Tch_Press=0,//触摸按下
	Tch_Continue,//保持触摸状态，用于传递长按时的实时坐标
	Tch_Release,//在有效区域触摸松开,有效区域指Press时的已注册区域
	Tch_ReleaseVain,//在非有效区域松开
	Tch_Normal,//用于改变图标，无实际case
}TCH_EVT;//4	传入TouchEventHandler的事件

typedef struct {//sizeof=8
	u16 x;	//触点x坐标
	u16 y;	//触点y坐标
	u16 Id;	//只有按下才会导致id++,所以在页面可以匹配id以判断是不是同一次触碰动作
	u16 TimeStamp;//每个动作的时间，相对值，单位ms
}TOUCH_INFO;//4		传入TouchEventHandler的参数

//第一参数为键值，第二参数为按键事件，第三参数为触摸信息
typedef TCH_MSG (*TouchHandlerFunc)(u8 ,TCH_EVT , TOUCH_INFO *);
#ifdef QSYS_FRAME_FULL	
//第一个参数为键值，第二个参数为当前值，当值变化时触发
typedef TCH_MSG (*YesNoHandlerFunc)(u8 ,bool );
typedef TCH_MSG (*NumBoxHanderFunc)(u8 ,TCH_EVT ,bool ,TOUCH_INFO *);
typedef TCH_MSG (*StrOptBoxHandlerFunc)(u8 ,TCH_EVT ,bool ,TOUCH_INFO *);
typedef TCH_MSG (*StrInputBoxHandlerFunc)(u8 ,TCH_EVT ,bool ,TOUCH_INFO *);
#endif	

//4	按键区域的OptionMask掩码值(最大支持16个掩码)
//注:因为Touch线程和Input线程资源交换有限的缘故，按键事件掩码必须放在低8位
#define PrsMsk (1<<0) //Press事件掩码，用于决定板子是否响应Press事件，以下类同
#define CotMsk (1<<1) //Continue事件掩码
#define RelMsk (1<<2)  //Release事件掩码
#define ReVMsk (1<<3) //ReleaseVain事件掩码

//非按键事件掩码可放高位
#define LandMsk (1<<4) //用于决定按键是否用横屏模式显示文字或者图片
#define BinMsk		(1<<5) //当采用时，不再使用bmp文件，而是使用bin文件画图标，此时图标必须和区域一样大
#define DbgMsk	(1<<6) //调试模式，用于显示触摸区域，图片按键则不会显示图片，只显示区域
#define F16Msk	(1<<7) //改变文字大小为16x16
#define B14Msk	(1<<8) //改变文字为B14粗体，仅对Ascii字符有效
#define PathMsk (1<<9) //该掩码位有效时，BmpPathPrefix指定的字符串代表整个图片的路径前缀；该位无效时，BmpPathPrefix配合theme路径才为图标路径。
#define RoueMsk (1<<10) //Round Edge 圆边
//#define CirbMsk 	(1<<11) //Circular Bead 圆角，暂不支持

typedef struct {
	u8 *Name;//键名，图标按键下用于不能显示图标时的提示。文字按键下用于文字显示。
	
	//键值,用于区别点击的按键
	// 1.小于USER_KEY_VALUE_START的键值被赋予系统使用，用来传递类似于外部按键这样的按键
	// 2.如果按键是图片或者文字性质，则键值范围为USER_KEY_VALUE_START-0xff，注意不显示图标也是属于图片按键类型。
	// 3.多个按键的键值可以相同，包括图片性质和文字性质之间都可以有相同键值的按键。同一键值的按键有连锁反应。
	u8 ObjID;

	//用于定义一些按键的特性，有如下几类
	// 1.按键事件，对于文字型和图片型都有效，用于决定本按键需要响应哪些事件，取值有PrsMsk|CotMsk|RelMsk|ReVMsk
	// 2.横屏选项，对于文字型和图片型都有效，用于决定本按键是否横屏显示，注意设计时的思维转换
	// 3.调试模式，对于文字型和图片型都有效，用于显示触摸区域，方便调试时使用
	// 4.文字大小选项，对于文字型和图片型都有效，用于决定是否采用16x16的文字大小，默认为12x12
	// 5.圆角背景，只对于文字型有效，可让文字背景四脚为圆角
	// 6.圆边背景，只对于文字型有效，可让文字背景左右为圆边
	// 以上所有选项可共存，赋值时使用"|"符号，如PrsMsk|LandMsk|F16Msk
	u16 OptionsMask;
	
	u16 x; 	//触摸区域起点的x值，屏幕左上点为0，0坐标。如果是横屏模式，则需要横过显示屏，指定显示区域的左上点。
	u16 y;	//触摸区域起点的y值
	u16 w; //触摸区域宽度,如果按键是横屏模式,注意横过显示屏来定义
	u16 h;	//触摸区域高度,如果按键是横屏模式,注意横过显示屏来定义	
	
	u16 ImgX;// 触摸区域图片的显示起点x值,相对于区域左上角的相对值
	u16 ImgY;//触摸区域图片的显示起点y值,相对于区域左上角的相对值

	//按键图标路径前缀，为了节省rom空间，1.2版之后，指定
	//三种图标的相同前缀，再指定每种动作对应的后缀字母
	//例如:如果BmpPathprefix="Dir/SubDir/Button",NormalSuffix='N',
	//则得到普通状态按键路径为"Dir/SubDir/ButtonN.bmp"
	u8 *BmpPathPrefix;//最长不要超过60个字符
	//u8 NormalSuffix;//普通状态后缀，单字节，如果指定为0，则不显示
	//u8 PressSuffix;//按下状态后缀，单字节，如果指定为0，则不显示
	//u8 ReleaseSuffix;//释放状态后缀，单字节，如果指定为0，则不显示
	//u8 ReserveU8;//保留
	COLOR_TYPE TransColor; //32位透明色，NO_TRANSP表示不用透明色
}IMG_TCH_OBJ;//4		触摸按键区域定义

typedef struct {
	u8 *Name;//键名，图标按键下用于不能显示图标时的提示。文字按键下用于文字显示。
	
	//键值,用于区别点击的按键
	// 1.小于0x40的键值被赋予系统使用，用来传递类似于外部按键这样的按键
	// 2.如果按键是图片或者文字性质，则键值范围为0x40-0xff，注意不显示图标也是属于图片按键类型。
	// 3.多个按键的键值可以相同，包括图片性质和文字性质之间都可以有相同键值的按键。同一键值的按键有连锁反应。
	u8 ObjID;

	//用于定义一些按键的特性，有如下几类
	// 1.按键事件，对于文字型和图片型都有效，用于决定本按键需要响应哪些事件，取值有PrsMsk|CotMsk|RelMsk|ReVMsk
	// 2.横屏选项，对于文字型和图片型都有效，用于决定本按键是否横屏显示，注意设计时的思维转换
	// 3.调试模式，对于文字型和图片型都有效，用于显示触摸区域，方便调试时使用
	// 4.文字大小选项，对于文字型和图片型都有效，用于决定是否采用16x16的文字大小，默认为12x12
	// 5.圆角背景，只对于文字型有效，可让文字背景四脚为圆角
	// 6.圆边背景，只对于文字型有效，可让文字背景左右为圆边
	// 以上所有选项可共存，赋值时使用"|"符号，如PrsMsk|CotMsk|RelMsk|ReVMsk。
	u16 OptionsMask;
	
	u16 x; 	//触摸区域起点的x值，屏幕左上点为0，0坐标。如果是横屏模式，则需要横过显示屏，指定显示区域的左上点。
	u16 y;	//触摸区域起点的y值
	u16 w; //触摸区域宽度,如果按键是横屏模式,注意横过显示屏来定义
	u16 h;	//触摸区域高度,如果按键是横屏模式,注意横过显示屏来定义	
	
	u8 CharX;// 触摸区域文字的显示起点x值,相对于区域左上角的相对值
	u8 CharY;//触摸区域文字的显示起点y值,相对于区域左上角的相对值
	u8 Margin;//高四位表示背景左右的区域旁白，低四位表示背景上下的区域旁白。留此值是为了让触摸区域大于背景区域。h和w减去对应的Margin等于背景色实际的h和w。
	u8 Space;//高四位表示字间距，低四位表示行间距

	//TRANSPARENT表示使用透明色
	COLOR_TYPE NormalColor;//16位色彩，指定普通状态文字颜色
	COLOR_TYPE NormalBG;//16位色彩，指定普通状态背景色
	COLOR_TYPE PressColor;//16位色彩，指定按下状态文字颜色
	COLOR_TYPE PressBG;//16位色彩，指定按下状态背景色	
	COLOR_TYPE ReleaseColor;//16位色彩，指定释放状态文字颜色
	COLOR_TYPE ReleaseBG;//16位色彩，指定释放状态背景色
}CHAR_TCH_OBJ;//4		触摸按键区域定义

#ifdef QSYS_FRAME_FULL	
typedef struct {		
	u8 ObjID;//标识符，页面内必须唯一，以区分其他控件对象

	bool DefVal;//默认值，TRUE或者FALSE
	
	u16 x; 	//触摸区域起点的x值，屏幕左上点为0，0坐标。如果是横屏模式，则需要横过显示屏，指定显示区域的左上点。
	u16 y;	//触摸区域起点的y值
}YES_NO_OBJ;//4		yes no控件定义

typedef enum{
	NBT_Num,
	NBT_NumList,
	NBT_NumEnum,
}NUM_BOX_TYPE;

typedef struct{
	u8 ObjID;//标识符，页面内必须唯一，以区分其他控件对象
	NUM_BOX_TYPE Type;//num box类型

	u16 x; 	//触摸区域起点的x值，屏幕左上点为0，0坐标。如果是横屏模式，则需要横过显示屏，指定显示区域的左上点。
	u16 y;	//触摸区域起点的y值
	u16 w; //触摸区域宽度,如果按键是横屏模式,注意横过显示屏来定义
	
	s32 Value;
}NUM_BOX_OBJ;//4		数字输入框

typedef struct{
	u8 ObjID;//标识符，页面内必须唯一，以区分其他控件对象
	NUM_BOX_TYPE Type;//num box类型

	u16 x; 	//触摸区域起点的x值，屏幕左上点为0，0坐标。如果是横屏模式，则需要横过显示屏，指定显示区域的左上点。
	u16 y;	//触摸区域起点的y值
	u16 w; //触摸区域宽度,如果按键是横屏模式,注意横过显示屏来定义

	s32 Value;
	s32 Max;
	s32 Min;
	s32 Step;
}NUM_LIST_BOX_OBJ;//4		数字输入框

typedef struct{
	u8 ObjID;//标识符，页面内必须唯一，以区分其他控件对象
	NUM_BOX_TYPE Type;//num box类型

	u16 x; 	//触摸区域起点的x值，屏幕左上点为0，0坐标。如果是横屏模式，则需要横过显示屏，指定显示区域的左上点。
	u16 y;	//触摸区域起点的y值
	u16 w; //触摸区域宽度,如果按键是横屏模式,注意横过显示屏来定义

	u8 Idx;//当前值索引
	u8 Num;//当前值个数
	u8 Total;//可容纳总数
	s32 EnumList[1];
}NUM_ENUM_BOX_OBJ;//4		数字输入框

typedef struct{
	u8 ObjID;//标识符，页面内必须唯一，以区分其他控件对象
}STR_OPT_OBJ;//4		字符串选项框

typedef struct{
	u8 ObjID;//标识符，页面内必须唯一，以区分其他控件对象
}STR_INPUT_OBJ;//4		字符串输入框
#endif

typedef u8 REP_IMG_SUFX;

//api

//更改当前触摸域某个按键的显示图片
// 1.只对当前页面有效,转换页面或子页面后失效
// 2.只能指定后缀，比如原来的资源图标是"MusicN.bmp"
//    指定Suffix='T'，则图标资源变成"MusicT.bmp"
//如果Suffix=0，则恢复原始图标
void Q_ChangeImgTchImg(u8 Key,u8 Suffix);

//和ChangeKeyImg相对，读取当前的图标替换后缀值
//返回0表示是默认值
//否则返回替换的后缀
u8 Q_ReadImgTchImg(u8 Key);

//更改当前触摸域某个文字按键的显示文字
// 1.只对当前页面有效,转换页面或子页面后失效
//如果NewName=NULL，则恢复原始文字
void Q_ChangeCharTchName(u8 Key,u8 *NewName);

//和ChangeKeyName相对，读取当前的图标替换后缀值
//返回NULL表示是默认值或者key超出范围
u8 *Q_ReadCharTchName(u8 Key);

//立刻呈现指定按键键值的图标或文字，对所有此键值的按键均有效
void Q_PresentTch(u8 Key,TCH_EVT Type);

//用于设置新的动态图标按键
bool Q_SetDynamicImgTch(u8 Idx,IMG_TCH_OBJ *pTchReg);

//用于设置新的动态文字按键
bool Q_SetDynamicCharTch(u8 Idx,CHAR_TCH_OBJ *pTchReg);

#ifdef QSYS_FRAME_FULL
bool Q_SetYesNo(u8 Idx,YES_NO_OBJ *pYesNo);
#endif

#endif

