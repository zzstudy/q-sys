#include "System.h"
#include "Q_Heap.h"

//概述:这是一个简单的堆管理机制
//其主要思想是将内存分配为若干个最小单元，申请内存时，会分配整数个最小单元的内存
//因为将内存分配为最小单元了，所以用来记录内存块索引的变量只需要16位即可
//如果最小单元是4byte，那么此代码可以管理的堆大小为0xffff的4倍
//但实际上，16bit里面，最高1位用来表示内存块是否被使用，所以实际大小是0x7fff的4倍
//此堆管理机制的额外开销有2部分
// 1.需要一个记录表用来记录内存块信息(定义类型HEAP_RECORD)，每一块内存对应一个记录表成员
//   这个记录表的大小MAX_RECORD_NUM可以根据实际情况修改
// 2.当申请的空间不是最小单元的整数倍+2时，会有多余的内存消耗，最后2个字节用于存放防冲标记。
//   例如当最小单元是4byte，如果申请大小为7，则实际分配的内存是12，为4的整数倍。
//在堆分配时，此机制不负责将申请空间清零
//该堆管理机制没有解决的问题:内存碎片

#if HEAP_TRACK_DEBUG == 1
#define Need_Debug 1
#else
#define Need_Debug 0
#endif

#if Need_Debug
#define Heap_Debug Debug
#else
#define Heap_Debug(x,y...)
#endif

//为保证多线程下堆分配的原子操作，必须借助操作系统临界区
//如果此机制不被用于多线程，则可以定义下面三个个宏为空白
#define Q_HEAP_CRIT_SAVE_DATA_DEFINE CPU_SR cpu_sr
#define Q_HEAP_ENTER_CRIT OS_EnterCritical()
#define Q_HEAP_EXIT_CRIT OS_ExitCritical()

//用来验证内存是否被冲毁的标识
//占用2个字节，所以内存前后被冲毁而不能察觉的概率为65535分之1
#define HEAP_UNIT_REWRITE_ID 0xa55a

#define HEAP_SIZE_BYTE (24*1024)//堆栈总大小，因为只用16bit表示内存块索引，所以实际大小必须小于(0xffff*内存单元大小)

static struct
{
	u32 dDummy;//align
	u16 Heap[ HEAP_SIZE_BYTE/2 ];//mem
}H;

//下面两个宏必须同时修改
#define HEAP_MINI_UNIT 4 //内存单元大小，单位Byte，考虑到32位处理器的4字节对齐，此值必须为4倍数
#define HEAP_UNIT_OFFSET 2 // 2的HEAP_UNIT_OFFSET次方必须等于HEAP_MINI_UNIT

#define UNIT_MAX_NUM (HEAP_SIZE_BYTE/HEAP_MINI_UNIT)//单元个数

//内存块索引
#define UnitIdxMask 0x7fff //用低15位表示内存块的索引，以单元为单位
#define GetUnitIdx(x) (HeapRecord[x].UnitIdx&UnitIdxMask)//x为记录表序号，返回内存块的第一个单元索引
#define SetUnitIdx(x,Index) {HeapRecord[x].UnitIdx=((HeapRecord[x].UnitIdx&(~UnitIdxMask))|((Index)&UnitIdxMask));}
#define GetUnitSize(x) (GetUnitIdx(GetNext((x)))-GetUnitIdx((x)))//x为记录表序号，返回内存块占用单元数
//#define UnitIdxEnd UNIT_MAX_NUM

//操作指定块的读标志，入口参数x为记录表序号
#define UsedMask 0x8000
#define UNUSE 0x0000
#define USED 0x8000
#define GetUsedFlag(x) (HeapRecord[x].UnitIdx&(UsedMask))//返回非0表示被使用，返回0表示空闲
#define SetUsedFlag(x,v) {HeapRecord[x].UnitIdx=((HeapRecord[x].UnitIdx&(~UsedMask))|(v?USED:UNUSE));}

//操作链表，根据记录表序号获取下一个块对应记录表的序号
#define GetNext(x) (HeapRecord[x].Next)
#define SetNext(x,v) {HeapRecord[x].Next=(v);}
#define GetPrev(x) (HeapRecord[x].Prev)
#define SetPrev(x,v) {HeapRecord[x].Prev=(v);}

//清除指定块的记录
#define CleanRecord(x) {HeapRecord[x].Prev=HeapRecord[x].Next=HeapRecord[x].UnitIdx=0;}

//For Track
#if HEAP_TRACK_DEBUG == 1
#define SetRecdName(x,p) HeapRecord[x].pFuncName=p
#define SetRecdLines(x,s) HeapRecord[x].Lines=s
#define GetRecdName(x) HeapRecord[x].pFuncName
#define GetRecdLines(x) HeapRecord[x].Lines
#else
#define SetRecdName(x,p)
#define SetRecdLines(x,s)
#define GetRecdName(x)
#define GetRecdLines(x)
#endif

//单元和字节的转换
#define Unit2Byte(U) ((U)<<HEAP_UNIT_OFFSET) //转换单元数到字节数,用位移代替乘法
#define Byte2Unit(B) ((B)?((((B)-1)>>HEAP_UNIT_OFFSET)+1):0) //转换字节数到占用单元数，用位移代替除法

typedef struct {
	u8 Prev;//记录上一个内存块的记录表序号
	u8 Next;//记录下一个内存块的记录表序号
	u16 UnitIdx;//记录内存块起始单元位置
#if HEAP_TRACK_DEBUG == 1
	u8 *pFuncName;
	u32 Lines;
#endif
}HEAP_RECORD;
#define MAX_RECORD_NUM 256
#define RECORD_START 0
#define RECORD_END (MAX_RECORD_NUM-1)
static HEAP_RECORD HeapRecord[MAX_RECORD_NUM];

//用于调试
void DebugHeap(void)
{
	u16 Index;//表示块的起始单元
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;
	
	Debug("--------------Heap Record--0x%08x----R:0x%08x-0x%08x--------\n\r",(u32)H.Heap,(u32)HeapRecord,(u32)HeapRecord+MAX_RECORD_NUM*sizeof(HEAP_RECORD));
	Q_HEAP_ENTER_CRIT;
	for(Index=RECORD_START;;Index=GetNext(Index))
	{
		Debug("%3d:Used %d,Unit 0x%04x-0x%04x, Addr 0x%08x-0x%08x, Size %5dBytes, Prev %3d,Next %3d",
			Index,GetUsedFlag(Index)?1:0,
			GetUnitIdx(Index),GetUnitIdx(GetNext(Index)),
			(u32)H.Heap+Unit2Byte(GetUnitIdx(Index)),(u32)H.Heap+Unit2Byte(GetUnitIdx(GetNext(Index))),
			Unit2Byte(GetUnitSize(Index)),
			GetPrev(Index),GetNext(Index));
#if HEAP_TRACK_DEBUG == 1
		Debug(",Function %s@%d",GetRecdName(Index),GetRecdLines(Index));
#endif
		Debug("\n\r");

		if(GetNext(Index)==RECORD_END) break;
	}
	Q_HEAP_EXIT_CRIT;
	Debug("--------------Heap Record End--0x%08x---------\n\r",(u32)H.Heap+HEAP_SIZE_BYTE);
}

void QS_HeapInit(void)
{
	u16 Index;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	if( (Unit2Byte(1)!=HEAP_MINI_UNIT)||(Byte2Unit(HEAP_MINI_UNIT)!=1) )
	{
		Debug("HEAP_MINI_UNIT & HEAP_UNIT_OFFSET Marco Define Error!!!\n\r");
		while(1);
	}
		
	Q_HEAP_ENTER_CRIT;
	for(Index=0;Index<(sizeof(HeapRecord)/sizeof(HEAP_RECORD));Index++)
	{
		CleanRecord(Index);
	}

	//建立头记录
	SetPrev(RECORD_START,RECORD_END);
	SetNext(RECORD_START,RECORD_END);
	SetUsedFlag(RECORD_START,UNUSE);
	SetUnitIdx(RECORD_START,0);
#if HEAP_TRACK_DEBUG == 1
	SetRecdName(RECORD_START,"<Idle>");
	SetRecdLines(RECORD_START,0);
#endif

	//建立尾记录
	SetPrev(RECORD_END,RECORD_START);
	SetNext(RECORD_END,RECORD_START);
	SetUsedFlag(RECORD_END,USED);
	SetUnitIdx(RECORD_END,UNIT_MAX_NUM);
#if HEAP_TRACK_DEBUG == 1
	SetRecdName(RECORD_END,"<Idle>");
	SetRecdLines(RECORD_END,0);
#endif

	//建立冲内存检查的头标记
	H.dDummy=HEAP_UNIT_REWRITE_ID;
	H.dDummy<<=16;
	
	DebugHeap();
	Q_HEAP_EXIT_CRIT;
}

//插入一个存储块记录到记录表中
//Index:将插入到此记录,
//Index必须是个记录空内存的记录块
static bool InsertRecord(u16 Index,u16 NeedUnit)
{	
	u16 i;

	//Heap_Debug("##Insert UnitNum 0x%x @ %d\n\r",NeedUnit,Index);

	if(NeedUnit<GetUnitSize(Index))//需要的空间小于空闲空间
	{
		//重新建立一个内存记录块
		for(i=(RECORD_START+1);i<RECORD_END;i++)
		{
			if(GetUnitIdx(i)==0)
			{
				//设置新内存块，这是一个空闲块
				SetPrev(i,Index);//prev设置
				SetNext(i,GetNext(Index));//next转移
				SetUnitIdx(i,GetUnitIdx(Index)+NeedUnit);
				SetUsedFlag(i,UNUSE);

				//处理原本空闲的内存块之后的内存块
				SetPrev(GetNext(Index),i);
				
				//处理要使用的内存块
				SetUsedFlag(Index,USED);//标记使用标志
				SetNext(Index,i);//next转移
				
#if Need_Debug
				//DebugHeap();
#endif
				return TRUE;
			}
		}
		
		if(i==RECORD_END)
		{
			Debug("!!!Record Num is not enough!!!\n\r");
		}
		return FALSE;
	}
	else //如果需要插入的内存块和空闲内存块刚好相等
	{
		SetUsedFlag(Index,USED);//标记被使用
		return TRUE;
	}
}

//UnitIdx为起始单元索引，以HEAP_MINI_UNIT为单位
//返回0表示错误，否则返回free的内存块大小
#if HEAP_TRACK_DEBUG == 1
static u16 DeleteRecord(u16 UnitIdx,u8 *pFuncName,u32 Lines)
#else
static u16 DeleteRecord(u16 UnitIdx)
#endif
{
	u16 Index,Size=0;
	u16 i;

	//Debug("DeleteRecord Unit 0x%x\n\r",UnitIdx);

	for(Index=RECORD_START;;Index=GetNext(Index))
	{
		if(GetUnitIdx(Index)==UnitIdx)//找到匹配项了
		{
#if HEAP_TRACK_DEBUG == 1
			Debug("##Free   Func:%s Line:%d (For %s %d)\n\r",pFuncName,Lines,GetRecdName(Index),GetRecdLines(Index));
			SetRecdName(Index,"<Idle>");
			SetRecdLines(Index,0);
#endif
			Size=Unit2Byte(GetUnitSize(Index));//获取内存块大小
			
			SetUsedFlag(Index,UNUSE);//设置当前为未使用
			
			//先看前面一个是不是空的
			i=GetPrev(Index);
			if(GetUsedFlag(i)==UNUSE)//前面一个是空的，合并前面一个
			{
				SetNext(i,GetNext(Index));
				SetPrev(GetNext(Index),i);
				CleanRecord(Index);		
				Index=i;
			}

			//再看后面一个是不是空的
			i=GetNext(Index);
			if(GetUsedFlag(i)==UNUSE)
			{
				SetNext(Index,GetNext(i));
				SetPrev(GetNext(i),Index);
				CleanRecord(i);
			}

#if Need_Debug
			//DebugHeap();
#endif
			return Size;
		}
		
		if(GetNext(Index)==RECORD_END) break;//到了最后一个块了，退出。
	}

	return 0;	
}

#if HEAP_TRACK_DEBUG == 1
void *QS_Mallco(u16 Size,u8 *pFuncName,u32 Lines)
#else
void *QS_Mallco(u16 Size)
#endif
{
	u16 UnitNum,Index;
	u16 *Mem=NULL;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	if(Size==0) return NULL;
	
	UnitNum=Byte2Unit(Size);//计算要几个单元
	
	//多加一个单元，用来验证内存头尾是否被冲毁。
	//如果Size本来就比最小单元的整数倍小2或者小3，就不用多余的一个单元了。
	Size%=HEAP_MINI_UNIT;
	if(Size==0||(HEAP_MINI_UNIT-Size)<2) UnitNum++;

	Q_HEAP_ENTER_CRIT;
	for(Index=RECORD_START;;Index=GetNext(Index))
	{
		if(GetUsedFlag(Index)==UNUSE)//找到一个空的单元
		{
			if(GetUnitSize(Index)>=UnitNum)
			{
				if(InsertRecord(Index,UnitNum)==TRUE)
				{
					Mem=(void *)((u32)H.Heap+Unit2Byte(GetUnitIdx(Index)));
					Mem[(Unit2Byte(UnitNum)>>1)-1]=HEAP_UNIT_REWRITE_ID;
					Heap_Debug("##Mallco Size %d, Addr 0x%x,Unit 0x%x\n\r",Unit2Byte(UnitNum),(u32)Mem,GetUnitIdx(Index));
#if HEAP_TRACK_DEBUG == 1
					SetRecdName(Index,pFuncName);
					SetRecdLines(Index,Lines);
					Debug("##Mallco Func:%s Line:%d\n\n\r",pFuncName,Lines);
#endif
					Q_HEAP_EXIT_CRIT;		
					return (void *)Mem;
				}
				else 
					break;
			}
		}			
		
		if(GetNext(Index)==RECORD_END) 
			break;
	}
	Q_HEAP_EXIT_CRIT;
	
	DebugHeap();
	Debug("!!!No Get Heap!!!\n\r");
	return 0;
//	OS_SchedLock();
//	while(1);
}

#if HEAP_TRACK_DEBUG == 1
bool QS_Free(void *Ptr,u8 *pFuncName,u32 Lines)
#else
bool QS_Free(void *Ptr)
#endif
{	
	u16 *Mem=Ptr;
	u16 Size;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	if(Ptr)
	{
		Q_HEAP_ENTER_CRIT;

#if HEAP_TRACK_DEBUG == 1
		Size=DeleteRecord(Byte2Unit((u32)Ptr-(u32)H.Heap),pFuncName,Lines);
#else
		Size=DeleteRecord(Byte2Unit((u32)Ptr-(u32)H.Heap));
#endif

		if(Size==0)
		{
			Debug("##Free Error!!!Can not find the match memory!!!0x%x\n\r",(u32)Ptr);
#if HEAP_TRACK_DEBUG == 1
			Debug("##Free   Func:%s Line:%d\n\r",pFuncName,Lines);
#endif
			while(1);
		}
		else 
			Heap_Debug("##Free   Size %d, Addr 0x%x,Unit 0x%x\n\n\r",Size,(u32)Ptr,Byte2Unit((u32)Ptr-(u32)H.Heap));
			
		//检查内存冲毁情况
		if(Mem[(Size>>1)-1]!=HEAP_UNIT_REWRITE_ID)
		{
			Debug("Memory end be rewrited!!!%x %d\n\r",Mem[(Size>>1)-1],Size);
#if HEAP_TRACK_DEBUG == 1
			Debug("##Free   Func:%s Line:%d\n\r",pFuncName,Lines);
#endif
			while(1);
		}

		Mem--;
		if(Mem[0]!=HEAP_UNIT_REWRITE_ID)
		{
			Debug("Memory header be rewrited!!!%x\n\r",Mem[0]);
#if HEAP_TRACK_DEBUG == 1
			Debug("##FuncName:%s Lines:%d\n\r",pFuncName,Lines);
#endif
			while(1);
		}
				
		Q_HEAP_EXIT_CRIT;
		return TRUE;
	}
	else
		return FALSE;
}

//用于内存碎片监控
/*为了描述内存碎片有多少，引入了一个浮点数fragindices来描述它：
fragindices被定义为：
fragindices = (float)MaxSize/(float)TotalSize
其中MaxSize表示堆中最大可分配的连续空闲内存大小
TotalSize表示堆中所有空闲内存的总和
可见当系统无碎片时fragindices = 1
而当系统无内存可分配时fragindices = 0 
fragindices越小表示碎片越多*/
void QS_MonitorFragment(void)
{
	u16 Index,Size,MaxSize=0,TotalSize=0;
	
	for(Index=RECORD_START;;Index=GetNext(Index))
	{ 
		if(GetUsedFlag(Index)==UNUSE)
		{
			Size=Unit2Byte(GetUnitSize(Index));//获取内存块大小
			TotalSize+=Size;
			if(Size>MaxSize)
				MaxSize=Size;
		}
		if(GetNext(Index)==RECORD_END) break;
	}

	Debug("********** Heap Monitor **********\n\r");
	Debug(" TotalFreeMem=%d Byte\n\r",TotalSize);
	Debug(" MaxFreeMem=%d Byte\n\r",MaxSize);
	Debug(" Fragindices=%.2f\n\r",(float)MaxSize/(float)TotalSize);
	Debug("**********************************\n\r");
}


//用于整理内存碎片
bool QS_HeapArrange(void)
{//unfinish
	return TRUE;
}


