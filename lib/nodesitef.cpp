#include "nodesitef.hpp"
#include "promises/promises.hpp"

#ifdef _WIN32
static HMODULE handler = nullptr;
#endif

#ifdef linux
static void *handler = nullptr;
#endif

bool sitefCarregada()
{
  return handler != nullptr;
}

void *sitefSymbol(const char *nome)
{
  if (!handler)
    return nullptr;

  #ifdef _WIN32
  return reinterpret_cast<void *>(GetProcAddress(handler, nome));
  #endif

  #ifdef linux
  return dlsym(handler, nome);
  #endif
}

string sitefErroSimbolo(const char *nome)
{
  if (!handler)
    return "Carregue a DLL do SiTef!";
  return string("A DLL do SiTef nao exporta ") + nome + ".";
}

Value carregarDLL(const CallbackInfo &info)
{
  Env env = info.Env();

  if (handler)
    return Boolean::New(env, true);

  if (info.Length() < 1)
  {
    Error::New(env, "Informe o caminho da DLL.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (!info[0].IsString())
  {
    TypeError::New(env, "O caminho informado nao e uma string valida.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  string path = info[0].As<String>().Utf8Value();

  #ifdef _WIN32
  // LOAD_WITH_ALTERED_SEARCH_PATH faz o loader procurar as DLLs irmas
  // (libemv64, QREncode64, libcurl64) no diretorio da propria CliSiTef em vez
  // do diretorio do processo.
  handler = LoadLibraryExA(path.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
  if (!handler)
  {
    string erro = "Nao foi possivel carregar a DLL " + path + " (GetLastError=" + std::to_string(GetLastError()) + ").";
    Error::New(env, erro).ThrowAsJavaScriptException();
    return env.Undefined();
  }
  #endif

  #ifdef linux
  handler = dlopen(path.c_str(), RTLD_LAZY);
  if (!handler)
  {
    const char *detalhe = dlerror();
    string erro = "Nao foi possivel carregar a biblioteca " + path + (detalhe ? string(": ") + detalhe : string("."));
    Error::New(env, erro).ThrowAsJavaScriptException();
    return env.Undefined();
  }
  #endif

  return Boolean::New(env, true);
}

Object Init(Env env, Object exports)
{
  exports.Set(
      String::New(env, "carregarDLL"),
      Function::New(env, carregarDLL));

  exports.Set(
      String::New(env, "configuraIntSiTefInterativo"),
      Function::New(env, ConfigPromise::Create));

  exports.Set(
      String::New(env, "verificaPresencaPinPad"),
      Function::New(env, VerificarPresencaPromise::Create));

  exports.Set(
      String::New(env, "leSimNaoPinPad"),
      Function::New(env, LeSimNaoPromise::Create));

  exports.Set(
      String::New(env, "escreveMensagemPermanentePinPad"),
      Function::New(env, EscreverMensagemPromise::Create));

  exports.Set(
      String::New(env, "iniciaFuncaoSiTefInterativo"),
      Function::New(env, IniciaFuncaoPromise::Create));

  exports.Set(
      String::New(env, "continuaFuncaoSiTefInterativo"),
      Function::New(env, ContinuaFuncaoPromise::Create));

  exports.Set(
      String::New(env, "finalizaFuncaoSiTefInterativo"),
      Function::New(env, FinalizaFuncaoPromise::Create));

  exports.Set(
      String::New(env, "obtemQuantidadeTransacoesPendentes"),
      Function::New(env, TransacoesPendentesPromise::Create));

  return exports;
}

NODE_API_MODULE(nodesitef, Init);
