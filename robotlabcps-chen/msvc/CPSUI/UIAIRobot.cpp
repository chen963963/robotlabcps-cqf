#include "UIAIRobot.h"
#include "UIMainWindow.h"
#include "stb/stb_sprintf.h"
#include "IconsFontAwesome5.h"
#include "UILog.h"
#include "UIApplication.h"
#include "CPSMsg.h"
#include "CPSAPI/CPSDef.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <map>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <string>
#include <ur_rtde/rtde_control_interface.h>
#include <ur_rtde/rtde_receive_interface.h>
using json=nlohmann::json;
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

    if (ImGui::Button(u8"拍照"))
    {

        UI_INFO(u8"发送拍照请求....");
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
		UI_INFO("Received from CameraServer %s", data);

		//std::string json_data = "{\"name\":\"capture\", \"value\" : data}";
        //m_cpsapi->SendAPPMsg(775, MSG_PREDICT, json_data.c_str(), json_data.size()+1);

        m_cpsapi->SendAPPMsg(775, MSG_PREDICT, data, strlen(data)+1);
        // 创建新线程，发送控制指令给UR
		//std::thread newThread(&UIAIRobot::MoveURThreadFunc, this);
		// 等待新线程执行完毕
		//newThread.join();

		//UI_INFO("UR move done");
		break;
	case MSG_PREDICT_RSP:
//	    Json::CharReaderBuilder reader;
//        Json::Value root;
//        std::string errs;
//
//	    std::istringstream s(data);
//	    std::string name = root["name"].asString();
//        std::string value = root["value"].asString();
	    UI_INFO("Received from VisionServer %s", data);
	    // 创建新线程，发送控制指令给UR
		std::thread newThread(&UIAIRobot::MoveURThreadFunc, this, data);
		// 等待新线程执行完毕
		newThread.join();
	    break;
	}
}

void UIAIRobot::MoveURThreadFunc(const char* data)
{
        json j = json::parse(data);
        std::string state = j["name"];
        std::array<int,6> data_pose = j["value"];
        UI_INFO("哈哈，%s",data);


//
//	// 连接UR
//	RTDEControlInterface rtde_control("192.168.12.252");//both are UR's IP, not PC
//	RTDEReceiveInterface rtde_receive("192.168.12.252");
//
//	//到达拍照位姿
//	rtde_control.moveJ({ 1.948, -1.316, 0.830, -0.925, -1.921, 0.393 }, 0.25, 0.2);//rad
//	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
//
//	// Stop the movement
//	rtde_control.stopJ(0.5);
//	printf("moveJ done\n");
//
//	//grasp pose
//	rtde_control.moveJ({ 1.309,-1.633, 1.269, -1.207, -1.591, -0.288 }, 0.25, 0.2);
//	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
//	printf("Grasp done\n");
//
//	// Stop the RTDE control script
//	rtde_control.stopScript();
//    if(data_pose=="error"){
//        UI_INFO("要撞桌子了");
//    }
//    else if (data_pose=="catch"){
//            UI_INFO("到达目的位姿，准备抓取");
//            rtde_control.moveL({data_pose},0.1,0.2);
//    }
//    else {
//            UI_INFO("移动位置%s",data_pose);
//            rtde_control.moveL({data_pose},0.1,0.2);
//            std::string json_data = "{\"name\":\"capture\", \"value\" : 1}";
//            m_cpsapi->SendAPPMsg(774, MSG_CAPTURE_IMAGE, json_data.c_str(), json_data.size()+1);
//    }


}
