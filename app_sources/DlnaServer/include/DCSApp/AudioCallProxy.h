#include "DcsSdk/AudioCallInterface.h"
#include <libcchip/platform.h>

namespace duerOSDcsApp {
namespace voip {

using namespace duerOSDcsSDK::sdkInterfaces;

class AudioCallProxy :public duerOSDcsSDK::sdkInterfaces::AudioCallInterface {
public:
  void initiatecallByName(const std::string& name)  override;

  void initiatecallByNumber(const std::string& number) override;

  void selectCall(std::vector<std::string>& numbers) override;

  void pickUpCall() override;

  void hangUpCall() override;

  void uploadLog() override;

  void updateAccounts() {
	 inf(__func__);
  }

  void bindPhoneCall(bool status) {
	 inf(__func__);
  }
  void startedCall();

  void failedCall();

  void acceptedCall();

  void receivedCall();

  void updateAccounts(const std::string& type, const std::string& msisdn,
        const std::string& appKey, const std::string& appSecret) {
	 inf(__func__);
  }

  std::string getCallStatus() { return "PERMIT"; }

  void setObserver(std::shared_ptr<AudioCallObserverInterface> AudoiCallObserver) override;

  static void* tipSoundAudio(void* arg);

  static AudioCallProxy* getInstance();
private:
  static std::shared_ptr<AudioCallObserverInterface> m_AudoiCallObserver;

  AudioCallProxy();

  static AudioCallProxy* m_AudioCallProxy;

};
}
}
