#pragma once
#include <CPSAPI/CPSDef.h>

#pragma pack(push, 4)
////////////////////////////////SensorServer服务消息////////////////////////////////////////

#define MAX_SENSOR_DATA_NUM						20   // 最大20个，如果不够，可以修改
#define	MSG_SENSOR_DATA					         0xA01
typedef struct
{
	unsigned int	sensor_num;
	float			data[MAX_SENSOR_DATA_NUM];
}ST_SensorData;

#define TS_SEG_NUM 6
typedef struct
{
	int	req_id;            // 计算请求的ID
	//目前只适用前三个量，即接触位置、方向和力大小
	double l1; //接触位置
	double l2; //接触位置
	double f1;       //接触力大小
	double f2;       //接触力大小
	double q[TS_SEG_NUM];     // 弹性板当前关节角度
	double rxix[TS_SEG_NUM];
	double rxiy[TS_SEG_NUM];
	double note_y;			//音符y坐标1
	double note_y2;			//音符y坐标2
	ST_MsgRsp info;          // 结果信息，如果出错，给error_code赋非0值
}ST_TS_Calc_Result;

#pragma pack(pop)

