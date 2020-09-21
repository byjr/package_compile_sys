#include "CrcTool.h"
using namespace cppUtils;

void CrcTool::GenerateCrc16Table() {
	CrcU16_t crc = 0;
	CrcU16_t i = 0, j = 0;
	for(j = 0; j < 256; j++) {
		crc = j;
		for (i = 0; i < 8; i++) {
			if (crc & 0x0001)       //由于前面和后面省去了反转，所以这里是右移，且异或的值为多项式的反转值
				crc = (crc >> 1) ^ 0xA001;//右移后与多项式反转后异或
			else                   //否则直接右移
				crc >>= 1;
		}
		mHi16Tbl[j] = (CrcU8_t)(crc & 0xff); //取低字节
		mLow16Tbl[j] = (CrcU8_t)((crc >> 8) & 0xff); //取高字节
	}
}

void CrcTool::GenerateCrc32Table() {
	CrcU32_t c = 0;
	CrcU32_t i = 0;
	int bit = 0;
	for(i = 0; i < 256; i++) {
		c  = i;
		for(bit = 0; bit < 8; bit++) {
			if(c & 1) {
				c = (c >> 1) ^ (0xEDB88320);
			} else {
				c =  c >> 1;
			}
		}
		mCrcU32Tbl[i] = c;
	}
}

CrcTool::CrcTool(CrcToolPar *par) {
	memset(this, 0, sizeof(CrcTool));
	mPar = par;
	if(mPar->workMode == CrcToolMode_t::RSTB_16) {
		mHi16Tbl = new CrcU8_t[256];
		if(!mHi16Tbl) {
			s_err("oom");
			return ;
		}
		mLow16Tbl = new CrcU8_t[256];
		if(!mLow16Tbl) {
			delete []mHi16Tbl;
			s_err("oom");
			return ;
		}
		GenerateCrc16Table();
	}
	if(mPar->workMode == CrcToolMode_t::RSTB_32) {
		mCrcU32Tbl = new CrcU32_t[256];
		if(!mCrcU32Tbl) {
			s_err("oom");
			return ;
		}
		GenerateCrc32Table();
	}
}
CrcTool::~CrcTool() {
	s_trc(__func__);
	if(mHi16Tbl) {
		delete []mHi16Tbl;
	}
	if(mLow16Tbl) {
		delete []mLow16Tbl;
	}
	if(mCrcU32Tbl) {
		delete []mCrcU32Tbl;
	}
}

CrcU16_t CrcTool::GetCrc16Val(const CrcU8_t *addr, size_t len) {
	s_inf("len=%u", len);
	CrcU8_t crcHi = 0x00;
	CrcU8_t crcLo = 0x00;
	CrcU8_t index = 0;
	CrcU16_t crc  = 0;
	for (; len > 0; len--) {
		index = crcLo ^ *(addr++);//低8位异或，得到表格索引值
		crcLo = crcHi ^ mHi16Tbl[index];
		crcHi = mLow16Tbl[index];
	}
	crc = (CrcU16_t)(crcHi << 8 | crcLo);
	return crc; //返回校验值
}

CrcU32_t CrcTool::GetCrc32Val(const CrcU8_t *data, size_t size) {
	CrcU32_t crc = 0xffffffff;
	while (size --)	{
		crc = (crc >> 8) ^ ( mCrcU32Tbl[(crc ^ * (data++)) & 0x000000ff] );
	}
	return crc;
}