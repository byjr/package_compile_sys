#ifndef _APP_SOURCES_MYBOX_SRC_ONE2MULTI_ONE2MULTI_H_
#define _APP_SOURCES_MYBOX_SRC_ONE2MULTI_ONE2MULTI_H_
#include <memory>
namespace cppUtils {
	struct MyTmplatePar {

	};
	class MyTmplate {
		std::shared_ptr<MyTmplatePar> mPar;
		bool mIsReadyFlag, mGotExitFlag;
	public:
		bool isReady();
		MyTmplate(std::shared_ptr<MyTmplatePar> &par);
		~MyTmplate();
	};
}
#endif//_APP_SOURCES_MYBOX_SRC_ONE2MULTI_ONE2MULTI_H_