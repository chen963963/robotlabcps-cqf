﻿#pragma once
#include <stdint.h>

// 总控界面APP ID
#define CONTROL_SERVER_APP_ID		0x1001 // 4097
// VisionServer APP ID
#define VISION_SERVER_APP_ID		0x1002 // 4098

// OptServer DEV ID
#define OPT_SERVER_DEV_ID			0x304  // 772
// SensorServer DEV ID
#define SENSOR_SERVER_DEV_ID		0x305  // 773
// CameraServer DEV ID
#define CAMERA_SERVER_DEV_ID		0x306  // 774
// VisionServer DEV ID
#define VISION_SERVER_DEV_ID		0x307  // 775
// MoveServer DEV ID
#define MOVE_SERVER_DEV_ID		0x308  // 776


//
//#pragma pack(push, 4)
//////////////////////////////////消息定义////////////////////////////////////////
//// 错误消息
//#define MSG_RSP_INFO				0x500
//typedef struct
//{
//	int		error_code;
//	char	error_msg[128];
//}ST_MsgRsp;
//
//// 初始化消息
//#define MSG_CMD_INIT				0x501
//typedef struct
//{
//	int req_no;
//}ST_CMDInit;
//
//// 初始化消息应答
//#define MSG_CMD_INIT_RSP			0x502
//typedef struct
//{
//	int req_no;
//	ST_MsgRsp rsp;
//}ST_CMDInitRsp;
//
//// 停止消息
//#define MSG_CMD_STOP				0x503
//typedef struct
//{
//	int req_no;
//}ST_CMDStop;
//
//// 停止消息应答
//#define MSG_CMD_STOP_RSP			0x504
//typedef struct
//{
//	int req_no;
//	ST_MsgRsp rsp;
//}ST_CMDStopRsp;
//
//// Reset请求
//#define MSG_CMD_RESET				0x505
//typedef struct
//{
//	int req_no;
//}ST_CMDReset;

//// Continue请求
//#define MSG_CMD_CONTINUE				0x506
//typedef struct
//{
//	int req_no;
//}ST_CMDContinue;
//
//// 认证请求
//#define	MSG_REQ_AUTH				0x5F0
//typedef struct
//{
//	char	client_key[32];
//	char	client_secret[64];
//}ST_ReqAuth;
//
//// 认证应答
//#define	MSG_RSP_AUTH				0x5F1
//typedef struct
//{
//	char		client_key[16];
//	ST_MsgRsp	info;
//}ST_RspAuth;
//
//#pragma pack(pop)

