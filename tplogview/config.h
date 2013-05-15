#pragma once

#include "common.h"
#include "filter.h"

struct hilighter
{
	std::wstring name;
	filter* f;
	disp_info d;
};

typedef std::vector<hilighter> hilighters_t;

struct quickfilter
{
	UINT level;
	std::wstring tags;
	std::wstring text;
};


struct config
{
	struct ui_cfg
	{
		struct list_cfg
		{
			bool show_gridline;
			std::wstring calldepth_sign;
		} list;
		COLORREF theme_color;
		COLORREF theme_color_offline;
		int default_filter_cfg_page;
		struct
		{
			bool show_menubar;
			bool show_toolbar;
			bool show_filterbar;
			bool show_statusbar;
		} placement;
		struct
		{
			std::wstring logfile;
			std::wstring filter;
			std::wstring script;
		} savedpath;
		strlist_t recent_files;
		struct
		{
			bool enable;
			COLORREF mark_color;
			int maxinterval;
		} perfmark;
	} ui;

	struct sys_cfg
	{
		bool confirm_on_exit;
		time_t last_check_update_time;
	} syscfg;

	struct search_cfg
	{
		std::wstring history;
	}search;

	filter* log_filter;
	quickfilter log_quickfilter;
	hilighters_t hls;

	bool first_time_run;

	std::wstring product_name;
};

class CConfig
{
public:
	static CConfig* Instance();

	config& GetConfig();

	// TODO 使用serializer
	bool Load(LPCWSTR pszFileName);
	bool Save();

	CStringW GetConfigFilePath() const;
	static CStringW GetDefaultConfigFilePath();

private:
	CConfig();
	~CConfig();

	// 为没有设置的值赋缺省值
	// 这个动作需要在load完后做，而不是在构造函数中做
	// 因为旧版本已经把一些变量存成了空值
	void SetEmptyValueDefaults();

	config m_cfg;
	CStringW m_strConfigFilePath;
};

#define CFG CConfig::Instance()->GetConfig()
