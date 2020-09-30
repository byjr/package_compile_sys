#ifndef _cppUtils_UrlContex_H
#define _cppUtils_UrlContex_H
#include <string>
#include <vector>

namespace cppUtils {

	typedef struct UrlParam_s {
		std::string key;
		std::string value;
	} UrlParam;

	class UrlContex {
		std::string mMethod;
		std::string mCmd;
		std::vector<UrlParam> mPars;
		bool paraParse(std::string &tail);
	public:
		bool parse(std::string url);
		std::string getMethod() {
			return mMethod;
		}
		std::string getCmd() {
			return mCmd;
		}
		std::vector<UrlParam> getPars() {
			return mPars;
		}
		std::string getParaVal(std::string key);
	};
}
#endif