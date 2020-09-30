#include "Base64Tool.h"
#include <thread>
#include <unistd.h>
#include <lzUtils/base.h>
#include <fstream>
using namespace cppUtils;
ssize_t Base64Tool::GetEnSafeOutBytes(size_t bytes) {
	return ((bytes) % 3) ? ((bytes) / 3 + 1) * 4 : (bytes) / 3 * 4;
}
ssize_t Base64Tool::GetDeSafeOutBytes(size_t bytes) {
	return (bytes) * 3 / 4;
}
Base64Tool::Base64Tool(Base64ToolPar *par) {
	memset(this, 0, sizeof(Base64Tool));
	mPar = par;
	mEniBytes = par->eniBytes;
	mEnoBytes = GetEnSafeOutBytes(mEniBytes) + 1;//为分隔符（换行符）留下空间
	if(mPar->workMode == Base64ToolMode_t::RF_ENC) {
		mEniBuf = new char[mEniBytes];
		if(!mEniBuf) {
			s_err("oom");
			return ;
		}
	}
	if(mPar->workMode == Base64ToolMode_t::EI_ENC ||
			mPar->workMode == Base64ToolMode_t::RF_ENC) {
		mEnoBuf = new char[mEnoBytes];
		if(!mEnoBuf) {
			s_err("oom");
			return ;
		}
	}
	mDeiBytes = mEnoBytes;
	mDeoBytes = GetDeSafeOutBytes(mDeiBytes);
	if(mPar->workMode == Base64ToolMode_t::RF_DEC) {
		mDeiBuf = new char[mDeiBytes];
		if(!mDeiBuf) {
			s_err("oom");
			return ;
		}
	}
	if(mPar->workMode == Base64ToolMode_t::EI_DEC ||
			mPar->workMode == Base64ToolMode_t::RF_DEC ) {
		mDeoBuf = new char[mDeoBytes];
		if(!mDeoBuf) {
			s_err("oom");
			return ;
		}
	}
	s_inf("mEniBytes:%u", mEniBytes);
	s_inf("mEnoBytes:%u", mEnoBytes);
	s_inf("mDeiBytes:%u", mDeiBytes);
	s_inf("mDeoBytes:%u", mDeoBytes);
}
Base64Tool::~Base64Tool() {
	s_trc(__func__);
	if(mEniBuf) {
		delete []mEniBuf;
	}
	if(mEnoBuf) {
		delete []mEnoBuf;
	}
	if(mDeiBuf) {
		delete []mDeiBuf;
	}
	if(mDeoBuf) {
		delete []mDeoBuf;
	}
}
size_t Base64Tool::DataEncode(const char *bin, const size_t binSize) {
	int i, j;
	char current;
	for ( i = 0, j = 0 ; i < binSize ; i += 3 ) {
		if(j + 4 > mEnoBytes) {
			s_err("out buf is't enough,alredy en %u bytes,output %u bytes !", i, j);
			return -1;
		}
		current = (bin[i] >> 2) ;
		current &= (char)0x3F;
		mEnoBuf[j++] = mPar->base64Char[(int)current];

		current = ( (char)(bin[i] << 4 ) ) & ( (char)0x30 );
		if ( i + 1 >= binSize ) {
			mEnoBuf[j++] = mPar->base64Char[(int)current];
			mEnoBuf[j++] = '=';
			mEnoBuf[j++] = '=';
			break;
		}
		current |= ( (char)(bin[i + 1] >> 4) ) & ( (char) 0x0F );
		mEnoBuf[j++] = mPar->base64Char[(int)current];

		current = ( (char)(bin[i + 1] << 2) ) & ( (char)0x3C ) ;
		if ( i + 2 >= binSize ) {
			mEnoBuf[j++] = mPar->base64Char[(int)current];
			mEnoBuf[j++] = '=';
			break;
		}
		current |= ( (char)(bin[i + 2] >> 6) ) & ( (char) 0x03 );
		mEnoBuf[j++] = mPar->base64Char[(int)current];

		current = ( (char)bin[i + 2] ) & ( (char)0x3F ) ;
		mEnoBuf[j++] = mPar->base64Char[(int)current];
	}
	return j;
}
size_t Base64Tool::DataDecode(const char *base64, const size_t baseSize) {
	int i, j;
	char k;
	char temp[4];
	if((int)((baseSize / 4 - 1) * 3) > (int)mDeiBytes) {
		s_err("out buf is't enough,alredy encode %u bytes,output %u bytes !", i, j);
		return -1;
	}
	for ( i = 0, j = 0; i < baseSize ; i += 4 ) {
		memset( temp, 0xFF, sizeof(temp) );
		for ( k = 0 ; k < 64 ; k ++ ) {
			if ( mPar->base64Char[k] == base64[i] )
				temp[0] = k;
		}
		for ( k = 0 ; k < 64 ; k ++ ) {
			if ( mPar->base64Char[k] == base64[i + 1] )
				temp[1] = k;
		}
		for ( k = 0 ; k < 64 ; k ++ ) {
			if ( mPar->base64Char[k] == base64[i + 2] )
				temp[2] = k;
		}
		for ( k = 0 ; k < 64 ; k ++ ) {
			if ( mPar->base64Char[k] == base64[i + 3] )
				temp[3] = k;
		}

		mDeoBuf[j++] = ((char)(((char)(temp[0] << 2)) & 0xFC)) |
					   ((char)((char)(temp[1] >> 4) & 0x03));
		if ( base64[i + 2] == '=' )
			break;

		mDeoBuf[j++] = ((char)(((char)(temp[1] << 4)) & 0xF0)) |
					   ((char)((char)(temp[2] >> 2) & 0x0F));
		if ( base64[i + 3] == '=' )
			break;

		mDeoBuf[j++] = ((char)(((char)(temp[2] << 6)) & 0xF0)) |
					   ((char)(temp[3] & 0x3F));
	}
	return j;
}
bool Base64Tool::FileEncode(const char *iPath, const char *oPath) {
	if(mPar->workMode != Base64ToolMode_t::RF_ENC) {
		s_err("only ENC mode can excute %s", __func__);
		return false;
	}
	const char *inPath = iPath ? iPath : mPar->iPath;
	const char *outPath = oPath ? oPath : mPar->oPath;
	s_inf("inPath:%s", inPath);
	s_inf("outPath:%s", outPath);
	std::ofstream ofs(outPath, std::ios::binary);
	if(!ofs.is_open()) {
		s_err("outPath:%s", outPath);
		show_errno(0, "ifs open");
		return false;
	}
	std::ifstream ifs(inPath, std::ios::binary);
	if(!ifs.is_open()) {
		s_err("inPath:%s", inPath);
		show_errno(0, "ifs open");
		ifs.close();
		return false;
	}
	ssize_t res = 0;
	do {
		ifs.read(mEniBuf, mEniBytes);
		res = ifs.gcount();
		if(res < mEniBytes) {
			if(!ifs.eof()) {
				show_errno(0, "ifs read");
				ifs.close();
				ofs.close();
				return false;
			}
		}
		s_inf("%s:gcount=%d", __func__, res);
		res = DataEncode(mEniBuf, res);
		if(res < 0) {
			s_err("DataEncode");
			ifs.close();
			ofs.close();
			return false;
		}
		s_inf("%s:write=%d", __func__, res);
		mEnoBuf[res] = '\n';
		ofs.write(mEnoBuf, res + 1);
		if(!ofs.good()) {
			show_errno(0, "ofs.write");
			ifs.close();
			ofs.close();
			return false;
		}
	} while(!ifs.eof());
	ifs.close();
	ofs.close();
	return true;
}
bool Base64Tool::FileDecode(const char *iPath, const char *oPath) {
	if(mPar->workMode != Base64ToolMode_t::RF_DEC ) {
		s_err("only RF_DEC mode can excute %s", __func__);
		return false;
	}
	const char *inPath = iPath ? iPath : mPar->iPath;
	const char *outPath = oPath ? oPath : mPar->oPath;
	s_inf("inPath:%s", inPath);
	s_inf("outPath:%s", outPath);
	std::ofstream ofs(outPath, std::ios::binary);
	if(!ofs.is_open()) {
		s_err("outPath:%s", outPath);
		show_errno(0, "ifs open");
		return false;
	}
	std::ifstream ifs(inPath, std::ios::binary);
	if(!ifs.is_open()) {
		s_err("inPath:%s", inPath);
		show_errno(0, "ifs open");
		ifs.close();
		return false;
	}
	ssize_t res = 0;
	do {
		ifs.getline(mDeiBuf, mDeiBytes);
		res = ifs.gcount();
		s_inf("%s:gcount=%d", __func__, res);
		if(res <= 1) {//考虑分隔符
			break;
		}
		res = DataDecode(mDeiBuf, res - 1);
		if(res < 0) {
			s_err("DataEncode");
			ifs.close();
			ofs.close();
			return false;
		}
		s_inf("%s:write=%d", __func__, res);
		ofs.write(mDeoBuf, res);
		if(!ofs.good()) {
			show_errno(0, "ofs.write");
			ifs.close();
			ofs.close();
			return false;
		}
	} while(!ifs.eof());
	ifs.close();
	ofs.close();
	return true;
}