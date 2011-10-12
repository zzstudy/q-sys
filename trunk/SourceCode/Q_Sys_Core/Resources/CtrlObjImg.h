#ifndef QSYS_CTRL_OBJ_IMG_H
#define QSYS_CTRL_OBJ_IMG_H


//yes no 控件
#define CO_YES_NO_W	58 //yes no控件的宽度
#define CO_YES_NO_H	18 //yes no控件的高度
#define CO_YES_NO_TRAN_COLOR	FatColor(0x0000ff)//yes no控件的透明色
extern const unsigned char gCtrlObj_On[]; //yes no控件的on图片数组
extern const unsigned char gCtrlObj_Off[]; //yes no控件的yes图片数组
extern const unsigned char gCtrlObj_OnOff[]; //yes no控件的yesno数组

//num控件 
#define CO_NUM_H 18 //num控件高度
#define CO_NUM_FRAME_W 4 //num控件左右边框宽度
#define CO_NUM_ARROW_W 18 //num控件箭头宽度
#define CO_NUM_MIDDLE_W	1 //num控件中间填充部分的单位宽度
#define CO_NUM_TRAN_COLOR FatColor(0xff0000) //num控件透明色
#define CO_NUM_FONT_STYLE 	ASC14B_FONT //num控件显示字体
#define CO_NUM_FONT_W	8 //num控件ascii字体宽度
#define CO_NUM_FONT_SPACE 0 //num控件字体间距设置
#define CO_NUM_FONT_COLOR FatColor(0x333333) //num控件字体颜色
#define CO_NUM_FONT_COLOR_H FatColor(0xf1aa00) //num控件高亮字体色
extern const unsigned char gCtrlObj_NumLeft[]; //num控件左边框
extern const unsigned char gCtrlObj_NumLeftArrow[]; //num控件左箭头
extern const unsigned char gCtrlObj_NumMiddle[]; //num控件中间填充
extern const unsigned char gCtrlObj_NumRight[]; //num控件右边框
extern const unsigned char gCtrlObj_NumRightArrow[]; //num控件右箭头
extern const unsigned char gCtrlObj_NumLeftH[]; //num控件左边框高亮
extern const unsigned char gCtrlObj_NumLeftArrowH[]; //num控件左箭头高亮
extern const unsigned char gCtrlObj_NumMiddleH[]; //num控件中间填充高亮
extern const unsigned char gCtrlObj_NumRightH[]; //num控件右边框高亮
extern const unsigned char gCtrlObj_NumRightArrowH[]; //num控件右箭头高亮

#endif

