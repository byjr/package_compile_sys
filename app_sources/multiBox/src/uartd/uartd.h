#ifndef AML_UARTD_H_
#define AML_UARTD_H_
#include <stdio.h>
#include <string>
#include <vector>
#include <thread>
#include <stdlib.h>
#include "condFifo.h"
class ItemPar {
public:
	int argc;
	char **argv;
	void *args;
};
class CmdItem {
public:
	char *name;
	int (*handle)(ItemPar &par);
	void *args;
};
class UartdPar {
public:
	char *devNode;		//设备节点
	size_t baudRate;	//波特率
	size_t openMode;	//打开方式
	bool ttMasterFlag;	//是否启用测试模式
	char *ttHardPath;	//测试管道路径
};
class Uartd {
	UartdPar *mPar;					//启动参数
	int mUartdFd;					//串口句柄
	int mHardWriteFd;				//测试模式下写管道句柄
	int mHardReadFd;				//测试模式下读管道句柄
	CondFifo *wFifo;				//写入的串口命令队列
	CondFifo *rFifo;				//读到的串口命令队列
	std::thread rUartThread;		//读串口线程句柄
	std::thread wUartThread;		//写串口线程句柄
	std::thread parseThread;		//命令解析线程句柄
	std::thread ttMasterThread;		//测试模式下获取stdin命令的线程句柄
	int isHardReadable();			//串口可读性探测方法（用select来探测）
	int isHardWriteable();			//串口可写性探测方法（用select来探测）
	int setOpt(int nSpeed, int nBits, char nEvent, int nStop);
	//串口参数设置方法
	std::vector<CmdItem> mCmdVct;	//命令处理方法表入口
	volatile bool ClientExitFlag;	//客户端退出标志
public:
	Uartd(UartdPar *par);
	~Uartd();
	int HardReadProcess();			//读串口线程主函数
	int HardWriteProcess();			//写串口线程主函数
	int CmdparseProcess();			//命令解析线程主函数
	int TtMasterProcess();			//测试模式下获取stdin命令的线程主函数
	int sendCmd(const char *cmd);	//发送命令给主机的方法
	void setExitFlag() {				//设置退出标志的方法
		ClientExitFlag = true;
	}
	bool getExitFlag() {				//获取退出标志的方法
		return ClientExitFlag;
	}
};
#endif
