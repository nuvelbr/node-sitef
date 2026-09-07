#include <napi.h>
#include "../nodesitef.hpp"

using std::string;

class FinalizaFuncaoPromise : public PromiseWorker<bool>
{
public:
  static Value Create(const CallbackInfo &info)
  {
    if (info.Length() < 5)
      return Reject(info.Env(), "MissingArgument");
    else if (!info[1].IsString() || !info[2].IsString() || !info[3].IsString() || !info[4].IsString())
      return Reject(info.Env(), "InvalidArgument");

    short confirma = 0;
    if (!lerShort(info[0], &confirma))
      return Reject(info.Env(), "InvalidArgument");

    string cupomFiscal = jsParaLatin1(info[1]);
    string dataFiscal = jsParaLatin1(info[2]);
    string horaFiscal = jsParaLatin1(info[3]);
    string paramAdicionais = jsParaLatin1(info[4]);

    FinalizaFuncaoPromise *worker = new FinalizaFuncaoPromise(info.Env(), confirma, cupomFiscal, dataFiscal, horaFiscal, paramAdicionais);
    worker->Queue();
    return worker->deferredPromise.Promise();
  }

protected:
  void Execute() override
  {
    FinalizaFuncaoSiTefInterativo finalizaFuncao;
    if (!vincular("FinalizaFuncaoSiTefInterativo", &finalizaFuncao))
      return;

    finalizaFuncao(confirma, cupomFiscal.c_str(), dataFiscal.c_str(), horaFiscal.c_str(), paramAdicionais.c_str());
    result = true;
  }

  virtual void OnOK() override
  {
    deferredPromise.Resolve(Boolean::New(Env(), result));
  }

private:
  FinalizaFuncaoPromise(napi_env env,
                        short p_confirma,
                        const string &p_cupomFiscal,
                        const string &p_dataFiscal,
                        const string &p_horaFiscal,
                        const string &p_paramAdicionais) : PromiseWorker(env),
                                                           cupomFiscal(p_cupomFiscal),
                                                           dataFiscal(p_dataFiscal),
                                                           horaFiscal(p_horaFiscal),
                                                           paramAdicionais(p_paramAdicionais),
                                                           confirma(p_confirma) {}

  string cupomFiscal;
  string dataFiscal;
  string horaFiscal;
  string paramAdicionais;
  short confirma;
};
