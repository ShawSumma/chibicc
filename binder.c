#include "binder.h"
#include "chibicc.h"

char *bind_to_json(char *path) {
  Token *tok1 = tokenize_file(path);
  Token *tok2 = preprocess(tok1);
  
  Obj *prog = parse(tok2);

  char *buf;
  size_t buflen;
  FILE *output_buf = open_memstream(&buf, &buflen);

  codegen(prog, output_buf);
  return buf;
}