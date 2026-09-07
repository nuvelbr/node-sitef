#include <napi.h>
#include "../nodesitef.hpp"

using std::string;

class ConfigPromise : public PromiseWorker<int>
{
public:
  static Value Create(const CallbackInfo &info)
  {
    if (info.Length() < 5)
      return Reject(info.Env(), "MissingArgument");
    else if (!info[0].IsString() || !info[1].IsString() || !info[2].IsString() || !info[4].IsString())
      return Reject(info.Env(), "InvalidArgument");

    short reservado = 0;
    if (!lerShort(info[3], &reservado))
      return Reject(info.Env(), "InvalidArgument");

    string ip = info[0].As<String>().Utf8Value();
    string loja = info[1].As<String>().Utf8Value();
    string terminal = info[2].As<String>().Utf8Value();
    string parametrosAdicionais = info[4].As<String>().Utf8Value();

    ConfigPromise *worker = new ConfigPromise(info.Env(), ip, loja, terminal, reservado, parametrosAdicionais);
    worker->Queue();
    return worker->deferredPromise.Promise();
  }

protected:
  void Execute() override
  {
    ConfiguraIntSiTefInterativoEx configuraSitef;
    if (!vincular("ConfiguraIntSiTefInterativoEx", &configuraSitef))
      return;

    result = configuraSitef(ip.c_str(), loja.c_str(), terminal.c_str(), reservado, parametrosAdicionais.c_str());
  }

  virtual void OnOK() override
  {
    deferredPromise.Resolve(criarInteiro(Env(), result));
  }

private:
  ConfigPromise(napi_env env,
                const string &p_ip,
                const string &p_loja,
                const string &p_terminal,
                short p_reservado,
                const string &p_parametrosAdicionais) : PromiseWorker(env),
                                                        ip(p_ip),
                                                        loja(p_loja),
                                                        terminal(p_terminal),
                                                        parametrosAdicionais(p_parametrosAdicionais),
                                                        reservado(p_reservado) {}

  string ip;
  string loja;
  string terminal;
  string parametrosAdicionais;
  short reservado;
};
