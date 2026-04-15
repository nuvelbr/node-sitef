#include <napi.h>
#include <vector>
#include <cstring>
#include "../nodesitef.hpp"

using std::string;
using std::vector;

// BUG 1 fix (DEV-850): the previous version allocated `char buffer[20000]` on
// the stack inside `Create()` and stored a raw `char *` pointer to it in the
// worker. When `Create()` returned the stack frame was destroyed, and
// `Execute()` (running on a worker thread) would read/write through a
// dangling pointer. This produced undefined behaviour: random crashes,
// memory corruption, or silent false success.
//
// The fix owns the buffer inside the worker itself as a `std::vector<char>`
// sized to the requested `tamBuffer` (bounded to a safe maximum). The SiTEF
// library writes into `buffer.data()` which remains valid for the lifetime
// of the worker, including while `Execute()` is running on the worker
// thread. `OnOK()` builds the JS string from the first NUL byte, mirroring
// the original expected behaviour.
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

    int comando = info[0].ToNumber().Int32Value();
    long tipoCampo = static_cast<long>(info[1].ToNumber().Int64Value());
    int tamMinimo = info[2].ToNumber().Int32Value();
    int tamMaximo = info[3].ToNumber().Int32Value();
    int tamBuffer = info[5].ToNumber().Int32Value();
    int continua = info[6].ToNumber().Int32Value();

    // Bound the buffer to a safe maximum (20000 bytes matches the prior
    // hardcoded stack buffer size). If the caller asks for something smaller
    // we respect that, but never less than 1 to avoid zero-size vectors.
    const int kMaxBuffer = 20000;
    if (tamBuffer <= 0 || tamBuffer > kMaxBuffer) tamBuffer = kMaxBuffer;

    string initial = info[4].ToString().Utf8Value();

    ContinuaFuncaoPromise *worker = new ContinuaFuncaoPromise(info.Env(),
                                                              comando,
                                                              tipoCampo,
                                                              tamMinimo,
                                                              tamMaximo,
                                                              initial,
                                                              tamBuffer,
                                                              continua);
    worker->Queue();
    return worker->deferredPromise.Promise();
  }

protected:
  void Execute() override
  {
    result = continuaFuncaoSiTefInterativo(
        &comando,
        &tipoCampo,
        &tamMinimo,
        &tamMaximo,
        buffer.data(),
        tamBuffer,
        continua);
  }

  virtual void OnOK() override
  {
    Object obj = Object::New(Env());
    obj.Set("retorno", result);
    obj.Set("comando", comando);
    obj.Set("tipoCampo", tipoCampo);
    obj.Set("tamMinimo", tamMinimo);
    obj.Set("tamMaximo", tamMaximo);
    // Build a safe C string view from the buffer (truncate at first NUL or
    // at tamBuffer-1 to guarantee termination).
    size_t len = 0;
    while (len < static_cast<size_t>(tamBuffer) && buffer[len] != '\0') len++;
    obj.Set("buffer", string(buffer.data(), len));

    deferredPromise.Resolve(obj);
  }

private:
  ContinuaFuncaoPromise(napi_env env,
                        int p_comando,
                        long p_tipoCampo,
                        int p_tamMinimo,
                        int p_tamMaximo,
                        const string &p_initial,
                        int p_tamBuffer,
                        int p_continua) : PromiseWorker(env),
                                          comando(p_comando),
                                          tipoCampo(p_tipoCampo),
                                          tamMinimo(p_tamMinimo),
                                          tamMaximo(p_tamMaximo),
                                          tamBuffer(p_tamBuffer),
                                          continua(p_continua)
  {
    // Own the buffer in the worker. Pre-fill with the initial string and
    // NUL-terminate so the native call sees a valid C string.
    buffer.assign(tamBuffer, '\0');
    size_t copy_len = p_initial.size();
    if (copy_len >= static_cast<size_t>(tamBuffer)) {
      copy_len = static_cast<size_t>(tamBuffer) - 1;
    }
    std::memcpy(buffer.data(), p_initial.data(), copy_len);
    buffer[copy_len] = '\0';
  }

  int comando;
  long tipoCampo;
  int tamMinimo;
  int tamMaximo;
  // BUG 1 fix: buffer is now owned by the worker (std::vector<char>).
  // It outlives Create() and stays valid through Execute()/OnOK().
  vector<char> buffer;
  int tamBuffer;
  int continua;
};

