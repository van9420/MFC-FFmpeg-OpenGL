
// MFC-FFmpeg-OpenGLDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MFC-FFmpeg-OpenGL.h"
#include "MFC-FFmpeg-OpenGLDlg.h"
#include "afxdialogex.h"
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
};

#define ATTRIB_VERTEX 3
#define ATTRIB_TEXTURE 4


// 摄像机视频流的宽高
//#define WIDTH 2560
//#define HEIGHT 1920

#define WIDTH 1920
#define HEIGHT 960

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCFFmpegOpenGLDlg 对话框



CMFCFFmpegOpenGLDlg::CMFCFFmpegOpenGLDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCFFMPEGOPENGL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCFFmpegOpenGLDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMFCFFmpegOpenGLDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDOK, &CMFCFFmpegOpenGLDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CMFCFFmpegOpenGLDlg 消息处理程序

BOOL CMFCFFmpegOpenGLDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// 获取DC句柄
    hrenderDC = GetDlgItem(IDC_VIDEO)->GetDC()->m_hDC;

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCFFmpegOpenGLDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCFFmpegOpenGLDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/// <summary>
/// 读取文件内容
/// </summary>
/// <param name="filename"></param>
/// <returns></returns>
char* CMFCFFmpegOpenGLDlg::textFileRead(char* filename)
{
    char* s = (char*)malloc(8000);
    memset(s, 0, 8000);
    FILE* infile = fopen(filename, "rb");
    int len = fread(s, 1, 8000, infile);
    fclose(infile);
    s[len] = 0;
    return s;
}

/// <summary>
/// 初始化 Shader
/// </summary>
void CMFCFFmpegOpenGLDlg::InitShaders()
{
    glewInit();         //此函数初始化后，我们就可以在之后的代码里面方便地使用相关的gl函数。

    GLint vertCompiled, fragCompiled, linked;

    /*-----------------------------（1）  创建一个Shader对象-----------------------------------------*/
    //（1）创建Shader对象（着色器）
    GLint v, f;
    v = glCreateShader(GL_VERTEX_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER);
    //（2）读取文件内容,GLSL的语法
    const char* vs, * fs;
    vs = textFileRead("Shader.vsh");    //Vertex Shader（顶点着色器）
    fs = textFileRead("Shader.fsh");    //Fragment Shader（片元着色器）
    //（3）给Shader实例指定源码。
    glShaderSource(v, 1, &vs, NULL);
    glShaderSource(f, 1, &fs, NULL);
    //（4）编译Shader对象
    glCompileShader(v);
    glCompileShader(f);
    //（5）从Shader对象返回一个参数，用来检测着色器编译是否成功。
    glGetShaderiv(v, GL_COMPILE_STATUS, &vertCompiled);
    glGetShaderiv(f, GL_COMPILE_STATUS, &fragCompiled);

    /*-------------------------------（2）  创建一个Program对象--------------------------------------*/
    //（1）创建program对象（类似于C链接器）
    p = glCreateProgram();
    //（2）绑定shader到program对象中
    glAttachShader(p, v);
    glAttachShader(p, f);
    //（3）将顶点属性（位置、纹理、颜色、法线）索引与着色器中的变量名进行绑定，
    //     如此一来就不需要在着色器中定义该变量（会提示重定义），直接使用即可
    glBindAttribLocation(p, ATTRIB_VERTEX, "vertexIn");
    glBindAttribLocation(p, ATTRIB_TEXTURE, "textureIn");   //vertexIn与textureIn是Shader.vsh文件中定义的变量名
    //（3）连接program对象
    glLinkProgram(p);
    //（4）从program对象返回一个参数的值，用来检测是否连接成功。【linked=1：OK】
    glGetProgramiv(p, GL_LINK_STATUS, &linked);
    //（5）使用porgram对象
    glUseProgram(p);
    // 在程序对象成功链接之前，分配给统一变量的实际位置是不知道的。
    //（6）发生链接后，命令glGetUniformLocation可用于获取统一变量的位置
    uniform_y = glGetUniformLocation(p, "tex_y");
    uniform_u = glGetUniformLocation(p, "tex_u");
    uniform_v = glGetUniformLocation(p, "tex_v"); //tex_y、tex_u和tex_v是Shader.fsh文件中定义的变量名

    /*---------------------------------（3）  初始化Texture-----------------------------------------*/
    //（1）定义顶点及纹理数组
    static const GLfloat vertexVertices[] = {
            -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f,  1.0f,
            1.0f,  1.0f,
    };
    static const GLfloat textureVertices[] = {
            0.0f,  1.0f,
            1.0f,  1.0f,
            0.0f,  0.0f,
            1.0f,  0.0f,
    };
    //（2）设置通用顶点属性数组
    glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, vertexVertices);
    //（3）启用属性数组
    glEnableVertexAttribArray(ATTRIB_VERTEX);
    glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, textureVertices);
    glEnableVertexAttribArray(ATTRIB_TEXTURE);
    //（4）初始化纹理【1:用来生成纹理的数量,&texture_y:存储纹理索引的数组】
    glGenTextures(1, &texture_y);
    //（5）绑定纹理，才能对该纹理进行操作
    glBindTexture(GL_TEXTURE_2D, texture_y);
    //（6）纹理过滤函数：确定如何把纹理象素映射成像素
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);       //【GL_TEXTURE_MAG_FILTER: 放大过滤，GL_LINEAR: 线性过滤, 使用距离当前渲染像素中心最近的4个纹素加权平均值.】
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);       //【GL_TEXTURE_MIN_FILTER: 缩小过滤】
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);    //【GL_TEXTURE_WRAP_S: S方向上的贴图模式，GL_CLAMP_TO_EDGE：边界处采用纹理边缘自己的的颜色，和边框无关】
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);    //【GL_TEXTURE_WRAP_T: T方向上的贴图模式】

    glGenTextures(1, &texture_u);
    glBindTexture(GL_TEXTURE_2D, texture_u);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &texture_v);
    glBindTexture(GL_TEXTURE_2D, texture_v);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

/// <summary>
/// 设置像素格式,为OpenGL与HDC连接做准备
/// </summary>
/// <param name="hDC">设备上下文(dc)的句柄</param>
/// <returns></returns>
BOOL CMFCFFmpegOpenGLDlg::SetWindowPixelFormat(HDC hDC)
{
    PIXELFORMATDESCRIPTOR pixelDesc;                // 像素格式
    pixelDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);// 结构大小
    pixelDesc.nVersion = 1;                         // 结构版本
    pixelDesc.dwFlags = PFD_DRAW_TO_WINDOW |        // (PFD_DRAW_TO_WINDOW)  绘制到窗口，
        PFD_SUPPORT_OPENGL |                        // (PFD_SUPPORT_OPENGL)  绘制到窗口支持opengl
        PFD_DOUBLEBUFFER |                          // (PFD_DOUBLEBUFFER)    告知OpenGL如何处理像素,采用双缓冲
        PFD_TYPE_RGBA;                              // (PFD_TYPE_RGBA)       颜色模式，像素类型 RGBA
    pixelDesc.iPixelType = PFD_TYPE_RGBA;           // 颜色模式，像素类型 RGBA
    pixelDesc.cColorBits = 32;                      // 颜色位数
    pixelDesc.cRedBits = 0;
    pixelDesc.cRedShift = 0;
    pixelDesc.cGreenBits = 0;
    pixelDesc.cGreenShift = 0;
    pixelDesc.cBlueBits = 0;
    pixelDesc.cBlueShift = 0;
    pixelDesc.cAlphaBits = 0;                       // RGBA颜色缓存中Alpha的位数
    pixelDesc.cAlphaShift = 0;                      // 现不支持置为0
    pixelDesc.cAccumBits = 0;                       // 累计缓存的位数
    pixelDesc.cAccumRedBits = 0;
    pixelDesc.cAccumGreenBits = 0;
    pixelDesc.cAccumBlueBits = 0;
    pixelDesc.cAccumAlphaBits = 0;
    pixelDesc.cDepthBits = 0;                       // 深度缓冲区位数
    pixelDesc.cStencilBits = 1;                     // 模板缓冲位数
    pixelDesc.cAuxBuffers = 0;                      // 辅助缓存为主
    pixelDesc.iLayerType = PFD_MAIN_PLANE;          // 层面类型:主层
    pixelDesc.bReserved = 0;
    pixelDesc.dwLayerMask = 0;
    pixelDesc.dwVisibleMask = 0;
    pixelDesc.dwDamageMask = 0;


    int PixelFormat;                                    // 选择匹配像素格式，并返回索引
    PixelFormat = ChoosePixelFormat(hDC, &pixelDesc);   // 匹配像素格式的索引
    if (PixelFormat == 0) // Choose default
    {
        PixelFormat = 1;
        if (DescribePixelFormat(hDC, PixelFormat,
            sizeof(PIXELFORMATDESCRIPTOR), &pixelDesc) == 0)
        {
            return FALSE;
        }
    }
    if (SetPixelFormat(hDC, PixelFormat, &pixelDesc) == FALSE)//设置像素格式
    {
        return FALSE;
    }
    return TRUE;
}

/// <summary>
/// 创建OpenGL渲染上下文，使得OpenGL可在上面绘制
/// </summary>
/// <param name="hDC"></param>
/// <returns></returns>
BOOL CMFCFFmpegOpenGLDlg::CreateViewGLContext(HDC hDC)
{
    // 创建一个新的OpenGL渲染描述表
    HGLRC hrenderRC = wglCreateContext(hDC);
    if (hrenderRC == NULL)
        return FALSE;
    // 设定OpenGL当前线程的渲染环境
    if (wglMakeCurrent(hDC, hrenderRC) == FALSE)
        return FALSE;
    return TRUE;
}

void CMFCFFmpegOpenGLDlg::Display()
{
    //视频流地址
    //char* url = "rtsp://192.168.1.88:554/11";
    char* url = "video-h265.mkv";//本地视频地址，格式HEVC(H265)。因为我IP摄像机获取的视频流就是这格式
    int ret;
    /***********************************************获取码流参数信息**********************************************************/
    AVFormatContext* fmt_ctx = NULL;    //包含码流参数较多的结构体
    AVDictionary* options = NULL;       //健值对存储工具，类似于c++中的map
    AVFrame* frame = NULL;

    AVCodecContext* pCodecCtx = NULL;   // 解码器上下文   

    //参数设置解析
    av_dict_set(&options, "buffer_size", "1044000", 0); //buffer_size：减少卡顿或者花屏现象，相当于增加或扩大了缓冲区，给予编码和发送足够的时间
    av_dict_set(&options, "stimeout", "20000000", 0);   //stimeout：设置超时断开，在进行连接时是阻塞状态，若没有设置超时断开则会一直去阻塞获取数据，单位是微秒。
    av_dict_set(&options, "rtsp_transport", "tcp", 0);  //rtsp_transport：修改优先连接发送方式，可以用udp、tcp、rtp
    av_dict_set(&options, "tune", "zerolatency", 0);    //zerolatency:转码延迟，以牺牲视频质量减少时延    
    /***
    * 打开多媒体数据并且获取一些参数信息
    * ps：函数调用成功之后处理过的AVFormatContext结构体。
    * file：打开的视音频流的URL。
    * fmt：强制指定AVFormatContext中AVInputFormat的。这个参数一般情况下可以设置为NULL，这样FFmpeg可以自动检测AVInputFormat。
    * dictionay：附加的一些选项，一般情况下可以设置为NULL。
    **/
    ret = avformat_open_input(&fmt_ctx, url, NULL, &options);
    if (ret < 0)
    {
        fprintf(stderr, "Could not open input\n");
        goto end;
    }
    /***
    * 读取一部分视音频数据并且获得一些相关的信息(AVStream)
    * ic：输入的AVFormatContext。
    * options：额外的选项，目前没有深入研究过。
    **/
    ret = avformat_find_stream_info(fmt_ctx, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not find stream information\n");
        goto end;
    }
    /***
    * 获取音视频对应的流索引(stream_index)
    **/
    AVCodec* pCodec = NULL;             // 存储编解码器的结构体
    ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &pCodec, 0);
    if (ret < 0)
    {
        fprintf(stderr, "Cannot find a video stream in the input file\n");
        goto end;
    }

    int video_stream = ret;             // 视频对应流索引

    /***********************************************获取解码器并解码*********************************************************/
    pCodecCtx = avcodec_alloc_context3(pCodec);                                           // 配置解码器,申请AVCodecContext空间。需要传递一个编码器，也可以不传，但不会包含编码器。
    avcodec_parameters_to_context(pCodecCtx, fmt_ctx->streams[video_stream]->codecpar);   // 该函数用于将流里面的参数，也就是AVStream里面的参数直接复制到AVCodecContext的上下文当中。
    
    ret = avcodec_open2(pCodecCtx, pCodec, NULL);                                         // 初始化一个视音频编解码器的AVCodecContext
    if (ret < 0)
    {
        fprintf(stderr, "Cannot open decode\n");
        goto end;
    }

    AVPacket packet;                    // 解码前的音频或者视频数据
    frame = av_frame_alloc();           // 用来存储解码后的（或原始）音频或视频数据
                                        // 必须由av_frame_alloc()分配内存，同时必须由av_frame_free()释放
    
    // yuv420p是临近的4个像素（一个2x2正方形）共用一个色差分量u和另一个色差分量v，亮度分量y则是每个像素各用各自的。
    // 算下来，一组2x2的像素，需要4个亮度y，1个色差u，1个色差v，共6个字节，平均每像素1.5字节。
    uint8_t y[WIDTH * HEIGHT];
    uint8_t u[WIDTH * HEIGHT / 4];
    uint8_t v[WIDTH * HEIGHT / 4];
    int i;
    while (true)
    {
        
        if ((ret = av_read_frame(fmt_ctx, &packet)) < 0)   //读取码流中的音频若干帧或者视频一帧
            break;
        if (video_stream == packet.stream_index)
        {
            ret = avcodec_send_packet(pCodecCtx, &packet); //发送视频一帧到解码器中
            if (ret < 0)
            {
                av_packet_unref(&packet);           // 将缓存空间的引用计数-1，并将Packet中的其他字段设为初始值。如果引用计数为0，自动的释放缓存空间。
                avcodec_flush_buffers(pCodecCtx);   // 清空内部缓存的帧数据
                continue;
            }
            else 
            {
                ret = avcodec_receive_frame(pCodecCtx, frame);    // 从解码器中获取解码的输出数据。ret=0：成功，返回一帧数据
                if (ret < 0)
                {
                    av_packet_unref(&packet);           // 将缓存空间的引用计数-1，并将Packet中的其他字段设为初始值。如果引用计数为0，自动的释放缓存空间。
                    continue;
                }
                /*****************使用OpenGL绘制yuv420p******************/
             
                //把解码后的AVFrame数据复制到为OpenGL准备的3个缓冲区里
                for (i = 0; i < frame->height; i++)
                    memcpy(y + i * frame->width, frame->data[0] + i * frame->linesize[0], frame->width);
                for (i = 0; i < frame->height / 2; i++)
                    memcpy(u + i * frame->width / 2, frame->data[1] + i * frame->linesize[1], frame->width / 2);
                for (i = 0; i < frame->height / 2; i++)
                    memcpy(v + i * frame->width / 2, frame->data[2] + i * frame->linesize[2], frame->width / 2);

                glClearColor(0.0, 0.0, 0.0, 0.0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glLoadIdentity();

                //（1）使用glActiveTexture()选择可以由纹理函数进行修改的当前纹理单位
                glActiveTexture(GL_TEXTURE0);
                //（2）接着使用glBindTexture()告诉OpenGL下面对纹理的任何操作都是针对它所绑定的纹理对象的
                glBindTexture(GL_TEXTURE_2D, texture_y);
                //（3）使用glTexImage2D()根据指定的参数，生成一个2D纹理（Texture）
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, WIDTH, HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, y);
                //（4）使用glUniform()为当前程序对象指定Uniform变量的值
                glUniform1i(uniform_y, 0);

                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, texture_u);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, WIDTH / 2, HEIGHT / 2, 0, GL_RED, GL_UNSIGNED_BYTE, u);
                glUniform1i(uniform_u, 1);

                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, texture_v);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, WIDTH / 2, HEIGHT / 2, 0, GL_RED, GL_UNSIGNED_BYTE, v);
                glUniform1i(uniform_v, 2);

                //（5）提供绘制功能。当采用顶点数组方式绘制图形时，使用该函数。该函数根据顶点数组中的坐标数据和指定的模式，进行绘制。
                //【arg1：绘制方式，arg2：从数组缓存中的哪一位开始绘制，一般为0，arg3：数组中顶点的数量】
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                //采用双缓冲技术绘制
                SwapBuffers(hrenderDC);

                Sleep(30u);
            }
        }
        av_packet_unref(&packet);
    }
    av_packet_unref(&packet);

    avcodec_flush_buffers(pCodecCtx);
    avcodec_close(pCodecCtx);  //close,如果为rk3399的硬件编解码,则需要等待MPP_Buff释放完成后再关闭?是否需要这样不知道

end:
    if (frame != NULL) 
        av_frame_free(&frame);
    if (fmt_ctx != NULL) 
        avformat_close_input(&fmt_ctx);
    if (pCodecCtx != NULL) 
        avcodec_free_context(&pCodecCtx);
}

void CMFCFFmpegOpenGLDlg::ThreadDisplay()
{
    if (SetWindowPixelFormat(hrenderDC) == FALSE)
        return;

    if (CreateViewGLContext(hrenderDC) == FALSE)
        return;

    InitShaders();

    Display();
}

static DWORD WINAPI PlayThread(LPVOID lpParam)
{
    CMFCFFmpegOpenGLDlg* dlg = (CMFCFFmpegOpenGLDlg*)lpParam;

    dlg->ThreadDisplay();

    return 0;
}

void CMFCFFmpegOpenGLDlg::OnBnClickedOk()
{
    HANDLE hThread = CreateThread(NULL, 0, PlayThread, this, 0, NULL);
    CloseHandle(hThread);
}
