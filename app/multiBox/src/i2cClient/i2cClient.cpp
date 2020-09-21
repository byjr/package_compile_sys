class i2cClientPar {
public:
	const char *devPath;
	unsigned char devAddr;
	unsigned char subDev;
};
class i2cClient {
	i2cClientPar *mPar;
	int mFd;
	struct i2c_rdwr_ioctl_data mDat;
	unsigned char mDevAddr;
	unsigned char buftmp[32];
public:
	i2cClient(i2cClientPar *par) {
		mPar = par;
		mFd = open(devPath, O_RDWR);
		if(mFd < 0) {
			show_errno(0, "open");
			return ;
		}
		mDat.nmsgs = 2;
		mDat.msgs = (struct i2c_msg *)malloc(mDat.nmsgs * sizeof(struct i2c_msg));
		if(!mDat.msgs) {
			s_err("oom");
			return;
		}
		ioctl(mFd, I2C_TIMEOUT, 1);
		ioctl(mFd, I2C_RETRIES, 2);

		mDevAddr = mPar->devAddr >> 1;

		//write reg
		memset(buftmp, 0, 32);
		buftmp[0] = mPar->subDev;
		mDat.msgs[0].len = 1;
		mDat.msgs[0].addr = mDevAddr;
		mDat.msgs[0].flags = 0;     // 0: write 1:read
		mDat.msgs[0].buf = buftmp;

		mDat.msgs[1].addr = mDevAddr;
	}

	unsigned char i2cRead(unsigned char *dat, size_t size) {
		//read data
		mDat.msgs[1].flags = 1;     // 0: write 1:read
		mDat.msgs[1].buf = dat;
		mDat.msgs[1].len = size;
		int ret = ioctl(fd, I2C_RDWR, (unsigned long)&mDat);
		if (ret < 0) {
			s_err("read data %x %x error", mDevAddr, mPar->subDev);
		}
		close(mFd);
		free(mDat.msgs);
		return ret;
	}
	unsigned char i2cWrite(unsigned char *dat, size_t size) {
		//read data
		mDat.msgs[1].flags = 1;     // 0: write 1:read
		mDat.msgs[1].buf = dat;
		mDat.msgs[1].len = size;
		int ret = ioctl(fd, I2C_RDWR, (unsigned long)&mDat);
		if (ret < 0) {
			s_err("read data %x %x error", mDevAddr, sub_addr);
		}
		close(mFd);
		free(mDat.msgs);
		return ret;
	}
};