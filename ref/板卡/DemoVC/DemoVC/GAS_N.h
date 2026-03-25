#pragma once

#include <stddef.h>

#pragma pack(push)
#pragma pack(2)

#define MAX_MACRO_CHAR_LENGTH (128)

//函数执行返回值
#define GA_COM_SUCCESS			        (0)	//执行成功
#define GA_COM_ERR_EXEC_FAIL			(1)	//执行失败
#define GA_COM_ERR_LICENSE_WRONG		(2)	//license不支持
#define GA_COM_ERR_DATA_WORRY			(7)	//参数错误
#define GA_COM_ERR_SEND					(-1)//发送失败
#define GA_COM_ERR_CARD_OPEN_FAIL		(-6)//打开失败
#define GA_COM_ERR_TIME_OUT				(-7)//无响应
#define GA_COM_ERR_COM_OPEN_FAIL        (-8)//打开串口失败

//轴状态位定义
#define AXIS_STATUS_ESTOP               (0x00000001)	//急停
#define AXIS_STATUS_SV_ALARM            (0x00000002)	//驱动器报警标志（1-伺服有报警，0-伺服无报警）
#define AXIS_STATUS_POS_SOFT_LIMIT      (0x00000004)	//正软限位触发标志（规划位置大于正向软限位时置1）
#define AXIS_STATUS_NEG_SOFT_LIMIT      (0x00000008)	//负软位触发标志（规划位置小于负向软限位时置1）
#define	AXIS_STATUS_FOLLOW_ERR          (0x00000010)	//轴规划位置和实际位置的误差大于设定极限时置1┅
#define AXIS_STATUS_POS_HARD_LIMIT      (0x00000020)	//正硬限位触发标志（正限位开关电平状态为限位触发电平时置1）
#define AXIS_STATUS_NEG_HARD_LIMIT      (0x00000040)	//负硬限位触发标志（负限位开关电平状态为限位触发电平时置1）
#define AXIS_STATUS_IO_SMS_STOP         (0x00000080)	//IO平滑停止触发标志（正限位开关电平状态为限位触发电平时置1，规划位置大于正向软限位时置1）
#define AXIS_STATUS_IO_EMG_STOP         (0x00000100)	//IO紧急停止触发标志（负限位开关电平状态为限位触发电平时置1，规划位置小于负向软限位时置1）
#define AXIS_STATUS_ENABLE              (0x00000200)	//电机使能标志
#define	AXIS_STATUS_RUNNING             (0x00000400)	//规划运动标志，规划器运动时置1
#define AXIS_STATUS_ARRIVE              (0x00000800)	//电机到位（规划器静止，规划位置和实际位置的误差小于设定误差带，并在误差带内保持设定时间后，置起到位标志）
#define AXIS_STATUS_HOME_RUNNING        (0x00001000)	//正在回零
#define AXIS_STATUS_HOME_SUCESS	        (0x00002000)	//回零成功
#define AXIS_STATUS_HOME_SWITCH			(0x00004000)	//零位信号
#define AXIS_STATUS_INDEX				(0x00008000)    //z索引信号
#define AXIS_STATUS_GEAR_START  		(0x00010000)    //电子齿轮开始啮合
#define AXIS_STATUS_GEAR_FINISH         (0x00020000)    //电子齿轮完成啮合

//坐标系状态位定义
#define	CRDSYS_STATUS_PROG_RUN						(0x00000001)	//启动中
#define	CRDSYS_STATUS_PROG_STOP						(0x00000002)	//平滑停止中
#define	CRDSYS_STATUS_PROG_ESTOP					(0x00000004)	//紧急停止中

#define	CRDSYS_STATUS_FIFO_FINISH_0		    (0x00000010)	//板卡FIFO-0数据已执行完毕的状态位
#define	CRDSYS_STATUS_FIFO_FINISH_1		    (0x00000020)	//板卡FIFO-1数据已执行完毕的状态位

//输入IO类型宏定义
#define MC_LIMIT_POSITIVE               0
#define MC_LIMIT_NEGATIVE               1
#define MC_ALARM                        2
#define MC_HOME                         3
#define MC_GPI                          4
#define MC_ARRIVE                       5
#define MC_IP_SWITCH                    6
#define MC_MPG                          7

//输出IO类型宏定义
#define MC_ENABLE                       10
#define MC_CLEAR                        11
#define MC_GPO                          12


//高速捕获输入类型宏定义
#define CAPTURE_HOME                    1
#define CAPTURE_INDEX                   2
#define CAPTURE_PROBE1                  3
#define CAPTURE_PROBE2                  4

//PT模式宏定义
#define PT_MODE_STATIC                  0
#define PT_MODE_DYNAMIC                 1

#define PT_SEGMENT_NORMAL               0
#define PT_SEGMENT_EVEN                 1
#define PT_SEGMENT_STOP                 2

#define GEAR_MASTER_ENCODER             1
#define GEAR_MASTER_PROFILE             2
#define GEAR_MASTER_AXIS                3


//电子齿轮启动事件定义
#define GEAR_EVENT_IMMED                1//立即启动电子齿轮
#define GEAR_EVENT_BIG_EQU              2//主轴规划或者编码器位置大于等于指定数值时启动电子齿轮
#define GEAR_EVENT_SMALL_EQU            3//主轴规划或者编码器位置小于等于指定数值时启动电子齿轮
#define GEAR_EVENT_IO_ON                4//指定IO为ON时启动电子齿轮
#define GEAR_EVENT_IO_OFF               5//指定IO为OFF时启动电子齿轮


#define FROCAST_LEN (200)                     //前瞻缓冲区深度

#define INTERPOLATION_AXIS_MAX          6
#define CRD_FIFO_MAX                    4096
#define CRD_MAX                         2


//点位模式参数结构体
typedef struct TrapPrm
{
	double acc;//加速度
	double dec;//减速度
	double velStart;//起始速度
	short  smoothTime;//平滑时间
}TTrapPrm;

//JOG模式参数结构体
typedef struct JogPrm
{
	double dAcc;//加速度
	double dDec;//减速度
	double dSmooth;//平滑时间
}TJogPrm;



//坐标系参数结构体
typedef struct _CrdPrm
{
    short dimension;                              // 坐标系维数
    short profile[8];                             // 关联profile和坐标轴(从1开始)
    double synVelMax;                             // 最大合成速度
    double synAccMax;                             // 最大合成加速度
    short evenTime;                               // 最小匀速时间
    short setOriginFlag;                          // 设置原点坐标值标志,0:默认当前规划位置为原点位置;1:用户指定原点位置
    long originPos[8];                            // 用户指定的原点位置
}TCrdPrm;

//命令类型
enum _CMD_TYPE
{
	CMD_G00=1,		//快速定位
	CMD_G01,		//直线插补
	CMD_G02,		//顺圆弧插补
	CMD_G03,		//逆圆弧插补
	CMD_G04,		//延时,G04 P1000是暂停1秒(单位为ms),G04 X2.0是暂停2秒
	CMD_G05,		//设置自定义插补段段号
	CMD_G54,

	CMD_M00 = 11,  //暂停
	CMD_M30,        //结束
	CMD_M31,        //切换到XY1Z坐标系
	CMD_M32,        //切换到XY2Z坐标系
	CMD_M99,        //循环

	CMD_SET_IO = 101,     //设置IO
	CMD_WAIT_IO,           //等待IO
	CMD_BUFFER_MOVE_SET_POS,      //CMD_BUFFER_MOVE_SET_POS
	CMD_BUFFER_MOVE_SET_VEL,      //CMD_BUFFER_MOVE_SET_VEL
	CMD_BUFFER_MOVE_SET_ACC,      //CMD_BUFFER_MOVE_SET_ACC
	CMD_BUFFER_GEAR,      //BUFFER_GEAR
};


//G00(快速定位)命令参数
struct _G00PARA{
	float synVel; //插补段合成速度
	float synAcc; //插补段合成加速度
    long lX;       //X轴到达位置绝对位置(单位：pluse)
    long lY;       //Y轴到达位置绝对位置(单位：pluse)
    long lZ;       //Z轴到达位置绝对位置(单位：pluse)
    long lA;       //A轴到达位置绝对位置(单位：pluse)
   
	unsigned char iDimension; //参与插补的轴数量
	long segNum;
};
//G01(直线插补)命令参数(任意2到3轴，上位机保证)
struct _G01PARA{
	float synVel;    //插补段合成速度
	float synAcc;    //插补段合成加速度
	float velEnd;   //插补段的终点速度
    long lX;       //X轴到达位置绝对位置(单位：pluse)
    long lY;       //Y轴到达位置绝对位置(单位：pluse)
    long lZ;       //Z轴到达位置绝对位置(单位：pluse)
    long lA;       //A轴到达位置绝对位置(单位：pluse)
    
	long segNum;

	unsigned char iDimension; //参与插补的轴数量
	unsigned char iPreciseStopFlag;   //精准定位标志位，如果为1，终点按照终点坐标来
    
};

//G02_G03(顺圆弧插补)命令参数(任意2轴，上位机保证)
struct _G02_3PARA{
	float synVel;    //插补段合成速度
	float synAcc;    //插补段合成加速度
	float velEnd;   //插补段的终点速度
    int iPlaneSelect;       //平面选择0：XY平面 1：XZ平面 2：YZ平面
    int iEnd1;              //第一轴终点坐标（单位um）
    int iEnd2;              //第二轴终点坐标（单位um）
    int iI;                 //圆心坐标（单位um）(相对于起点)
    int iJ;                 //圆心坐标（单位um）(相对于起点)
	long segNum;
    unsigned char iPreciseStopFlag;   //精准定位标志位，如果为1，终点按照终点坐标来
};

//G04延时
struct _G04PARA{
unsigned long ulDelayTime;       //延时时间,单位MS
long segNum;
};

//G05设置用户自定义段号
struct _G05PARA{
long lUserSegNum;       //用户自定义段号
};

//BufferMove命令参数(最多支持8轴)
struct _BufferMoveGearPARA{
	long lAxis1Pos[8];         //轴目标位置，最大支持8轴。轴的加速度和速度采用点位运动速度和加速度。该轴必须处于点位模式且不是插补轴
	long lUserSegNum;          //用户自定义行号
	unsigned char cAxisMask;   //轴掩码，bit0代表轴1，bit1代表轴2，.......对应位为1代表该轴要bufferMove
	unsigned char cModalMask;  //轴掩码，bit0代表轴1，bit1代表轴2，.......对应位为1代表该轴为阻塞，该轴到位后才进入下一行
};

//BufferMove设置Vel和Acc命令参数(最多支持8轴)
struct _BufferMoveVelAccPARA{
	float dVelAcc[8];          //轴速度及加速度，最大支持8轴。
	long lUserSegNum;          //用户自定义行号
	unsigned char cAxisMask;   //轴掩码，bit0代表轴1，bit1代表轴2，.......对应位为1代表该轴要bufferMove
};

//SetIO设置物理IO
struct _SetIOPara{
	unsigned short nCarkIndex;  //板卡索引，0代表主卡，1代表扩展卡1，2代表扩展卡2......依次类推
	unsigned short nDoMask;
	unsigned short nDoValue;
	long lUserSegNum;
};

//G代码参数
union _CMDPara{
    struct _G00PARA     G00PARA;
    struct _G01PARA     G01PARA;
    struct _G02_3PARA   G02_3PARA;
    struct _G04PARA     G04PARA;
    struct _G05PARA     G05PARA;
	struct _BufferMoveGearPARA  BufferMoveGearPARA;
	struct _BufferMoveVelAccPARA BufferMoveVelAccPARA;
	struct _SetIOPara   SetIOPara;
};

//每一行程序结构体
typedef struct _CrdData{
    unsigned char CMDType;              //指令类型，支持最多255种指令0：GOO 1：G01 2：G02 FF:文件结束
    union _CMDPara CMDPara;         //指令参数，不同命令对应不同参数
}TCrdData;

//前瞻参数结构体
typedef struct _LookAheadPrm
{
	int lookAheadNum;                               //前瞻段数
	TCrdData *pLookAheadBuf;                        //前瞻缓冲区指针
	double dSpeedMax[INTERPOLATION_AXIS_MAX];	    //各轴的最大速度(p/ms)
	double dAccMax[INTERPOLATION_AXIS_MAX];			//各轴的最大加速度
	double dMaxStepSpeed[INTERPOLATION_AXIS_MAX];   //各轴的最大速度变化量（相当于启动速度）
	double dScale[INTERPOLATION_AXIS_MAX];			//各轴的脉冲当量
}TLookAheadPrm;

//轴回零参数
typedef struct _AxisHomeParm{
	short		nHomeMode;					//回零方式：0--无 1--HOME回原点	2--HOME加Index回原点3----Z脉冲	
	short		nHomeDir;					//回零方向，1-正向回零，0-负向回零
	long        lOffset;                    //回零偏移，回到零位后再走一个Offset作为零位

	double		dHomeRapidVel;			    //回零快移速度，单位：Pluse/ms
	double		dHomeLocatVel;			    //回零定位速度，单位：Pluse/ms
	double		dHomeIndexVel;			    //回零寻找INDEX速度，单位：Pluse/ms
	double      dHomeAcc;                   //回零使用的加速度

}TAxisHomePrm;

//系统状态结构体
typedef struct _AllSysStatusData
{
	double dAxisEncPos[9];//轴编码器位置，包含一个手轮
	double dAxisPrfPos[8];//轴规划位置
	unsigned long lAxisStatus[8];//轴状态
	short nADCValue[2];//ADC值
	long lUserSegNum[2];//两个坐标系的用户段号
	long lRemainderSegNum[2];//两个坐标系的剩余段号
	short nCrdRunStatus[2];//两个坐标系的坐标系状态
	long lCrdSpace[2];//两个坐标系的剩余空间
	double dCrdVel[2];//两个坐标系的速度
	double dCrdPos[2][5];//两个坐标系的坐标
	long lLimitPosRaw;//正硬限位
	long lLimitNegRaw;//负硬限位
	long lAlarmRaw;//报警输入
	long lHomeRaw;//零位输入
	long lMPG;//手轮信号
	long lGpiRaw[4];//通用IO输入（除主卡外，最大支持3个扩展模块）
}TAllSysStatusData;

//其他指令列表
extern "C" __declspec(dllimport) int GA_Open(short iType=0,char* cName="COM1");
extern "C" __declspec(dllimport) int GA_OpenByIP(char* cPCIP,char* cCardIP);
extern "C" __declspec(dllimport) int GA_Close(void);
extern "C" __declspec(dllimport) int GA_SetCardNo(short iCardNum);
extern "C" __declspec(dllimport) int GA_GetCardNo(short *pCardNum);
extern "C" __declspec(dllimport) int GA_Reset();
extern "C" __declspec(dllimport) int GA_GetVersion(char *pVersion);
extern "C" __declspec(dllimport) int GA_SetPrfPos(short profile,long prfPos);
extern "C" __declspec(dllimport) int GA_SynchAxisPos(long mask);
extern "C" __declspec(dllimport) int GA_ZeroPos(short nAxisNum,short nCount=1);
extern "C" __declspec(dllimport) int GA_SetAxisBand(short nAxisNum,long lBand,long lTime);
extern "C" __declspec(dllimport) int GA_GetAxisBand(short nAxisNum,long *pBand,long *pTime);
extern "C" __declspec(dllimport) int GA_SetBacklash(short nAxisNum,long lCompValue,double dCompChangeValue,long lCompDir);
extern "C" __declspec(dllimport) int GA_GetBacklash(short nAxisNum,long *pCompValue,double *pCompChangeValue,long *pCompDir);
extern "C" __declspec(dllimport) int GA_SendString(char* cString,int iLen,int iOpenFlag=0);
extern "C" __declspec(dllimport) int GA_SetPCEthernetPort(unsigned short nPCEthernetPort);
extern "C" __declspec(dllimport) int GA_SetBaudRate(int BaudRate);
//系统配置信息
extern "C" __declspec(dllimport) int GA_HomeSns(unsigned short sense);
extern "C" __declspec(dllimport) int GA_GetHomeSns(short *pSense);
extern "C" __declspec(dllimport) int GA_AlarmOn(short nAxisNum);
extern "C" __declspec(dllimport) int GA_AlarmOff(short nAxisNum);
extern "C" __declspec(dllimport) int GA_GetAlarmOnOff(short nAxisNum,short *pAlarmOnOff);
extern "C" __declspec(dllimport) int GA_AlarmSns(unsigned short nSense);
extern "C" __declspec(dllimport) int GA_GetAlarmSns(short *pSense);
extern "C" __declspec(dllimport) int GA_LmtsOn(short nAxisNum,short limitType=-1);
extern "C" __declspec(dllimport) int GA_LmtsOff(short nAxisNum,short limitType=-1);
extern "C" __declspec(dllimport) int GA_GetLmtsOnOff(short nAxisNum,short *pPosLmtsOnOff, short *pNegLmtsOnOff);
extern "C" __declspec(dllimport) int GA_LmtSns(unsigned short nSense);
extern "C" __declspec(dllimport) int GA_GetLmtSns(unsigned short *pSense);
extern "C" __declspec(dllimport) int GA_ProfileScale(short nAxisNum,short alpha,short beta);
extern "C" __declspec(dllimport) int GA_EncScale(short nAxisNum,short alpha,short beta);
extern "C" __declspec(dllimport) int GA_StepDir(short step);
extern "C" __declspec(dllimport) int GA_StepPulse(short step);
extern "C" __declspec(dllimport) int GA_GetStep(short nAxisNum,short *pStep);
extern "C" __declspec(dllimport) int GA_StepSns(unsigned short sense);
extern "C" __declspec(dllimport) int GA_GetStepSns(short *pSense);
extern "C" __declspec(dllimport) int GA_EncSns(unsigned short sense);
extern "C" __declspec(dllimport) int GA_GetEncSns(short *pSense);
extern "C" __declspec(dllimport) int GA_EncOn(short nEncoderNum);
extern "C" __declspec(dllimport) int GA_EncOff(short nEncoderNum);
extern "C" __declspec(dllimport) int GA_GetEncOnOff(short nAxisNum,short *pEncOnOff);
extern "C" __declspec(dllimport) int GA_SetPosErr(short nAxisNum,long lError);
extern "C" __declspec(dllimport) int GA_GetPosErr(short nAxisNum,long *pError);
extern "C" __declspec(dllimport) int GA_SetStopDec(short nAxisNum,double decSmoothStop,double decAbruptStop);
extern "C" __declspec(dllimport) int GA_GetStopDec(short nAxisNum,double *pDecSmoothStop,double *pDecAbruptStop);
extern "C" __declspec(dllimport) int GA_CtrlMode(short nAxisNum,short mode);
extern "C" __declspec(dllimport) int GA_GetCtrlMode(short nAxisNum,short *pMode);
extern "C" __declspec(dllimport) int GA_SetStopIo(short nAxisNum,short stopType,short inputType,short inputIndex);

//运动状态检测指令列表
extern "C" __declspec(dllimport) int GA_GetSts(short nAxisNum,long *pSts,short nCount=1,unsigned long *pClock=NULL);
extern "C" __declspec(dllimport) int GA_ClrSts(short nAxisNum,short nCount=1);
extern "C" __declspec(dllimport) int GA_GetPrfMode(short profile,long *pValue,short nCount=1,unsigned long *pClock=NULL);
extern "C" __declspec(dllimport) int GA_GetPrfPos(short nAxisNum,double *pValue,short nCount=1,unsigned long *pClock=NULL);
extern "C" __declspec(dllimport) int GA_GetPrfVel(short nAxisNum,double *pValue,short nCount=1,unsigned long *pClock=NULL);
extern "C" __declspec(dllimport) int GA_GetPrfAcc(short nAxisNum,double *pValue,short nCount=1,unsigned long *pClock=NULL);
extern "C" __declspec(dllimport) int GA_GetAxisPrfPos(short nAxisNum,double *pValue,short nCount=1,unsigned long *pClock=NULL);
extern "C" __declspec(dllimport) int GA_GetAxisPrfVel(short nAxisNum,double *pValue,short nCount=1,unsigned long *pClock=NULL);
extern "C" __declspec(dllimport) int GA_GetAxisPrfAcc(short nAxisNum,double *pValue,short nCount=1,unsigned long *pClock=NULL);
extern "C" __declspec(dllimport) int GA_GetAxisEncPos(short nAxisNum,double *pValue,short nCount=1,unsigned long *pClock=NULL);
extern "C" __declspec(dllimport) int GA_GetAxisEncVel(short nAxisNum,double *pValue,short nCount=1,unsigned long *pClock=NULL);
extern "C" __declspec(dllimport) int GA_GetAxisEncAcc(short nAxisNum,double *pValue,short nCount=1,unsigned long *pClock=NULL);
extern "C" __declspec(dllimport) int GA_GetAxisError(short nAxisNum,double *pValue,short nCount=1,unsigned long *pClock=NULL);
extern "C" __declspec(dllimport) int GA_Stop(long lMask,long lOption);
extern "C" __declspec(dllimport) int GA_AxisOn(short nAxisNum);
extern "C" __declspec(dllimport) int GA_AxisOff(short nAxisNum);
extern "C" __declspec(dllimport) int GA_GetAllSysStatus(TAllSysStatusData *pAllSysStatusData);

//点位运动指令列表（包括点位和速度模式）
extern "C" __declspec(dllimport) int GA_PrfTrap(short nAxisNum);
extern "C" __declspec(dllimport) int GA_SetTrapPrm(short nAxisNum,TTrapPrm *pPrm);
extern "C" __declspec(dllimport) int GA_SetTrapPrmSingle(short nAxisNum,double dAcc,double dDec,double dVelStart,short  dSmoothTime);
extern "C" __declspec(dllimport) int GA_GetTrapPrm(short nAxisNum,TTrapPrm *pPrm);
extern "C" __declspec(dllimport) int GA_GetTrapPrmSingle(short nAxisNum,double* dAcc,double* dDec,double* dVelStart,short*  dSmoothTime);
extern "C" __declspec(dllimport) int GA_PrfJog(short nAxisNum);
extern "C" __declspec(dllimport) int GA_SetJogPrm(short nAxisNum,TJogPrm *pPrm);
extern "C" __declspec(dllimport) int GA_SetJogPrmSingle(short nAxisNum,double dAcc,double dDec,double dSmooth);
extern "C" __declspec(dllimport) int GA_GetJogPrm(short nAxisNum,TJogPrm *pPrm);
extern "C" __declspec(dllimport) int GA_GetJogPrmSingle(short nAxisNum,double* dAcc,double* dDec,double* dSmooth);
extern "C" __declspec(dllimport) int GA_SetPos(short nAxisNum,long pos);
extern "C" __declspec(dllimport) int GA_GetPos(short nAxisNum,long *pPos);
extern "C" __declspec(dllimport) int GA_SetVel(short nAxisNum,double vel);
extern "C" __declspec(dllimport) int GA_GetVel(short nAxisNum,double *pVel);
extern "C" __declspec(dllimport) int GA_SetMultiVel(short nAxisNum,double *pVel,short nCount=1);
extern "C" __declspec(dllimport) int GA_SetMultiPos(short nAxisNum,long *pPos,short nCount=1);
extern "C" __declspec(dllimport) int GA_Update(long mask);

//电子齿轮模式指令列表
extern "C" __declspec(dllimport) int GA_PrfGear(short nAxisNum,short dir=0);
extern "C" __declspec(dllimport) int GA_SetGearMaster(short nAxisNum,short nMasterAxisNum,short masterType=GEAR_MASTER_PROFILE);
extern "C" __declspec(dllimport) int GA_GetGearMaster(short nAxisNum,short *nMasterAxisNum,short *pMasterType=NULL);
extern "C" __declspec(dllimport) int GA_SetGearRatio(short nAxisNum,long masterEven,long slaveEven,long masterSlope=0,long lStopSmoothTime = 200);
extern "C" __declspec(dllimport) int GA_GetGearRatio(short nAxisNum,long *pMasterEven,long *pSlaveEven,long *pMasterSlope=NULL,long *pStopSmoothTime=NULL);
extern "C" __declspec(dllimport) int GA_GearStart(long mask);
extern "C" __declspec(dllimport) int GA_GearStop(long mask);
extern "C" __declspec(dllimport) int GA_SetGearEvent(short nAxisNum,short nEvent,double startPara0,double startPara1);
extern "C" __declspec(dllimport) int GA_GetGearEvent(short nAxisNum,short *pEvent,double *pStartPara0,double *pStartPara1);

//PT模式指令列表
extern "C" __declspec(dllimport) int GA_PrfPt(short nAxisNum,short mode=PT_MODE_STATIC);
extern "C" __declspec(dllimport) int GA_PtSpace(short nAxisNum,long *pSpace,short nCount);
extern "C" __declspec(dllimport) int GA_PtRemain(short nAxisNum,long *pRemainSpace,short nCount);
extern "C" __declspec(dllimport) int GA_PtData(short nAxisNum,short* pData,long lLength,double dDataID);
extern "C" __declspec(dllimport) int GA_PtClear(long lAxisMask);
extern "C" __declspec(dllimport) int GA_PtStart(long lAxisMask);

//插补运动模式指令列表
extern "C" __declspec(dllimport) int GA_SetCrdPrm(short nCrdNum,TCrdPrm *pCrdPrm);
extern "C" __declspec(dllimport) int GA_SetCrdPrmSingle(short nCrdNum,short dimension,short *profile,double synVelMax,double synAccMax,short evenTime,short setOriginFlag,long *originPos);
extern "C" __declspec(dllimport) int GA_GetCrdPrm(short nCrdNum,TCrdPrm *pCrdPrm);
extern "C" __declspec(dllimport) int GA_InitLookAhead(short nCrdNum,short FifoIndex,TLookAheadPrm* plookAheadPara);
extern "C" __declspec(dllimport) int GA_InitLookAheadSingle(short nCrdNum,short FifoIndex,int lookAheadNum,double* dSpeedMax,double* dAccMax,double *dMaxStepSpeed,double *dScale);
extern "C" __declspec(dllimport) int GA_CrdClear(short nCrdNum,short FifoIndex);
extern "C" __declspec(dllimport) int GA_LnXY(short nCrdNum,long x,long y,double synVel,double synAcc,double velEnd=0,short FifoIndex=0,long segNum = 0);
extern "C" __declspec(dllimport) int GA_LnXYZ(short nCrdNum,long x,long y,long z,double synVel,double synAcc,double velEnd=0,short FifoIndex=0,long segNum = 0);
extern "C" __declspec(dllimport) int GA_LnXYZG0(short nCrdNum,long x,long y,long z,double synVel,double synAcc,short FifoIndex=0,long segNum = 0);
extern "C" __declspec(dllimport) int GA_ArcXYC(short nCrdNum,long x,long y,double xCenter,double yCenter,short circleDir,double synVel,double synAcc,double velEnd=0,short FifoIndex=0,long segNum = 0);
extern "C" __declspec(dllimport) int GA_ArcXZC(short nCrdNum,long x,long z,double xCenter,double zCenter,short circleDir,double synVel,double synAcc,double velEnd=0,short FifoIndex=0,long segNum = 0);
extern "C" __declspec(dllimport) int GA_ArcYZC(short nCrdNum,long y,long z,double yCenter,double zCenter,short circleDir,double synVel,double synAcc,double velEnd=0,short FifoIndex=0,long segNum = 0);
extern "C" __declspec(dllimport) int GA_BufIO(short nCrdNum,unsigned short nDoType,unsigned short nCardIndex,unsigned short doMask,unsigned short doValue,short FifoIndex=0,long segNum = 0);
extern "C" __declspec(dllimport) int GA_BufDelay(short nCrdNum,unsigned long ulDelayTime,short FifoIndex=0,long segNum = 0);
extern "C" __declspec(dllimport) int GA_BufMoveVel(short nCrdNum,short nAxisMask,float* pVel,short nFifoIndex=0,long lSegNum=-1);
extern "C" __declspec(dllimport) int GA_BufMoveAcc(short nCrdNum,short nAxisMask,float* pAcc,short nFifoIndex=0,long lSegNum=-1);
extern "C" __declspec(dllimport) int GA_BufMove(short nCrdNum,short nAxisMask,long* pPos,short nModalMask,short nFifoIndex=0,long lSegNum=-1);
extern "C" __declspec(dllimport) int GA_BufGear(short nCrdNum,short nAxisMask,long* pPos,short nFifoIndex=0,long lSegNum=-1);
extern "C" __declspec(dllimport) int GA_CrdData(short nCrdNum,void *pCrdData,short FifoIndex=0);
extern "C" __declspec(dllimport) int GA_CrdStart(short mask,short option);
extern "C" __declspec(dllimport) int GA_SetOverride(short nCrdNum,double synVelRatio);
extern "C" __declspec(dllimport) int GA_GetCrdPos(short nCrdNum,double *pPos);
extern "C" __declspec(dllimport) int GA_GetCrdVel(short nCrdNum,double *pSynVel);
extern "C" __declspec(dllimport) int GA_CrdSpace(short nCrdNum,long *pSpace,short FifoIndex=0);
extern "C" __declspec(dllimport) int GA_CrdStatus(short nCrdNum,short *pRun,long *pSegment,short FifoIndex=0);
extern "C" __declspec(dllimport) int GA_SetUserSegNum(short nCrdNum,long segNum,short FifoIndex=0);
extern "C" __declspec(dllimport) int GA_GetUserSegNum(short nCrdNum,long *pSegment,short FifoIndex=0);
extern "C" __declspec(dllimport) int GA_GetRemainderSegNum(short nCrdNum,long *pSegment,short FifoIndex=0);
extern "C" __declspec(dllimport) int GA_GetLookAheadSpace(short nCrdNum,long *pSpace,short nFifoIndex=0);
extern "C" __declspec(dllimport) int GA_GetLookAheadSegCount(short nCrdNum,long *pSegCount,short FifoIndex=0);

//访问硬件资源指令列表
extern "C" __declspec(dllimport) int GA_GetDi(short nDiType,long *pValue);
extern "C" __declspec(dllimport) int GA_GetDiRaw(short nDiType,long *pValue);
extern "C" __declspec(dllimport) int GA_GetDiReverseCount(short nDiType,short diIndex,unsigned long *pReverseCount,short nCount=1);
extern "C" __declspec(dllimport) int GA_SetDiReverseCount(short nDiType,short diIndex,unsigned long ReverseCount,short nCount=1);
extern "C" __declspec(dllimport) int GA_SetDo(short nDoType,long value);
extern "C" __declspec(dllimport) int GA_SetDoBit(short nDoType,short nDoNum,short value);
extern "C" __declspec(dllimport) int GA_SetDoBitReverse(short nDoType,short nDoNum,long value,short reverseTime);
extern "C" __declspec(dllimport) int GA_GetDo(short nDoType,long *pValue);
extern "C" __declspec(dllimport) int GA_GetEncPos(short nEncodeNum,double *pValue,short nCount=1,unsigned long *pClock=NULL);
extern "C" __declspec(dllimport) int GA_GetEncVel(short nEncodeNum,double *pValue,short nCount=1,unsigned long *pClock=NULL);
extern "C" __declspec(dllimport) int GA_SetEncPos(short nEncodeNum,long encPos);
extern "C" __declspec(dllimport) int GA_SetDac(short nDacNum,short* pValue,short nCount=1);
extern "C" __declspec(dllimport) int GA_GetAdc(short nADCNum,short *pValue,short nCount=1,unsigned long *pClock=NULL);
extern "C" __declspec(dllimport) int GA_SetPwm(short nPwmNum ,double dFreq,double dDuty);
extern "C" __declspec(dllimport) int GA_GetPwm(short nPwmNum ,double *pFreq,double *pDuty);
extern "C" __declspec(dllimport) int GA_SetExtDoValue(short nCardIndex,unsigned long *value,short nCount=1);
extern "C" __declspec(dllimport) int GA_GetExtDiValue(short nCardIndex,unsigned long *pValue,short nCount=1);
extern "C" __declspec(dllimport) int GA_GetExtDoValue(short nCardIndex,unsigned long *pValue,short nCount=1);
extern "C" __declspec(dllimport) int GA_SetExtDoBit(short nCardIndex,short nBitIndex,unsigned short nValue);
extern "C" __declspec(dllimport) int GA_GetExtDiBit(short nCardIndex,short nBitIndex,unsigned short *pValue);
extern "C" __declspec(dllimport) int GA_GetExtDoBit(short nCardIndex,short nBitIndex,unsigned short *pValue);
extern "C" __declspec(dllimport) int GA_SendEthToUartString(short nUartNum,unsigned char*pSendBuf, short nLength);
extern "C" __declspec(dllimport) int GA_ReadUartToEthString(short nUartNum,unsigned char* pRecvBuf, short* pLength);
extern "C" __declspec(dllimport) int GA_SetExDac(short nCardIndex,short nDacNum,short* pValue,short nCount=1);
extern "C" __declspec(dllimport) int GA_GetExAdc(short nCardIndex,short nADCNum,short *pValue,short nCount=1,unsigned long *pClock=NULL);

//比较输出指令
extern "C" __declspec(dllimport) int GA_CmpPluse(short nChannel, short nPluseType1, short nPluseType2, short nTime1,short nTime2);
extern "C" __declspec(dllimport) int GA_CmpBufSetChannel(short nBuf1ChannelNum,short nBuf2ChannelNum);
extern "C" __declspec(dllimport) int GA_CmpBufData(short nCmpEncodeNum, short nPluseType, short nStartLevel, short nTime, long *pBuf1, short nBufLen1, long *pBuf2, short nBufLen2,short nAbsPosFlag=0,short nTimerFlag=0);
extern "C" __declspec(dllimport) int GA_CmpBufSts(short *pStatus,unsigned short *pCount1,unsigned short *pCount2);
extern "C" __declspec(dllimport) int GA_CmpBufStop(short nChannel);
extern "C" __declspec(dllimport) int GA_CmpRpt(short nEncode, short nEncodeType,short nChannel,long lStartPos, long lRptTime, long lInterval, short nTime);


//高速硬件捕获指令列表
extern "C" __declspec(dllimport) int GA_SetCaptureMode(short nEncodeNum,short mode);
extern "C" __declspec(dllimport) int GA_GetCaptureMode(short nEncodeNum,short *pMode,short nCount=1);
extern "C" __declspec(dllimport) int GA_GetCaptureStatus(short nEncodeNum,short *pStatus,long *pValue,short nCount=1,unsigned long *pClock=NULL);
extern "C" __declspec(dllimport) int GA_SetCaptureSense(short nEncodeNum,short mode,short sense);
extern "C" __declspec(dllimport) int GA_GetCaptureSense(short nEncodeNum,short mode,short *sense);
extern "C" __declspec(dllimport) int GA_ClearCaptureStatus(short nEncodeNum);
extern "C" __declspec(dllimport) int GA_SetContinueCaptureMode(short nEncodeNum,short nMode,short nContinueMode,short nFilterTime);
extern "C" __declspec(dllimport) int GA_GetContinueCaptureData(short nEncodeNum,long *pCapturePos,short* pCaptureCount);

//安全机制指令列表
extern "C" __declspec(dllimport) int GA_SetSoftLimit(short nAxisNum,long lPositive,long lNegative);
extern "C" __declspec(dllimport) int GA_GetSoftLimit(short nAxisNum,long *pPositive,long *pNegative);
extern "C" __declspec(dllimport) int GA_SetHardLimP(short nAxisNum,short nType ,short nCardIndex,short nIOIndex);
extern "C" __declspec(dllimport) int GA_SetHardLimN(short nAxisNum,short nType ,short nCardIndex,short nIOIndex);
extern "C" __declspec(dllimport) int GA_EStopSetIO(short nCardIndex,short nIOIndex,short nEStopSns,unsigned long lFilterTime);
extern "C" __declspec(dllimport) int GA_EStopOnOff(short nEStopOnOff);
extern "C" __declspec(dllimport) int GA_EStopGetSts(short *nEStopSts);
extern "C" __declspec(dllimport) int GA_EStopClrSts();

//自动回零相关API
extern "C" __declspec(dllimport) int GA_HomeStart(int iAxisNum);
extern "C" __declspec(dllimport) int GA_HomeStop(int iAxisNum);
extern "C" __declspec(dllimport) int GA_HomeSetPrm(int iAxisNum,TAxisHomePrm *pAxisHomePrm);
extern "C" __declspec(dllimport) int GA_HomeSetPrmSingle(short iAxisNum,short nHomeMode,short nHomeDir,long lOffset,double dHomeRapidVel,double dHomeLocatVel,double dHomeIndexVel,double dHomeAcc);
extern "C" __declspec(dllimport) int GA_HomeGetPrm(int iAxisNum,TAxisHomePrm *pAxisHomePrm);
extern "C" __declspec(dllimport) int GA_HomeGetPrmSingle(short iAxisNum,short *nHomeMode,short *nHomeDir,long *lOffset,double* dHomeRapidVel,double* dHomeLocatVel,double* dHomeIndexVel,double* dHomeAcc);
extern "C" __declspec(dllimport) int GA_HomeGetSts(int iAxisNum,unsigned short* pStatus);

//手轮相关
extern "C" __declspec(dllimport) int GA_StartHandwheel(short nAxisNum,short nMasterAxisNum = 9,long lMasterEven = 1,long lSlaveEven = 1,short nIntervalTime = 0,double dAcc = 0.1,double dDec = 0.1,double dVel = 50,short nStopWaitTime = 0);
extern "C" __declspec(dllimport) int GA_EndHandwheel(short nAxisNum);

//其他
extern "C" __declspec(dllimport) int GA_GetIP(unsigned long* pIP);
extern "C" __declspec(dllimport) int GA_SetIP(unsigned long ulIP);
extern "C" __declspec(dllimport) int GA_GetID(unsigned long* pID);
#pragma pack(pop)