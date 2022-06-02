
// MFC-FFmpeg-OpenGLDlg.h: 头文件
//

#pragma once
#include "glew.h"
#include "glut.h"

// CMFCFFmpegOpenGLDlg 对话框
class CMFCFFmpegOpenGLDlg : public CDialogEx
{
// 构造
public:
	CMFCFFmpegOpenGLDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCFFMPEGOPENGL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

private:
	GLuint p;
	GLuint texture_y, texture_u, texture_v; // 纹理id
	GLuint uniform_y, uniform_u, uniform_v;


	FILE* video_file;	// YUV视频文件

	// plane[3]分别表示YUV三个缓存区
	// 如果图像的宽高分别是w和h，yuv420p的数据排列如下：
	// 1，y在前面，共wh字节。
	// 2，接着u，共wh / 4字节，宽高都是y的一半。
	// 3，接着v，与u一样。
	// 4，如果是视频，则一帧帧按顺序排，第一帧的yuv，第二帧的yuv，...，最后一帧的yuv，每一帧都是wh * 3 / 2字节。
	// 格式非常规整，容易记。
	unsigned char* plane[3];

	HDC hrenderDC;
private:
	char* textFileRead(char* filename);
	void InitShaders();

	BOOL SetWindowPixelFormat(HDC hDC);
	BOOL CreateViewGLContext(HDC hDC);
public:
	void ThreadDisplay();
	void Display();


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};
