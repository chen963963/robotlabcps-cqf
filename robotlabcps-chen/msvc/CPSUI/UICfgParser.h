#pragma once
#include "UICfg.h"
#include "UIHelpDef.h"

class UICfgParser
{
public:
	static UICfgParser* Inst();

	bool LoadCfg();

public:
	// bus log cfg
	ST_BusLogCfg m_cpscfg = { 0 };
	// ui app id
	int m_app_id = -1;
	// help tree
	ST_UIHelpTree m_help_tree;

private:
	UICfgParser() = default;
};

// 全局静态变量
static UICfgParser * g_cfg = UICfgParser::Inst();
