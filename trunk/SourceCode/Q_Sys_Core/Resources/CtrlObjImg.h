#ifndef QSYS_CTRL_OBJ_IMG_H
#define QSYS_CTRL_OBJ_IMG_H


//yes or no
#define CO_YES_NO_W	58
#define CO_YES_NO_H	18
#define CO_YES_NO_TRAN_COLOR	FatColor(0x0000ff)
extern const unsigned char gCtrlObj_On[];
extern const unsigned char gCtrlObj_Off[];
extern const unsigned char gCtrlObj_OnOff[];

//num box
#define CO_NUM_BOX_H 18
#define CO_NUM_BOX_FRAME_W 4
#define CO_NUM_BOX_ARROW_W 18
#define CO_NUM_BOX_MIDDLE_W	1
#define CO_NUM_BOX_TRAN_COLOR FatColor(0xff0000)
#define CO_NUM_BOX_FONT_STYLE 	ASC14B_FONT
#define CO_NUM_BOX_FONT_W	8
#define CO_NUM_BOX_FONT_SPACE 0
#define CO_NUM_BOX_FONT_COLOR FatColor(0x333333)
#define CO_NUM_BOX_FONT_COLOR_H FatColor(0x000000)
extern const unsigned char gCtrlObj_NumBoxLeft[];
extern const unsigned char gCtrlObj_NumBoxLeftArrow[];
extern const unsigned char gCtrlObj_NumBoxMiddle[];
extern const unsigned char gCtrlObj_NumBoxRight[];
extern const unsigned char gCtrlObj_NumBoxRightArrow[];
extern const unsigned char gCtrlObj_NumBoxLeftH[];
extern const unsigned char gCtrlObj_NumBoxLeftArrowH[];
extern const unsigned char gCtrlObj_NumBoxMiddleH[];
extern const unsigned char gCtrlObj_NumBoxRightH[];
extern const unsigned char gCtrlObj_NumBoxRightArrowH[];
#endif

