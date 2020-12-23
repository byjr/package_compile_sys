
class SourceManager{
	std::shared_ptr<DataBuffer> mIn;
	std::unordered_map<int,std::shared_ptr<DataBuffer>> mOuts;
	std::mutex oMux;
	std::mutex iMux;
	std::thread mTrd;	
	std::atomic<bool> goExitFlag,isExitedFlag;
public:
	void addOutput(int fd,std::shared_ptr<DataBuffer> out){
		std::unique_lock<std::mutex> lk(oMux);
		mOuts[fd] = out;
	}
	void delOutput(int fd){
		std::unique_lock<std::mutex> lk(oMux);
		mOuts.erease(fd);
	}
	void setInput(std::shared_ptr<DataBuffer> in){
		std::unique_lock<std::mutex> lk(iMux);
		mIn.reset();
		mIn = in;
	}
	SourceManager(){
		goExitFlag = false;
		isExitedFlag = false;
		mTrd = std::thread([this](){			
			bool res = false;
			std::shared_ptr<data_unit> inData;
			for(;!goExitFlag;){
				{//read lock scope
					std::unique_lock<std::mutex> lk(iMux);
					if(!mIn.get()){
						std::this_thread::sleep_for(std::chrono::milliseconds(100));
						continue;
					}
					if(!mIn->wbPop(inData)){
						break;
					}					
				}		
				if(goExitFlag)break;
				{//write lock scope
					std::unique_lock<std::mutex> lk(oMux);
					for(auto i:mOuts){
						auto data = make_shared<data_unit>(inData.get());
						if(data.get()){
							i.second->crcPush(data);
						}
						if(goExitFlag)break;
					}					
				}			
			}
			isExitedFlag = true;
		});
	}
	void stop(){
		goExitFlag = true;		
	}
	bool isExited(){
		return isExitedFlag;
	}
	~SourceManager(){
		if(mTrd.joinable()){
			mTrd.join();
		}
	}
};