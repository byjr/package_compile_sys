#ifndef __WEB_SERVER_H__
#define __WEB_SERVER_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <netdb.h>
#include <list>
#include <unordered_map>
#include <lzUtils/base.h>

namespace multlBox {
	class MimePar {
	public:
		std::string path;
		std::string type;
	};
	class ReqParam {
	public:
		std::string key;
		std::string value;
	};
	class ReqContex {
	public:
		std::string mMethod;
		std::string mCmd;
		std::vector<ReqParam> m_params;
		bool parse(std::string &req);
		bool paraParse(std::string &tail);
	};
	class TCPSocket {
	protected:
		int m_sockfd;
		fd_set m_readfds;
		fd_set m_writefds;
		bool mExternStopFlag;
		bool mInnerAbortFlag;

	public:
		TCPSocket() {
			mExternStopFlag = false;
			mInnerAbortFlag = false;
		}
		~TCPSocket();

		int Send(const char *buf, int len);
		int Recv(char *buf, int len);

		int GetSocketFD() {
			return m_sockfd;
		}
		bool isReadable();
		bool isConnActive();
		void closeSocket();
	};

	class SockClient : public TCPSocket {
		enum {
			RESPONSE_WAITING,
			RESPONSE_START,
			RESPONSE_FINISH,
		};
	private:
		std::shared_ptr<std::istream> m_stream;
		std::thread mHhanleThred;
		int m_responseStep;

	public:
		SockClient(int sockfd) {
			m_sockfd = sockfd;
			m_responseStep = RESPONSE_WAITING;
			FD_ZERO(&m_readfds);
			FD_ZERO(&m_writefds);
			FD_SET(m_sockfd, &m_readfds);
			FD_SET(m_sockfd, &m_writefds);
		}
		std::string getCmdResult(const char *fmt, ...);
		bool cmdResultParse(std::string &res);
		int ResponseMime(ReqContex &reqCtx);
		int SendStreamOnece(ReqContex &reqCtx);
		int SendStreaming(ReqContex &reqCtx);
		int ResponseText(const char *txt);
		int HandleResponse(std::string &req) ;
		bool HandleRequest();
		void Stop();
		bool IsWaitingRequest() {
			return m_responseStep == RESPONSE_WAITING;
		}
		bool IsFinishResponse() {
			return m_responseStep == RESPONSE_FINISH;
		}
	};

	class SockServ : public TCPSocket {
	private:
		int m_bindPort;
		std::list<SockClient *> m_clients;
		std::thread m_mainLoopThread;
		std::shared_ptr<std::istream> m_stream;

	private:
		int Bind();
		int Listen();
	public:
		SockServ();
		~SockServ();
		int Run();
		void Stop();
	};
}
#endif
