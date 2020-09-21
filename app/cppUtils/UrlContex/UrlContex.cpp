#include "UrlContex.h"
#include <lzUtils/base.h>
using namespace cppUtils;
bool UrlContex::paraParse(std::string &tail) {
	std::string paraStr;
	bool isParseFinished = false;
	int idx = tail.find_first_of('&');
	if(idx < 0) {
		idx = tail.find_first_of(' ');
		if(idx < 0) {
			idx = tail.find("\r\n\r\n");
		}
		isParseFinished = true;
	}
	paraStr = tail.substr(0, idx);
	UrlParam para;
	int paraIdx = paraStr.find_first_of('=');
	if(paraIdx < 0) {
		s_err("para format err('=' is not find).");
		return false;
	}
	para.key = paraStr.substr(0, paraIdx);
	para.value = paraStr.substr(paraIdx + 1);
	mPars.push_back(para);
	if(isParseFinished) {
		s_dbg("mMethod=%s", mMethod.data());
		s_dbg("mCmd=%s", mCmd.data());
		return true;
	}
	std::string newStr = tail.substr(idx + 1);
	return paraParse(newStr);
}
bool UrlContex::parse(std::string url) {
	std::string tail = url;
	int idx = tail.find(" /");
	if(idx < 0) {
		return false;
	}
	mMethod = tail.substr(0,idx);
	tail = tail.substr(idx + 2);
	idx = tail.find_first_of('?');
	if(idx < 0) {
		idx = tail.find_first_of(' ');
		if(idx < 0) {
			idx = tail.find("\r\n\r\n");
			mCmd = tail.substr(0, idx);
			return true;
		}
		mCmd = tail.substr(0, idx);
		return true;
	}
	mCmd = tail.substr(0, idx);
	std::string newStr = tail.substr(idx + 1);
	return paraParse(newStr);
}
std::string UrlContex::getParaVal(std::string key){
	for(auto i:mPars){
		if(i.key == key){
			return i.value;
		}
	}
	return "";
}