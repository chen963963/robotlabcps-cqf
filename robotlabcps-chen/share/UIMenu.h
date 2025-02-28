#pragma once
#include <functional>
#include <string>
#include <memory>
#include <vector>
#include "UIBaseWindow.h"

// �˵�����
enum class EUIMenuType
{
	E_UI_MENU_WINDOW,  // ���ڲ˵���
	E_UI_MENU_FUNCTION,  // ���ܲ˵���
	E_UI_MENU_SUBMENU	// �Ӳ˵���
};


// ���˵���
struct UIMainMenu
{
	EUIMenuCategory cat;
	char name[64];
	bool show;
};

class UIMenuBase
{
public:
	UIMenuBase(EUIMenuType type, bool enabled = true, bool visible = true) : m_type(type), m_is_enabeld(enabled), m_is_visible(visible)
	{}
	virtual void OnClick() = 0;
	virtual EUIMenuCategory GetMenuCategory() = 0;
	virtual const char* GetMenuName() = 0;
	virtual const char* GetMenuShortCut() = 0;
	virtual bool* GetCheckVar() = 0;
	virtual void OnKeyEvent(const char* key) = 0;
public:
	EUIMenuType GetMenuType() { return m_type; }

	bool IsEnabled() { return m_is_enabeld; }
	void Enable() { m_is_enabeld = true; }
	void Disable() { m_is_enabeld = false; }

	bool IsVisible() { return m_is_visible; }
	void Show() { m_is_visible = true; }
	void Hide() { m_is_visible = false; }
protected:
	EUIMenuType m_type;
	bool m_is_enabeld;
	bool m_is_visible;
};
using UIMenuBasePtr = std::shared_ptr<UIMenuBase>;

// ���ڲ˵��ͨ���˵���ʾ/���ش��ڣ�
class UIWindowMenu: public UIMenuBase
{
public:
	UIWindowMenu(UIBaseWindow* win, bool enabled = true, bool visible = true);
	virtual void OnClick();
	virtual EUIMenuCategory GetMenuCategory();
	virtual const char* GetMenuName();
	virtual const char* GetMenuShortCut();
	virtual bool* GetCheckVar();
	virtual void OnKeyEvent(const char* key);
protected:
	UIBaseWindow* m_win;
};
using UIWindowMenuPtr = std::shared_ptr<UIWindowMenu>;

// ���ܲ˵���
class UIFunctionMenu : public UIMenuBase
{
public:
	UIFunctionMenu(EUIMenuCategory cat, std::function<void()> func,
		const char * name, const char * shortcut, bool enabled = true, bool visible = true);

	virtual void OnClick();
	virtual EUIMenuCategory GetMenuCategory();
	virtual const char* GetMenuName();
	virtual const char* GetMenuShortCut();
	virtual bool* GetCheckVar();
	virtual void OnKeyEvent(const char* key);
protected:
	EUIMenuCategory m_cat;
	std::function<void()> m_func;
	std::string m_name;
	std::string m_shorcut;
};
using UIFunctionMenuPtr = std::shared_ptr<UIFunctionMenu>;

// �Ӳ˵���
class UISubMenu : public UIMenuBase
{
public:
	UISubMenu(EUIMenuCategory cat, const char* name);

	virtual EUIMenuCategory GetMenuCategory() { return m_cat; }
	virtual const char* GetMenuName() { return m_name.c_str(); }
	virtual void OnKeyEvent(const char* key);

	void AddChildMenu(UIMenuBasePtr menu);

	// �Ӳ˵����½ӿڲ�����
	virtual void OnClick() {}
	virtual const char* GetMenuShortCut() { return nullptr; }
	virtual bool* GetCheckVar() { return nullptr; }
	
	void IterSubMenus(const std::function<void (UIMenuBasePtr)> & func);
protected:
	EUIMenuCategory m_cat;
	std::string m_name;
	std::vector< UIMenuBasePtr> m_sub_menus;
};
using UISubMenuPtr = std::shared_ptr<UISubMenu>;
