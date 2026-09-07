#include <napi.h>
#include "../nodesitef.hpp"

class VerificarPresencaPromise : public PromiseWorker<int>
{
public:
  static Value Create(const CallbackInfo &info)
  {
    VerificarPresencaPromise *worker = new VerificarPresencaPromise(info.Env());
    worker->Queue();
    return worker->deferredPromise.Promise();
  }

protected:
  void Execute() override
  {
    VerificaPresencaPinPad verificaPresenca;
    if (!vincular("VerificaPresencaPinPad", &verificaPresenca))
      return;

    result = verificaPresenca();
  }

  virtual void OnOK() override
  {
    deferredPromise.Resolve(criarInteiro(Env(), result));
  }

private:
  VerificarPresencaPromise(napi_env env) : PromiseWorker(env) {}
};
