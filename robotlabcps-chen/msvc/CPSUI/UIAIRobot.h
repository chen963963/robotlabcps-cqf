#pragma once
#include "UIBaseWindow.h"
#include "CPSAPI/CPSAPI.h"
#include "UIImageView.h"

class UIAIRobot :
    public UIBaseWindow
{
public:
    UIAIRobot(UIGLWindow* main_win, const char* title);

    virtual void Draw();

    virtual EUIMenuCategory GetWinMenuCategory() { return EUIMenuCategory::E_UI_CAT_TOOL; }
    virtual const char* GetShowShortCut() { return "Ctrl+1"; }

    virtual void OnCPSMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len);
    
    virtual void MoveURThreadFunc(const char* data);
protected:
    CCPSAPI* m_cpsapi = nullptr;
    UIImageBasePtr m_image = nullptr;
    UIImageBasePtr key_image = nullptr;
    UIImageBasePtr key0_image = nullptr;
    float m_image_zoom = 1.0f;
};

