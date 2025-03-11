#include "UIAIRobot.h"
#include "UIMainWindow.h"
#include "stb/stb_sprintf.h"
#include "IconsFontAwesome5.h"
#include "UILog.h"
#include "UIApplication.h"
#include "CPSMsg.h"
#include "CPSAPI/CPSDef.h"
#include "UIImageView.h"
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
std::string path="";
std::string path_old="";

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

    if (ImGui::Button(u8"Capture"))
    {

        std::string json_data = "{\"name\":\"capture\", \"value\" : 1}";
        m_cpsapi->SendAPPMsg(774, MSG_CAPTURE_IMAGE, json_data.c_str(), json_data.size()+1);


    }
    ImGui::SliderFloat("Zoom", &m_image_zoom, 0.1f, 5.0f, "%.1f");

    if (path_old!=path)
	{
        m_image = UIImageFactory::LoadFromFile(path.c_str());
        path_old = path ;
	}

    if (m_image && (path_old == path))
	{
		m_image->Draw(int(m_image->GetWidth() * m_image_zoom), int(m_image->GetHeight() * m_image_zoom));
	}
    ImGui::End();
}

void UIAIRobot::OnCPSMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len)
{
	switch (msg_type)
	{
	case MSG_CAPTURE_IMAGE_RSP:{
		UI_INFO("Received from CameraServer %s", data);

		json j = json::parse(data);
        std::string image_path = j["path"];
        image_path.append("/0.png");
        path = image_path;
        UI_INFO("path is %s",path.c_str());



        m_cpsapi->SendAPPMsg(775, MSG_PREDICT, data, strlen(data)+1);

		break;}
	case MSG_PREDICT_RSP:{
//	    Json::CharReaderBuilder reader;
//        Json::Value root;
//        std::string errs;
//
//	    std::istringstream s(data);
//	    std::string name = root["name"].asString();
//        std::string value = root["value"].asString();
	    UI_INFO("Received from VisionServer %s", data);
		std::thread newThread(&UIAIRobot::MoveURThreadFunc, this, data);
		newThread.join();
	    break;}
	}
}

void UIAIRobot::MoveURThreadFunc(const char* data)
{
        json j = json::parse(data);
        std::string state = j["name"];
        std::vector<double> data_pose = j["value"];

        std::stringstream ss;
        ss << "["; 
        for (size_t i = 0; i < data_pose.size(); i++) {
        ss << data_pose[i]; 
        if (i < data_pose.size() - 1) {
            ss << ","; 
        }
        }
        ss << "]"; 

        std::string command = ss.str();
        if(state == "error"){
        }
        else if (state == "catch"){
                UI_INFO("已到达目标位姿，准备抓取");

                RTDEControlInterface rtde_control("192.168.12.252");
                rtde_control.moveL({data_pose},0.1,0.2);
                rtde_control.disconnect();

                RTDEReceiveInterface rtde_receive("192.168.12.252");
                std::vector <double> ActualTCPPose = rtde_receive.getActualTCPPose();
                UI_INFO("目前位姿%.3f,%.3f,%.3f,%.3f,%.3f,%.3f",ActualTCPPose[0],ActualTCPPose[1],ActualTCPPose[2],ActualTCPPose[3],ActualTCPPose[4],ActualTCPPose[5]);
                rtde_receive.disconnect();


        }
        else if (state == "move"){
                UI_INFO("move command%s",command.c_str());

                RTDEControlInterface rtde_control("192.168.12.252");
                rtde_control.moveL({data_pose},0.1,0.2);
                rtde_control.disconnect();

                RTDEReceiveInterface rtde_receive("192.168.12.252");
                std::vector <double> ActualTCPPose = rtde_receive.getActualTCPPose();
                UI_INFO("目前位姿%.3f,%.3f,%.3f,%.3f,%.3f,%.3f",ActualTCPPose[0],ActualTCPPose[1],ActualTCPPose[2],ActualTCPPose[3],ActualTCPPose[4],ActualTCPPose[5]);
                rtde_receive.disconnect();

                std::string json_data = "{\"name\":\"capture\", \"value\" : 1}";
                m_cpsapi->SendAPPMsg(774, MSG_CAPTURE_IMAGE, json_data.c_str(), json_data.size()+1);
        }
        else {
                UI_INFO("incorrect state");
        }
}

