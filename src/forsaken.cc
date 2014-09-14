#include <node.h>
#include <nan.h>

#include <openssl/ssl.h>
#include "ssl.h"

// #define LOG(msg) printf("%s\n", msg);
#define LOG(msg) ;

#define REQ_BUF_ARG(I, VAR)                                                   \
  if (args.Length() <= (I) || !node::Buffer::HasInstance(args[I]))            \
    return NanThrowTypeError("Argument " #I " must be a buffer");             \
  Local<Object> _ ## VAR = args[I]->ToObject();                               \
  char *VAR = node::Buffer::Data(_ ## VAR);                                   \
  size_t VAR ## _len = node::Buffer::Length(_ ## VAR);

#define KEY_private                                                           \
  NanUtf8String passphrase(args[2]);                                          \
  RSA* rsa = rsa_private_key(key_pem, key_pem_len, *passphrase);

#define KEY_public                                                            \
  RSA* rsa = rsa_public_key(key_pem, key_pem_len);

#define RSAUTL_METHOD(NAME, KEY, OP)                                          \
NAN_METHOD(NAME) {                                                            \
  NanScope();                                                                 \
  LOG("Beginning " #NAME)                                                     \
                                                                              \
  ClearErrorOnReturn clear_error_on_return;                                   \
  (void) &clear_error_on_return; /* Silence compiler warning */               \
                                                                              \
  const char *err_msg = NULL;                                                 \
  int out_len = 0;                                                            \
  char *out = NULL;                                                           \
  int pad = RSA_PKCS1_PADDING;                                                \
  bool fatal = true;                                                          \
                                                                              \
  REQ_BUF_ARG(0, in)                                                          \
  REQ_BUF_ARG(1, key_pem)                                                     \
                                                                              \
  KEY_ ## KEY                                                                 \
  if (rsa == NULL)                                                            \
    goto exit;                                                                \
                                                                              \
  out_len = RSA_size(rsa);                                                    \
  out = new char[out_len];                                                    \
  out_len = RSA_ ## KEY ## _ ## OP(in_len,                                    \
                                   (unsigned char *) in,                      \
                                   (unsigned char *) out,                     \
                                   rsa,                                       \
                                   pad);                                      \
  if (out_len > 0)                                                            \
    fatal = false;                                                            \
                                                                              \
 exit:                                                                        \
  if (rsa != NULL)                                                            \
    RSA_free(rsa);                                                            \
                                                                              \
  if (fatal)                                                                  \
    return NanThrowError(ssl_error_str(&err_msg, #NAME " operation failed")); \
                                                                              \
  Local<Value> rc = NanNewBufferHandle(out, out_len);                         \
  NanReturnValue(rc);                                                         \
}

using namespace v8;

RSAUTL_METHOD(Decrypt, private, decrypt)
RSAUTL_METHOD(Encrypt, public,  encrypt)
RSAUTL_METHOD(Sign,    private, encrypt)
RSAUTL_METHOD(Verify,  public,  decrypt)

void InitOnce() {
  SSL_load_error_strings();
  SSL_library_init();
}

void Init(Handle<Object> exports) {
  static uv_once_t init_once = UV_ONCE_INIT;
  uv_once(&init_once, InitOnce);
  NODE_SET_METHOD(exports, "decrypt", Decrypt);
  NODE_SET_METHOD(exports, "encrypt", Encrypt);
  NODE_SET_METHOD(exports, "sign",    Sign);
  NODE_SET_METHOD(exports, "verify",  Verify);
}

NODE_MODULE(rsautl, Init)
