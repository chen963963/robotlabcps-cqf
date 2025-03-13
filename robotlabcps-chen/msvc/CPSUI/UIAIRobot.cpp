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
std::string m_image_path="";
std::string m_image_path_old="";

std::string key_image_path="";
std::string key_image_path_old="";

std::string key0_image_path="";
std::string key0_image_path_old="";

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

    if (m_image_path_old!=m_image_path)
	{
        m_image = UIImageFactory::LoadFromFile(m_image_path.c_str());
        m_image_path_old = m_image_path ;
	}




    if (m_image && (m_image_path_old == m_image_path))
	{
	    //ImVec2 image_position(100.0f, 50.0f);  // 你可以根据需要调整这些值
        //ImGui::SetCursorPos(image_position);
		m_image->Draw(int(m_image->GetWidth() * m_image_zoom), int(m_image->GetHeight() * m_image_zoom));
	}

    if (key_image_path_old!=key_image_path)
	{
        key_image = UIImageFactory::LoadFromFile(key_image_path.c_str());
        key0_image = UIImageFactory::LoadFromFile(key0_image_path.c_str());
        key_image_path_old = key_image_path ;
	}

    if (key_image && (key_image_path_old == key_image_path))
	{
	    ImVec2 image_position(650.0f, 100.0f);  // 你可以根据需要调整这些值
        ImGui::SetCursorPos(image_position);
		key_image->Draw(int(key_image->GetWidth() * 0.5* m_image_zoom), int(key_image->GetHeight() * 0.5* m_image_zoom));
		ImVec2 image_position0(650.0f, 340.0f);  // 你可以根据需要调整这些值
        ImGui::SetCursorPos(image_position0);
		key0_image->Draw(int(key0_image->GetWidth() * 0.5* m_image_zoom), int(key0_image->GetHeight() * 0.5* m_image_zoom));
	}
    ImGui::End();
}

void UIAIRobot::OnCPSMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len)
{
	switch (msg_type)
	{
	case MSG_CAPTURE_IMAGE_RSP:{
		//UI_INFO("Received from CameraServer %s", data);

		json j = json::parse(data);
        std::string image_path = j["path"];
        image_path.append("/0.png");
        m_image_path = image_path;
        //UI_INFO("path is %s",m_image_path.c_str());


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
//	    UI_INFO("Received from VisionServer %s", data);
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
        std::string image_path = j["image_path"];
        std::string image_path0 = j["image_path0"];

        key_image_path = image_path;
        key0_image_path = image_path0;

        //UI_INFO("key_path is %s",key_image_path.c_str());

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

                RTDEControlInterface rtde_control("192.168.12.252",50);
                rtde_control.moveL({data_pose},0.1,0.2);
                rtde_control.disconnect();

        }
        else if (state == "move"){
                UI_INFO("move command%s",command.c_str());

                RTDEControlInterface rtde_control("192.168.12.252",50);
                rtde_control.moveL({data_pose},0.1,0.2);
                rtde_control.disconnect();

                std::string json_data = "{\"name\":\"capture\", \"value\" : 1}";
                m_cpsapi->SendAPPMsg(774, MSG_CAPTURE_IMAGE, json_data.c_str(), json_data.size()+1);
        }
        else {
                UI_INFO("incorrect state");
        }
}