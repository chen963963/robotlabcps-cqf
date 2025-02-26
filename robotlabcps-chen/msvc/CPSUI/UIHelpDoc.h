#pragma once
#include "UIBaseWindow.h"
#include <unordered_map>
#include "UIImage.h"

#ifdef UI_CFG_BUILD_DOC


class UIHelpDoc :
    public UIBaseWindow
{
public:
	UIHelpDoc(UIGLWindow* main_win, const char* title);
	~UIHelpDoc();

	virtual void Draw();

	virtual EUIMenuCategory GetWinMenuCategory() { return EUIMenuCategory::E_UI_CAT_HELP; }

	bool HasImage(const std::string& name);
	void AddImage(const std::string& name, UIImageBasePtr image);
	UIImageBasePtr GetImage(const std::string& name);
protected:
	void ShowContent(const std::string & title);
protected:
	std::string m_sct;
	std::unordered_map<std::string, UIImageBasePtr> m_map_images;
	std::unordered_map<std::string, std::string> m_map_content;
};

#endif