#include <napi.h>
#include <cstring>
#include <vector>
#include "../nodesitef.hpp"

using std::string;
using std::vector;

// O Buffer e propriedade do worker (`vector<char>`), nao da pilha de `Create()`:
// a CliSiTef escreve nele enquanto `Execute()` roda na thread de trabalho, muito
// depois de `Create()` ter retornado.
class ContinuaFuncaoPromise : public PromiseWorker<int>
{
public:
  static Value Create(const CallbackInfo &info)
  {
    if (info.Length() < 7)
      return Reject(info.Env(), "MissingArgument");
    else if (!info[0].IsNumber() ||
             !info[1].IsNumber() ||
             !info[2].IsNumber() ||
             !info[3].IsNumber() ||
             !info[4].IsString() ||
             !info[5].IsNumber() ||
             !info[6].IsNumber())
      return Reject(info.Env(), "InvalidArgument");

    int comando = info[0].As<Number>().Int32Value();
    int32_t tipoCampo = info[1].As<Number>().Int32Value();
    short tamMinimo = static_cast<short>(info[2].As<Number>().Int32Value());
    short tamMaximo = static_cast<short>(info[3].As<Number>().Int32Value());
    int tamBuffer = info[5].As<Number>().Int32Value();
    int continua = info[6].As<Number>().Int32Value();

    if (tamBuffer < kSiTefBufferMinimo)
      tamBuffer = kSiTefBufferMinimo;
    else if (tamBuffer > kSiTefBufferMaximo)
      tamBuffer = kSiTefBufferMaximo;

    ContinuaFuncaoPromise *worker = new ContinuaFuncaoPromise(info.Env(),
                                                              comando,
                                                              tipoCampo,
                                                              tamMinimo,
                                                              tamMaximo,
                                                              jsParaLatin1(info[4]),
                                                              tamBuffer,
                                                              continua);
    worker->Queue();
    return worker->deferredPromise.Promise();
  }

protected:
  void Execute() override
  {
    ContinuaFuncaoSiTefInterativo continuaFuncao;
    if (!vincular("ContinuaFuncaoSiTefInterativo", &continuaFuncao))
      return;

    result = continuaFuncao(&comando, &tipoCampo, &tamMinimo, &tamMaximo, buffer.data(), tamBuffer, continua);
  }

  virtual void OnOK() override
  {
    napi_env env = Env();
    Object obj = Object::New(Env());
    obj.Set("retorno", criarInteiro(env, result));
    obj.Set("comando", criarInteiro(env, comando));
    obj.Set("tipoCampo", criarInteiro(env, tipoCampo));
    obj.Set("tamMinimo", criarInteiro(env, tamMinimo));
    obj.Set("tamMaximo", criarInteiro(env, tamMaximo));

    size_t tamanho = 0;
    while (tamanho < static_cast<size_t>(tamBuffer) && buffer[tamanho] != '\0')
      tamanho++;
    obj.Set("buffer", latin1ParaJs(env, buffer.data(), tamanho));

    deferredPromise.Resolve(obj);
  }

private:
  ContinuaFuncaoPromise(napi_env env,
                        int p_comando,
                        int32_t p_tipoCampo,
                        short p_tamMinimo,
                        short p_tamMaximo,
                        const string &p_resposta,
                        int p_tamBuffer,
                        int p_continua) : PromiseWorker(env),
                                          comando(p_comando),
                                          tipoCampo(p_tipoCampo),
                                          tamBuffer(p_tamBuffer),
                                          continua(p_continua),
                                          tamMinimo(p_tamMinimo),
                                          tamMaximo(p_tamMaximo)
  {
    buffer.assign(tamBuffer, '\0');
    size_t copiar = p_resposta.size();
    if (copiar >= static_cast<size_t>(tamBuffer))
      copiar = static_cast<size_t>(tamBuffer) - 1;
    std::memcpy(buffer.data(), p_resposta.data(), copiar);
  }

  vector<char> buffer;
  int comando;
  int32_t tipoCampo;
  int tamBuffer;
  int continua;
  short tamMinimo;
  short tamMaximo;
};
