// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "Plugin.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hSelect.hpp"

//OD主界面句柄
HWND g_hOllyDbg;
CHAR g_buffer[0x1000];

//************************************
// Method:菜单中显示的插件名
// Description: 必须的导出函数
//************************************
extern "C" __declspec(dllexport) cdecl int ODBG_Plugindata(char* shortname)
{
	const char* pluginName = "小钟_插件";
	strcpy_s(shortname, strlen(pluginName) + 1, pluginName);
	return PLUGIN_VERSION;
}

//************************************
// Method:插件初始化，用于判断当前OD版本号和插件所支持的版本是否一致
// Description:必须的导出函数 
//************************************
extern "C" __declspec(dllexport) cdecl int ODBG_Plugininit(int ollydbgversion, HWND hw, ulong * features)
{

	if (ollydbgversion < PLUGIN_VERSION)
	{
		MessageBox(hw, "本插件不支持当前版本OD!", "MyFirstPlugin", MB_ICONERROR);
		return -1;
	}
	g_hOllyDbg = hw;
	return 0;
}

//************************************
// Method:显示菜单项
// Description:显示对应的菜单选项
//************************************
extern "C" __declspec(dllexport) cdecl int  ODBG_Pluginmenu(int origin, TCHAR data[4096], VOID * item)
{
	// 主窗口菜单
	if (origin == PM_MAIN)
	{
		strcpy(data, "0&顶部菜单子菜单一,1&顶部菜单子菜单二");
	}

	if (origin == PM_MEMORY || origin == PM_CPUDUMP /*|| origin == PM_CPUSTACK || origin == PM_CPUREGS*/)
	{
		strcpy(data, "0&复制指定字节数,1&将选中的数据保存到文件");
	}

	// 反汇编窗口菜单
	if (origin == PM_DISASM)
	{
		strcpy(data, "鼠标右键主菜单{0&修改E8CALL函数名称,1&鼠标右键子菜单二{2&二级菜单1,3&二级菜单2}}");
	}
	return 1;
}

// 第二个参数是起始字符串
BOOL str_isBeginWith(PCHAR str1, PCCH str2)
{
	DWORD len1 = strlen(str1);
	DWORD len2 = strlen(str2);
	if (len2 > len1)
		return FALSE;
	if (strncmp(str1, str2, len2) == 0) return TRUE;
	else return FALSE;
}

VOID renameCall(PVOID item)
{
	t_dump* ptdump = (t_dump*)item;
	ULONG selectedAddr = ptdump->sel0;

	// 如果选择的地址是0
	if (selectedAddr == 0) return;

	// 如果选中地址的第一个字节不是E8（FF CALL也不要）
	UCHAR pbuffer[MAXCMDSIZE];
	_Readmemory(pbuffer, selectedAddr, MAXCMDSIZE, MM_SILENT);
	if (*pbuffer != 0xe8) return;

	// 反汇编获取地址
	t_disasm td;
	ULONG sizeofCode = _Disasm(pbuffer, MAXCMDSIZE, selectedAddr, NULL, &td, DISASM_ALL, NULL);
	// 如果发现不是一个call那么直接返回
	if (!str_isBeginWith(td.result, "call")) return;

	// 获取E8 call后面call的地址
	INT absAddr = 0;
	_Readmemory(&absAddr, selectedAddr + 1, 4, MM_SILENT); // 读取相对地址
	// 将相对地址转换为绝对地址
	absAddr = absAddr + selectedAddr + 5;
	sprintf(g_buffer, "尝试修改%x地址的标签", absAddr);
	_Addtolist(absAddr, 1, g_buffer);

	// 设置标签
	CHAR szUserInput[TEXTLEN] = { 0 };
	_Findlabel(absAddr, szUserInput);
	// 获取用户输入
	if (_Gettext("标题", szUserInput, 0, NM_NONAME, 0) != -1)
	{
		// 设置函数开头地址的标签
		_Insertname(absAddr, NM_LABEL, szUserInput);
	}
}

VOID saveToFile(PVOID item)
{
	t_dump* ptdump = (t_dump*)item;
	selFile t(g_hOllyDbg);
	const char* t_filepath = t.save();
	FILE* fp = fopen((PCHAR)t_filepath, "w");

	// 如果没有输入直接返回
	if (!t_filepath)
	{
		MessageBox(0, 0, 0, 0);
		return;
	}
	_Addtolist((0), 1, (char*)t_filepath);

	// 计算需要保存的长度
	DWORD sizeToSave = ptdump->sel1 - ptdump->sel0;
	PVOID t_buffer = malloc(sizeToSave);
	// 读取数据
	_Readmemory(t_buffer, ptdump->sel0, sizeToSave, MM_SILENT);
	// 写入
	fwrite(t_buffer, sizeToSave, 1, fp);
	// 提示保存成功以及保存的字节数
	sprintf(g_buffer, "成功往%s写入%d字节数据\n", t_filepath, sizeToSave);
	MessageBox(g_hOllyDbg, g_buffer, "保存成功", 0);
	fflush(fp);
	fclose(fp); // 关闭文件指针
}

VOID selectData(PVOID item)
{
	t_dump* ptdump = (t_dump*)item;
	if (_Gettext("请输入需要选择的长度（十六进制）", g_buffer, 0, NM_NONAME, 0) != -1)
	{
		DWORD t_sizeToSelect = 0;
		int len = strlen(g_buffer);
		for (int i = 0; i < len; i++)
		{
			if (isdigit(g_buffer[i])) continue;
			if (g_buffer[i] - 'A' >= 0 && g_buffer[i] - 'Z' <= 0)
			{
				g_buffer[i] = tolower(g_buffer[i]);
			}
			if (g_buffer[i] - 'a' >= 0 && g_buffer[i] - 'f' <= 'f' - 'a') continue;
			// 如果输入非法，直接返回
			MessageBox(g_hOllyDbg, "转换出错！", "输入的字符非法！", 0);
			return;
		}
		sscanf(g_buffer, "%x", &t_sizeToSelect);
		// 修改选择长度，sel0是开始位置，sel1是结束位置
		ptdump->sel1 = ptdump->sel0 + t_sizeToSelect;
		// 刷新表格
		_Tablefunction(&ptdump->table, ptdump->table.hw, WM_USER_CHALL, NULL, NULL);
	}
}

//************************************
// Method:菜单项被点击执行函数
// Description:所有的菜单项被点击都会执行到这个函数
//************************************
extern "C" __declspec(dllexport) cdecl void ODBG_Pluginaction(int origin, int action, VOID * item)
{
	//如果是在主窗口点击
	if (origin == PM_MAIN)
	{
		if (action == 0)
		{
		}
		if (action == 1)
		{
		}
	}

	// 如果是在内存窗口
	if (origin == PM_CPUDUMP)
	{
		if (action == 0)
		{
			selectData(item);
		}
		else if (action == 1)
		{
			saveToFile(item);
		}
	}

	//如果是在反汇编窗口点击
	if (origin == PM_DISASM)
	{
		if (action == 0)
		{
			renameCall(item);
		}
		if (action == 3)
		{
		}
		if (action == 1)
		{
		}
	}
}

