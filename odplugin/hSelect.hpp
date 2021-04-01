#pragma once
#ifndef __HSELECT_HPP__
#define __HSELECT_HPP__

#ifndef _WINDOWS_
#include <windows.h>
#endif
#include <tchar.h>
#include <commdlg.h>
#include <ShlObj.h>
#pragma comment(lib,"Shell32.lib")


class selFile {
public:
    selFile(HWND hWnd) {
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hWnd;
        ofn.hInstance = NULL;
        ofn.lpstrFilter = _T("Portable Network Graphics(*.png)\0*.png\0")
            _T("Joint Photographic experts Group(*.jpg)\0*.jpg\0")
            _T("Graphics Interchange Format(*.gif)\0*.gif\0")
            _T("Bitmap Files(*.bmp)\0*.bmp\0")
            _T("All Files\0*.*\0\0");
        ofn.lpstrCustomFilter = NULL;
        ofn.nMaxCustFilter = 0;
        ofn.nFilterIndex = 0;
        ofn.lpstrFile = NULL;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = MAX_PATH;
        ofn.lpstrInitialDir = NULL;
        ofn.lpstrTitle = NULL;
        ofn.Flags = 0;
        ofn.nFileOffset = 0;
        ofn.nFileExtension = 0;
        ofn.lpstrDefExt = TEXT("jpg");
        ofn.lCustData = 0;
        ofn.lpfnHook = NULL;
        ofn.lpTemplateName = NULL;
    }
    LPCTSTR open() {
        buf[0] = _T('\0');
        TCHAR tTitle[MAX_PATH] = _T("打开文件");
        ofn.lpstrFile = buf;
        ofn.lpstrFileTitle = tTitle;
        ofn.Flags = 0;
        if (GetOpenFileName(&ofn)) return buf;
        return NULL;
    }
    LPCTSTR save() {
        buf[0] = _T('\0');
        TCHAR tTitle[MAX_PATH] = _T("另存为");
        ofn.lpstrFile = buf;
        ofn.lpstrFileTitle = tTitle;
        ofn.Flags = OFN_OVERWRITEPROMPT;
        if (GetSaveFileName(&ofn)) return buf;
        return NULL;
    }
private:
    OPENFILENAME ofn;
    TCHAR buf[MAX_PATH];
};

class selFolder {
public:
    selFolder(HWND hWnd) {
        bi.hwndOwner = hWnd;
        bi.pidlRoot = NULL;
        bi.pszDisplayName = buf;//此参数如为NULL则不能显示对话框
        bi.lpszTitle = _T("指定目录");
        bi.ulFlags = 0;
        bi.lpfn = NULL;
        bi.iImage = NULL;
        bi.lParam = 0;
    }
    LPCTSTR get() {
        BOOL bRet = FALSE;
        LPITEMIDLIST pIDList = SHBrowseForFolder(&bi);//调用显示选择对话框
        if (pIDList) {
            SHGetPathFromIDList(pIDList, buf);
            bRet = TRUE;
        }

        LPMALLOC lpMalloc;
        if (FAILED(SHGetMalloc(&lpMalloc))) return FALSE;

        //释放内存
        lpMalloc->Free(pIDList);
        lpMalloc->Release();

        if (bRet) return buf;
        return NULL;
    }
private:
    BROWSEINFO bi;
    TCHAR buf[MAX_PATH];
};

#endif //__HSELECT_HPP__