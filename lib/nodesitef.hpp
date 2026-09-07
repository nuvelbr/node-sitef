#ifndef NODESITEF_H
#define NODESITEF_H 1

#include <napi.h>

#include <cstdint>
#include <cstdlib>
#include <string>

#ifdef _WIN32
    #include <windows.h>
#endif
#ifdef linux
    #include <dlfcn.h>
#endif

using Napi::Boolean;
using Napi::CallbackInfo;
using Napi::Env;
using Napi::Error;
using Napi::Function;
using Napi::Number;
using Napi::Object;
using Napi::String;
using Napi::TypeError;
using Napi::Value;

using std::string;

// Assinaturas da Interface Simplificada da CliSiTef (VRS-256/258). Os tipos
// escalares seguem a especificacao: `short` para Reservado/Confirma e para
// TamMinimo/TamMaximo, e um inteiro de 32 bits para TipoCampo (o `long` da
// especificacao tem 32 bits no ABI do Windows x64 e 64 bits no Linux, por isso
// a largura e fixada aqui).
typedef int (*VerificaPresencaPinPad)();
typedef int (*ConfiguraIntSiTefInterativoEx)(const char *, const char *, const char *, short, const char *);
typedef int (*IniciaFuncaoSiTefInterativo)(int, const char *, const char *, const char *, const char *, const char *, const char *);
typedef int (*ContinuaFuncaoSiTefInterativo)(int *, int32_t *, short *, short *, char *, int, int);
typedef void (*FinalizaFuncaoSiTefInterativo)(short, const char *, const char *, const char *, const char *);
typedef int (*EscreveMensagemPermanentePinPad)(const char *);
typedef int (*LeSimNaoPinPad)(const char *);
typedef int (*ObtemQuantidadeTransacoesPendentes)(const char *, const char *);

// Tamanho minimo exigido pela especificacao para o Buffer do ContinuaFuncao.
const int kSiTefBufferMinimo = 20000;
// Teto defensivo: o tamanho vem do JavaScript e so precisa acomodar comprovantes.
const int kSiTefBufferMaximo = 1024 * 1024;

Value carregarDLL(const CallbackInfo &info);

// Retorna o endereco de um export da CliSiTef, ou nullptr quando a DLL ainda
// nao foi carregada ou o export nao existe naquela versao.
void *sitefSymbol(const char *nome);
bool sitefCarregada();
string sitefErroSimbolo(const char *nome);

// Todo numero devolvido ao JavaScript passa por aqui. `napi_create_int32` mantem
// os valores fora do caminho de argumentos de ponto flutuante: o thunk de
// delay-load que o lld gera para node.exe reserva o save de xmm0/xmm1 dentro do
// home space de `__delayLoadHelper2`, que sobrescreve os dois registradores. Um
// double entregue a `napi_create_double` chega, por isso, como o padrao de bits
// de um ponteiro.
inline Value criarInteiro(napi_env env, int32_t valor)
{
  napi_value saida = nullptr;
  if (napi_create_int32(env, valor, &saida) != napi_ok)
    return Value();
  return Value(env, saida);
}

// A CliSiTef fala Latin-1. Decodificar byte a byte para UTF-16 preserva os
// acentos; entregar os bytes crus a `napi_create_string_utf8` os transformaria
// em U+FFFD.
inline Value latin1ParaJs(napi_env env, const char *dados, size_t tamanho)
{
  std::u16string texto;
  texto.reserve(tamanho);
  for (size_t i = 0; i < tamanho; i++)
    texto.push_back(static_cast<char16_t>(static_cast<unsigned char>(dados[i])));

  napi_value saida = nullptr;
  if (napi_create_string_utf16(env, texto.data(), texto.size(), &saida) != napi_ok)
    return Value();
  return Value(env, saida);
}

inline string jsParaLatin1(const Value &valor)
{
  std::u16string texto = valor.As<String>().Utf16Value();
  string bytes;
  bytes.reserve(texto.size());
  for (char16_t unidade : texto)
    bytes.push_back(unidade <= 0xFF ? static_cast<char>(unidade) : '?');
  return bytes;
}

// Reservado e Confirma sao `short` no ABI. A API JavaScript sempre aceitou
// string nesses campos, entao numero e string numerica continuam validos.
inline bool lerShort(const Value &valor, short *saida)
{
  if (valor.IsNumber())
  {
    *saida = static_cast<short>(valor.As<Number>().Int32Value());
    return true;
  }

  if (valor.IsUndefined() || valor.IsNull())
  {
    *saida = 0;
    return true;
  }

  if (!valor.IsString())
    return false;

  string texto = valor.As<String>().Utf8Value();
  if (texto.empty())
  {
    *saida = 0;
    return true;
  }

  char *fim = nullptr;
  long convertido = std::strtol(texto.c_str(), &fim, 10);
  if (fim == texto.c_str() || *fim != '\0')
    return false;

  *saida = static_cast<short>(convertido);
  return true;
}

#include "promises/promiseWorker.cpp"

#endif /* NODESITEF_H */
