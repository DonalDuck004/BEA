#include "string_utilies.hpp"

char* str_copy(char* str){
  char* x = (char*)malloc(sizeof(char) * strlen(str));
  memcpy(x, str, strlen(str));
  return x;
}

char* str_repeat(char* str, int times){
  int to = strlen(str) * times;
  char* buff = (char*)malloc(sizeof(char) * to + sizeof(char));

  for (int i = 0; i < to; i += strlen(str))
    memcpy(buff + i, str, strlen(str));

  buff[to] = 0;

  return buff;
}

char* str_span(char* src, int from, int to) {
    int str_len = strlen(src);
    if (to < 0)
        to = str_len + to;
    if (from < 0)
        from = str_len + from;

    if (to == str_len - 1)
        return src + from;

    char* r;
    if (from > to || from >= str_len) {
        r = (char*)malloc(sizeof(char));
        r[0] = 0;
        return r;
    }

    r = (char*)malloc(sizeof(char) * (to - from + 1));

    memcpy(r, src + from, to - from);
    r[to - from] = 0;

    return r;
}
