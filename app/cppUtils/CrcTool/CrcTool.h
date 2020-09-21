#ifndef _CrcTool_H
#define _CrcTool_H
#include <lzUtils/base.h>
namespace cppUtils {
	typedef unsigned char CrcU8_t;
	typedef unsigned short CrcU16_t;
	typedef unsigned int CrcU32_t;
	enum class CrcToolMode_t {
		MIN,
		RSTB_16,
		RSTB_32,
		MAX
	};

	class CrcToolPar {
	public:
		const char *iPath;
		size_t crcBytes;
		CrcToolMode_t workMode;
		CrcToolPar() {
			workMode = CrcToolMode_t::RSTB_32;
		}
	};

	class CrcTool {
		CrcToolPar *mPar;
		CrcU8_t *mHi16Tbl;
		CrcU8_t *mLow16Tbl;
		CrcU32_t *mCrcU32Tbl;
		void GenerateCrc16Table();
		void GenerateCrc32Table();
	public:
		CrcTool(CrcToolPar *par);
		~CrcTool();
		CrcU16_t GetCrc16Val(const CrcU8_t *addr, size_t len);
		CrcU32_t GetCrc32Val(const CrcU8_t *addr, size_t len);
	};
}
#endif