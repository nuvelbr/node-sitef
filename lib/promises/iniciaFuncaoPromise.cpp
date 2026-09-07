#include <napi.h>
#include "../nodesitef.hpp"

using std::string;

class IniciaFuncaoPromise : public PromiseWorker<int>
{
public:
  static Value Create(const CallbackInfo &info)
  {
    if (info.Length() < 7)
      return Reject(info.Env(), "MissingArgument");
    else if (!info[0].IsNumber() || !info[1].IsString() || !info[2].IsString() || !info[3].IsString() ||
             !info[4].IsString() || !info[5].IsString() || !info[6].IsString())
      return Reject(info.Env(), "InvalidArgument");

    int funcao = info[0].As<Number>().Int32Value();
    string valor = jsParaLatin1(info[1]);
    string cupomFiscal = jsParaLatin1(info[2]);
    string dataFiscal = jsParaLatin1(info[3]);
    string horaFiscal = jsParaLatin1(info[4]);
    string operador = jsParaLatin1(info[5]);
    string paramAdicionais = jsParaLatin1(info[6]);

    IniciaFuncaoPromise *worker = new IniciaFuncaoPromise(info.Env(), funcao, valor, cupomFiscal, dataFiscal, horaFiscal, operador, paramAdicionais);
    worker->Queue();
    return worker->deferredPromise.Promise();
  }

protected:
  void Execute() override
  {
    IniciaFuncaoSiTefInterativo iniciaFuncao;
    if (!vincular("IniciaFuncaoSiTefInterativo", &iniciaFuncao))
      return;

    result = iniciaFuncao(funcao, valor.c_str(), cupomFiscal.c_str(), dataFiscal.c_str(), horaFiscal.c_str(), operador.c_str(), paramAdicionais.c_str());
  }

  virtual void OnOK() override
  {
    deferredPromise.Resolve(criarInteiro(Env(), result));
  }

private:
  IniciaFuncaoPromise(napi_env env,
                      int p_funcao,
                      const string &p_valor,
                      const string &p_cupomFiscal,
                      const string &p_dataFiscal,
                      const string &p_horaFiscal,
                      const string &p_operador,
                      const string &p_paramAdicionais) : PromiseWorker(env),
                                                         valor(p_valor),
                                                         cupomFiscal(p_cupomFiscal),
                                                         dataFiscal(p_dataFiscal),
                                                         horaFiscal(p_horaFiscal),
                                                         operador(p_operador),
                                                         paramAdicionais(p_paramAdicionais),
                                                         funcao(p_funcao) {}

  string valor;
  string cupomFiscal;
  string dataFiscal;
  string horaFiscal;
  string operador;
  string paramAdicionais;
  int funcao;
};
