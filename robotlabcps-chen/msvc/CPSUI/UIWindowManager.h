#pragma once
#include <vector>
#include <string>
#include "UIWindowManagerBase.h"
#include "UILogView.h"
#include "UISysSettings.h"
#include "UIImageView.h"
#include "UIAIRobot.h"

class UIMainWindow;
class UIApplication;

class UIWindowManager: public UIWindowManagerBase
{
public:
	UIWindowManager(UIMainWindow * ui_main);
	~UIWindowManager();

	void Init() override;

	void Draw() override;

	void OnCPSMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len) override;

	void Destroy() override;
	
	UISysSettings* GetSysSettings() { return m_ui_sys; }
public:
	void ShowOpenModelFileDialog();
	void ShowSaveModelFileDialog();

	void OpenFile(const std::string& filename);
	void SaveFile(const std::string& filename);

	void SetResetLayoutFlag() { m_reset_layout = true; }

	void ShowAllWindow();
	void HideAllWindow();
	void SetShowDemo(bool show) { m_show_demo = show; }
#ifdef UI_CFG_USE_IMPLOT
	void SetShowPlotDemo(bool show) { m_show_plot_demo = show; }
#endif
protected:
	void InitWindows();
	void InitMenus();
	void InitImDialog();
	void InitShortCut();
	void ResetLayout();

	void ShowDemo();
#ifdef UI_CFG_USE_IMPLOT
	void ShowPlotDemo();
#endif
protected:
	// ´°¿ÚÁÐ±í
	std::vector<UIBaseWindow*> m_win_list;

	UILogView* m_ui_log = nullptr;
	UISysSettings* m_ui_sys = nullptr;
	UIImageView* m_ui_image = nullptr;
	UIAIRobot* m_ui_airobot = nullptr;

	// window show
	bool m_show_demo = false;
#ifdef UI_CFG_USE_IMPLOT
	bool m_show_plot_demo = false;
#endif

	bool m_reset_layout = true;
};

using UIWindowManagerPtr = std::shared_ptr<UIWindowManager>;
