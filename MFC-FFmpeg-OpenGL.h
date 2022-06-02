
// MFC-FFmpeg-OpenGL.h: PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含 'pch.h' 以生成 PCH"
#endif

#include "resource.h"		// 主符号


// CMFCFFmpegOpenGLApp:
// 有关此类的实现，请参阅 MFC-FFmpeg-OpenGL.cpp
//

class CMFCFFmpegOpenGLApp : public CWinApp
{
public:
	CMFCFFmpegOpenGLApp();

// 重写
public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

extern CMFCFFmpegOpenGLApp theApp;
