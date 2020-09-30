#ifndef _Base64Tool_H
#define _Base64Tool_H
#include <mutex>
#include <vector>
#include <lzUtils/base.h>
namespace cppUtils {
	enum class Base64ToolMode_t {
		MIN,
		EI_ENC,	//1:外部输入编码模式
		EI_DEC,	//2:外部输入解码模式
		RF_ENC, //3:读文件编码模式
		RF_DEC, //4:读文件解码模式
		MAX
	};

	class Base64ToolPar {
	public:
		const char *base64Char;
		const char *iPath;
		const char *oPath;
		size_t eniBytes;
		Base64ToolMode_t workMode;
		Base64ToolPar() {
			memset(this, 0, sizeof(Base64ToolPar));
			base64Char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789()";
			iPath = "./in.bin";
			oPath = "./out.bin";
			eniBytes = 16 * 1024;
			workMode = Base64ToolMode_t::EI_DEC;
		}
	};

	class Base64Tool {
		Base64ToolPar *mPar;
		size_t mEniBytes;
		size_t mEnoBytes;
		size_t mDeiBytes;
		size_t mDeoBytes;
		char *mEniBuf;
		char *mEnoBuf;
		char *mDeiBuf;
		char *mDeoBuf;
	public:
		static ssize_t GetEnSafeOutBytes(size_t bytes);
		static ssize_t GetDeSafeOutBytes(size_t bytes);
		Base64Tool(Base64ToolPar *par);
		~Base64Tool();
		size_t DataEncode(const char *bin, const size_t binSize);
		size_t DataDecode(const char *base64, const size_t baseSize);
		bool FileEncode(const char *iPath = NULL, const char *oPath = NULL);
		bool FileDecode(const char *iPath = NULL, const char *oPath = NULL);
		char *GetEncOutBuf() {
			return mEnoBuf;
		}
		char *GetDecOutBuf() {
			return mDeoBuf;
		}
	};
}
#endif