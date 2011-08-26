#ifndef QSYS_TOUCH_HANDLER_H
#define QSYS_TOUCH_HANDLER_H

typedef struct{
	u16 x;
	u16 y;
	u16 w;
	u16 h;
	u8 ObjID;
	u8 Type;
	u8 Index;
	u8 OptionsMask;
}TOUCH_REGION;
//对于touch handler而言，它只负责对应触摸点到注册的区域中
//如果找到当前触点对应的区域，并且掩码开放，则发送包含类型和索引值的事件给内核

extern u8 gTouchRegionNum;//记录所有注册区域个数的变量
extern TOUCH_REGION *gpTouchRegions;//记录所有注册区域的指针变量


#endif

