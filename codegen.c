#include "chibicc.h"

int align_to(int n, int align) { return (n + align - 1) / align * align; }

static Type **stack;

static const char *basic_type_json(const char *str) {
  return format("{\"type\": \"%s\"}", str);
}

static const char *sign_checked_type_json(const char *str, bool usig) {
  return format("{\"type\": \"%s\", \"signedness\": \"%s\"}", str,
                usig ? "unsigned" : "signed");
}

static const char *type_json(Type *type);

static const char *member_json(Member *member) {
  return format("{\"type\": %s, \"offset\": \"%i\"}", type_json(member->ty),
                member->offset);
}

static const char *members_json(Member *members) {
  if (members == NULL) {
    return "{}";
  }
  const char *ret = "";
  int n = 0;
  while (members != NULL) {
    const char *elem = member_json(members);
    ret = format("%s, \"%i\": %s", ret, n++, elem);
    members = members->next;
  }
  return format("{\"length\": \"%i\"%s}", n, ret);
}

static const char *params_json(Type *type) {
  if (type == NULL) {
    return "{\"length\": \"0\"}";
  }
  const char *ret = "";
  int n = 0;
  while (type) {
    const char *elem = type_json(type);
    ret = format("%s, \"%i\": %s", ret, n++, elem);
    type = type->next;
  }
  return format("{\"length\": \"%i\"%s}", n, ret);
}

static const char *type_json_impl(Type *type) {
  switch (type->kind) {
  case TY_VOID:
    return basic_type_json("void");
  case TY_BOOL:
    return basic_type_json("bool");
  case TY_CHAR:
    return basic_type_json("char");
  case TY_BYTE:
    return sign_checked_type_json("byte", type->is_unsigned);
  case TY_SHORT:
    return sign_checked_type_json("short", type->is_unsigned);
  case TY_INT:
    return sign_checked_type_json("int", type->is_unsigned);
  case TY_LONG:
    return sign_checked_type_json("long", type->is_unsigned);
  case TY_FLOAT:
    return basic_type_json("float");
  case TY_DOUBLE:
    return basic_type_json("double");
  case TY_LDOUBLE:
    return basic_type_json("real");
  case TY_ENUM:
    return format("{\"type\": \"enum\"}");
  case TY_PTR:
    return format("{\"type\": \"ptr\", \"elem\": %s}", type_json(type->base));
  case TY_FUNC:
    return format("{\"type\": \"func\", \"return\": %s, \"params\": %s, "
                  "\"variadic\": \"%s\"}",
                  type_json(type->return_ty), params_json(type->params),
                  type->is_variadic ? "true" : "false");
  case TY_ARRAY:
    return format("{\"type\": \"array\", \"elem\": %s}", type_json(type->base));
  case TY_VLA:
    return format("{\"type\": \"vector\", \"elem\": %s}",
                  type_json(type->base));
  case TY_STRUCT:
    return format("{\"type\": \"struct\", \"members\": %s, \"size\": \"%i\"}",
                  members_json(type->members), type->size);
  case TY_UNION:
    return format("{\"type\": \"union\", \"members\": %s, \"size\": \"%i\"}",
                  members_json(type->members), type->size);
  }
  error("unknown type: %i", type->kind);
}

static const char *type_json(Type *type) {
  Type **cur = stack;
  while (*cur != NULL) {
    if (*cur == type) {
      return format("{\"above\": \"%li\"}", cur - stack);
    }
    cur++;
    if (cur - stack >= (1 << 8)) {
      __builtin_trap();
    }
  }
  *cur = type;
  const char *ret = type_json_impl(type);
  *cur = NULL;
  return ret;
}

void codegen(Obj *prog, FILE *out) {
  if (prog == NULL) {
    fprintf(out, "{}");
  } else {
    stack = calloc(1, sizeof(Type *) * (1 << 8));
    const char *res = "";
    for (Obj *fn = prog; fn; fn = fn->next) {
      if (fn->name[0] == '.' || fn->name[0] == '_') {
        continue;
      }
      res = format("%s, \"%s\": %s", res, fn->name, type_json(fn->ty));
    }
    fprintf(out, "{%s}", res + 2);
  }
}
