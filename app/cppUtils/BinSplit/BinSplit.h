#ifndef _BIN_SPLIT_H
#define _BIN_SPLIT_H
#include <lzUtils/base.h>
#include <vector>
namespace cppUtils {
	class NaluHead {
	public:
		const char *data;
		size_t size;
		size_t preSize;
	};
	class BinSplitPar {
	public:
		const char *iPath;
		size_t bytesPerRead;
		std::vector<NaluHead *> nuhVct;
		std::vector<size_t> nuSizeVct;
	};

	class BinSplit {
		BinSplitPar *mPar;
		FILE *mFp;
		char *mBuf;
		NaluHead *mOldNuh;
		NaluHead *mNewNuh;
		NaluHead *mMaxNuh;
		size_t mMaxNuhSize;
		bool checkAndPreread();
		void getPreSize(const char *buf, size_t size, NaluHead *nuh);
		size_t getMaxNaluhSzie();
		bool updateMaxNuh(const char *buf, size_t size);
		char *findNextNalu(const char *buf, size_t size);
		void RunSplit();
		virtual void nuhVctInit(std::vector<NaluHead *> &nuhVct) {}
	public:
		BinSplit(BinSplitPar *par);
		~BinSplit();
	};
	class H264SplitPar: public BinSplitPar {};
	class H264Split: public BinSplit {
		NaluHead mH264Nuh1;
		NaluHead mH264Nu2h;
		void nuhVctInit(std::vector<NaluHead *> &nuhVct);
	};
}


#endif