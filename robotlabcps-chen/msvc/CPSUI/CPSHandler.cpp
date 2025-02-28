#include "CPSHandler.h"
#ifdef UI_CFG_ENABLE_CPS
#include "UIApplication.h"
#include "UILog.h"
#include "CPSAPI/CPSDef.h"

#ifdef _WIN32
#pragma comment(lib, "cpsapi.lib")
#endif

CPSHandler::CPSHandler()
{
	m_cpsapi = g_app.GetCPSApi();
}

CPSHandler::~CPSHandler()
{
}

void CPSHandler::OnConnected()
{
	m_is_connected = true;
	UI_INFO(u8"���������ӣ�");
	//m_cpsapi->SubscribeDevice(773); //SensorServer
	m_cpsapi->SubscribeDevice(CAMERA_SERVER_DEV_ID); //CameraServer
	m_cpsapi->SubscribeDevice(VISION_SERVER_DEV_ID); //VisionServer
}

void CPSHandler::OnDisconnected()
{
	m_is_connected = false;
	UI_INFO(u8"�����ѶϿ���");
}

void CPSHandler::OnDeviceOnline(uint32_t dev_id)
{
	UI_INFO(u8"�豸<%d>�����ߣ�", dev_id);
}

void CPSHandler::OnDeviceOffline(uint32_t dev_id)
{
	UI_INFO(u8"�豸<%d>�����ߣ�", dev_id);
}

void CPSHandler::OnMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len)
{
	// forward message to view msg handler
	g_app.OnCPSMsg(from_id, msg_type, data, msg_len);
}

#endif