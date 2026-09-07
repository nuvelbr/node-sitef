#include <napi.h>
#include <string>
#include "../nodesitef.hpp"

template <class T>
class PromiseWorker : public Napi::AsyncWorker
{
protected:
  virtual void OnError(const Napi::Error &e) override
  {
    deferredPromise.Reject(e.Value());
  }

  static Napi::Value Reject(Napi::Env env, const char *msg)
  {
    Napi::Promise::Deferred failed = Napi::Promise::Deferred::New(env);
    failed.Reject(Napi::Error::New(env, msg).Value());
    return failed.Promise();
  }

  // Resolve o export e marca o worker com erro quando ele nao esta disponivel.
  // Uma excecao C++ atravessando `Execute()` nao teria como ser convertida em
  // rejeicao da promise — o addon compila com NAPI_DISABLE_CPP_EXCEPTIONS.
  template <class Fn>
  bool vincular(const char *nome, Fn *destino)
  {
    *destino = reinterpret_cast<Fn>(sitefSymbol(nome));
    if (*destino)
      return true;

    SetError(sitefErroSimbolo(nome));
    return false;
  }

  PromiseWorker(napi_env env) : Napi::AsyncWorker(env), deferredPromise(Napi::Promise::Deferred::New(env)), result() {}

  Napi::Promise::Deferred deferredPromise;
  T result;
};
