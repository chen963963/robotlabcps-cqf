#pragma once
#include <CPSAPI/CPSDef.h>

#pragma pack(push, 4)
////////////////////////////////MoveServer服务消息////////////////////////////////////////

#define MAX_MOVE_COMMAND_NUM						 20   // 最大20个，如果不够，可以修改 //改
#define	MSG_MOVE_COMMAND					         0xA01
typedef struct
{
	unsigned int	image_num;
	double			data[MAX_MOVE_COMMAND_NUM]; //改
}ST_CameraData;

#define TS_COMMAND_NUM 6 //六个关节
typedef struct
{
	//int	req_id;            // 计算请求的ID
	double MoveCommand[TS_COMMAND_NUM]; //抓取指令
	//ST_MsgRsp info;          // 结果信息，如果出错，给error_code赋非0值
}ST_Move_Command;

#pragma pack(pop)

