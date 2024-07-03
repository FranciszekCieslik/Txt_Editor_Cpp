#include <string.h>
#include <stdlib.h>

class AppendBuffer
{
public:
    char *b;
    int len;
public:
    AppendBuffer(/* args */);
    ~AppendBuffer();
    void append(const char *s, int new_len);
    void free();
};

AppendBuffer::AppendBuffer() : b(nullptr), len(0) {} 

AppendBuffer::~AppendBuffer()
{
}

void AppendBuffer::append(const char *s, int new_len)
{
 char *n = static_cast<char*>(realloc(b, len + new_len));
  if (n == NULL) return;
  memcpy(&n[len], s, new_len);
  b = n;
  len += new_len;
}

void AppendBuffer::free()
{
   ::free(b);
   b = nullptr;
   len = 0;
}


