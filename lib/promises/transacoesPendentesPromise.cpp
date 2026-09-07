#include <napi.h>
#include "../nodesitef.hpp"

using std::string;

class TransacoesPendentesPromise : public PromiseWorker<int>
{
public:
  static Value Create(const CallbackInfo &info)
  {
    if (info.Length() < 2)
      return Reject(info.Env(), "MissingArgument");
    else if (!info[0].IsString() || !info[1].IsString())
      return Reject(info.Env(), "InvalidArgument");

    string dataFiscal = jsParaLatin1(info[0]);
    string cupomFiscal = jsParaLatin1(info[1]);

    TransacoesPendentesPromise *worker = new TransacoesPendentesPromise(info.Env(), dataFiscal, cupomFiscal);
    worker->Queue();
    return worker->deferredPromise.Promise();
  }

protected:
  void Execute() override
  {
    ObtemQuantidadeTransacoesPendentes obtemQuantidade;
    if (!vincular("ObtemQuantidadeTransacoesPendentes", &obtemQuantidade))
      return;

    result = obtemQuantidade(dataFiscal.c_str(), cupomFiscal.c_str());
  }

  virtual void OnOK() override
  {
    deferredPromise.Resolve(criarInteiro(Env(), result));
  }

private:
  TransacoesPendentesPromise(napi_env env, const string &p_dataFiscal, const string &p_cupomFiscal)
      : PromiseWorker(env), dataFiscal(p_dataFiscal), cupomFiscal(p_cupomFiscal) {}

  string dataFiscal;
  string cupomFiscal;
};
