/*
 * node-base62
 *
 * Base62 encode/decode functions are shamelessly stolen from:
 * http://github.com/mewz/maudir/blob/master/base62.cpp
 */
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <v8.h>
#include <node.h>
#include <node_buffer.h>

using namespace v8;
using namespace node;

#define EXCEPTION(message) ThrowException(Exception::TypeError(String::New(message)))

static const char base62_vals[] = "0123456789"
                                  "abcdefghijklmnopqrstuvwxyz"
                                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static const int base62_index[] = {
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,    0,    0, 
       0,    0,    0,    0,    0, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 
    0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 
    0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d,    0,    0,    0,    0,    0, 
       0, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 
    0x21, 0x22, 0x23, 
};

void
strreverse_inplace (char *str)
{
    char c;
    int half;
    int len;
    int i;

    assert(str);

    len = strlen(str);
    half = len >> 1;
    for (i = 0; i < half; i++) {
        c = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = c;
    }
}

bool
base62_encode (uint64_t val, char *str, size_t len)
{
    int i = 0;
    int v;

    assert(str);
    assert(len > 0);

    do {
        if (i + 1 >= len)
            return false;
        v = val % 62;
        str[i++] = base62_vals[v];
        val = (val - v) / 62;
    } while (val > 0);
    str[i] = '\0';
    strreverse_inplace(str);

    return true;
}

uint64_t
base62_decode (const char *str)
{
    uint64_t val = 0;
    char c;
    int len;
    int i;

    assert(str);

    len = strlen(str);
    for (i = 0; i < len; i++) {
        c = str[i];
        if (!isalnum(c)) {
            return -1;
        }
        val += base62_index[c] * powl(62, len - i - 1);
    }

    return val; 
}

Handle<Value>
base62_decode_binding(const Arguments &args)
{
    HandleScope scope;
    if (args.Length() != 1)
        return EXCEPTION("One argument required");

    if (!args[0]->IsString())
        return EXCEPTION("The value should be a string");

    uint64_t decoded;
    String::AsciiValue b62data(args[0]->ToString());
    decoded = base62_decode(*b62data);

    return scope.Close(Integer::New(decoded));
}

Handle<Value>
base62_encode_binding(const Arguments &args)
{
    HandleScope scope;
    if (args.Length() != 1)
        return EXCEPTION("One argument required");

    if (!args[0]->IsNumber())
        return EXCEPTION("The value should be an integer");

    char encoded[12];
    uint64_t value = args[0]->IntegerValue();

    base62_encode(value, encoded, sizeof(encoded));

    return scope.Close(String::New(encoded));
}

extern "C" void 
init(Handle<Object> target)
{
    HandleScope scope;
    target->Set(String::New("encode"), FunctionTemplate::New(base62_encode_binding)->GetFunction());
    target->Set(String::New("decode"), FunctionTemplate::New(base62_decode_binding)->GetFunction());
}
