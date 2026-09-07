#include <napi.h>
#include "../nodesitef.hpp"

using std::string;

class LeSimNaoPromise : public PromiseWorker<int>
{
public:
  static Value Create(const CallbackInfo &info)
  {
    if (info.Length() < 1)
      return Reject(info.Env(), "MissingArgument");
    else if (!info[0].IsString())
      return Reject(info.Env(), "InvalidArgument");

    string mensagem = jsParaLatin1(info[0]);
    LeSimNaoPromise *worker = new LeSimNaoPromise(info.Env(), mensagem);
    worker->Queue();
    return worker->deferredPromise.Promise();
  }

protected:
  void Execute() override
  {
    LeSimNaoPinPad leSimNao;
    if (!vincular("LeSimNaoPinPad", &leSimNao))
      return;

    result = leSimNao(mensagem.c_str());
  }

  virtual void OnOK() override
  {
    deferredPromise.Resolve(criarInteiro(Env(), result));
  }

private:
  LeSimNaoPromise(napi_env env, const string &p_mensagem) : PromiseWorker(env), mensagem(p_mensagem) {}

  string mensagem;
};
