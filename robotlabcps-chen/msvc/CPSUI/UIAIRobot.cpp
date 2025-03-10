#include "UIAIRobot.h"
#include "UIMainWindow.h"
#include "stb/stb_sprintf.h"
#include "IconsFontAwesome5.h"
#include "UILog.h"
#include "UIApplication.h"
#include "CPSMsg.h"
#include "CPSAPI/CPSDef.h"
#include <iostream>
#include <sstream>
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

    if (ImGui::Button(u8"����"))
    {
//        std::vector<double> x = {-0.031, -0.45, 0.6, 1.5, -2.6, 0.3};
//        x[2]-=0.3;
//        RTDEControlInterface rtde_control("192.168.12.252");
//        rtde_control.moveL({x},0.1,0.2);
//        rtde_control.disconnect();
//        RTDEReceiveInterface rtde_receive("192.168.12.252");
//        std::vector <double> ActualTCPPose = rtde_receive.getActualTCPPose();
//        rtde_receive.disconnect();
//        UI_INFO("%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",ActualTCPPose[0],ActualTCPPose[1],ActualTCPPose[2],ActualTCPPose[3],ActualTCPPose[4],ActualTCPPose[5]);
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
		UI_INFO("Received from CameraServer %s", data);

		//std::string json_data = "{\"name\":\"capture\", \"value\" : data}";
        //m_cpsapi->SendAPPMsg(775, MSG_PREDICT, json_data.c_str(), json_data.size()+1);

        m_cpsapi->SendAPPMsg(775, MSG_PREDICT, data, strlen(data)+1);
        // �������̣߳����Ϳ���ָ���UR
		//std::thread newThread(&UIAIRobot::MoveURThreadFunc, this);
		// �ȴ����߳�ִ�����
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
	    // �������̣߳����Ϳ���ָ���UR
		std::thread newThread(&UIAIRobot::MoveURThreadFunc, this, data);
		// �ȴ����߳�ִ�����
		newThread.join();
	    break;
	}
}

void UIAIRobot::MoveURThreadFunc(const char* data)
{
        json j = json::parse(data);
        std::string state = j["name"];
        std::vector<double> data_pose = j["value"];

        std::stringstream ss;
        ss << "["; // ���������
        for (size_t i = 0; i < data_pose.size(); i++) {
        ss << data_pose[i]; // �������Ԫ��
        if (i < data_pose.size() - 1) {
            ss << ","; // ��Ӷ��ţ��������һ��Ԫ�أ�
        }
        }
        ss << "]"; // ���������

        // �� stringstream ת��Ϊ�ַ���
        std::string command = ss.str();
        //UI_INFO("״̬��%s,Ŀ��λ��%s",command.c_str());
        if(state == "error"){
            UI_INFO("Ҫײ������");
        }
        else if (state == "catch"){
                UI_INFO("����Ŀ��λ�ˣ�׼��ץȡ");

                RTDEControlInterface rtde_control("192.168.12.252");
                rtde_control.moveL({data_pose},0.1,0.2);
                rtde_control.disconnect();

                RTDEReceiveInterface rtde_receive("192.168.12.252");
                std::vector <double> ActualTCPPose = rtde_receive.getActualTCPPose();
                UI_INFO("��ǰλ��%.3f,%.3f,%.3f,%.3f,%.3f,%.3f",ActualTCPPose[0],ActualTCPPose[1],ActualTCPPose[2],ActualTCPPose[3],ActualTCPPose[4],ActualTCPPose[5]);
                rtde_receive.disconnect();


        }
        else if (state == "move"){
                UI_INFO("�ƶ���%s",command.c_str());

                RTDEControlInterface rtde_control("192.168.12.252");
                rtde_control.moveL({data_pose},0.1,0.2);
                rtde_control.disconnect();

                RTDEReceiveInterface rtde_receive("192.168.12.252");
                std::vector <double> ActualTCPPose = rtde_receive.getActualTCPPose();
                UI_INFO("��ǰλ��%.3f,%.3f,%.3f,%.3f,%.3f,%.3f",ActualTCPPose[0],ActualTCPPose[1],ActualTCPPose[2],ActualTCPPose[3],ActualTCPPose[4],ActualTCPPose[5]);
                rtde_receive.disconnect();

                std::string json_data = "{\"name\":\"capture\", \"value\" : 1}";
                m_cpsapi->SendAPPMsg(774, MSG_CAPTURE_IMAGE, json_data.c_str(), json_data.size()+1);
        }
        else {
                UI_INFO("incorrect state");
        }
        }
//
//	// ����UR
//	RTDEControlInterface rtde_control("192.168.12.252");//both are UR's IP, not PC
//	RTDEReceiveInterface rtde_receive("192.168.12.252");
//
//	//��������λ��
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
//  rtde_control.stopScript();

