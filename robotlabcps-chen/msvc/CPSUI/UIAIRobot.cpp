#include "UIAIRobot.h"
#include "UIMainWindow.h"
#include "stb/stb_sprintf.h"
#include "IconsFontAwesome5.h"
#include "UILog.h"
#include "UIApplication.h"
#include "CPSMsg.h"
#include "CPSAPI/CPSDef.h"
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <iostream>
#include <string>

#include <ur_rtde/rtde_control_interface.h>
#include <ur_rtde/rtde_receive_interface.h>

using namespace ur_rtde;
using namespace std::chrono;


UIAIRobot::UIAIRobot(UIGLWindow* main_win, const char* title) :UIBaseWindow(main_win, title)
{
    m_cpsapi = g_app.GetCPSApi();
}

void UIAIRobot::Draw()
{
    if (!m_show)
    {
        return;
    }

    if (!ImGui::Begin(m_win_title, &m_show, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
    {
        ImGui::End();
        return;
    }
    char buf[64] = { 0 };

    if (ImGui::Button(u8"����"))
    {
        UI_INFO(u8"������������....");
        std::string json_data = "{\"name\":\"capture\", \"value\" : 1}";
        m_cpsapi->SendAPPMsg(774, MSG_CAPTURE_IMAGE, json_data.c_str(), json_data.size()+1);
    }

    ImGui::End();
}

void UIAIRobot::OnCPSMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len)
{
	switch (msg_type)
	{
	case MSG_CAPTURE_IMAGE_RSP:
		UI_INFO("Received from VisionServer %s", data);

		// �������̣߳����Ϳ���ָ���UR
		std::thread newThread(&UIAIRobot::MoveURThreadFunc, this);

		// �ȴ����߳�ִ�����
		newThread.join();

		UI_INFO("UR move done");
		break;
	}
}

void UIAIRobot::MoveURThreadFunc()
{
	UI_INFO("UR...\n");

	// ����UR
	RTDEControlInterface rtde_control("192.168.12.252");//both are UR's IP, not PC
	RTDEReceiveInterface rtde_receive("192.168.12.252");

	//��������λ��
	rtde_control.moveJ({ 1.948, -1.316, 0.830, -0.925, -1.921, 0.393 }, 0.25, 0.2);//rad
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	// Stop the movement
	rtde_control.stopJ(0.5);
	printf("moveJ done\n");

	//grasp pose
	rtde_control.moveJ({ 1.309,-1.633, 1.269, -1.207, -1.591, -0.288 }, 0.25, 0.2);
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	printf("Grasp done\n");
			
	// Stop the RTDE control script
	rtde_control.stopScript();


}
