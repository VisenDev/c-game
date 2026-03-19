#define CORE_SEXPR_STRIP_PREFIX
#define CORE_STRIP_PREFIX
#define CORE_IMPLEMENTATION
#include "core.h"

void handle_field_definition(Sexpr * v, int i, void * ctx) {
    Sexpr * t = s_first(v);
    Sexpr * n = s_second(v);
    const char * type = t->sym.v;
    const char * name = n->sym.v;
    size_t len = strlen(type);
    Bool array = CORE_FALSE;
    char buf[1024];
    memcpy(buf, type, MIN(len + 1, sizeof(buf)));
    buf[sizeof(buf) - 1] = 0;

    assert(t->tag == S_SYM);
    assert(n->tag == S_SYM);

    if(len > 2 && type[len - 2] =='[' && type[len - 1] == ']') {
        Bool array = true;
        buf[len - 2] = 0;
    }

    Sexpr string = s_sym("string");
    printf("    ");

    if(array) {
        printf("core_Vec(");
    }
    
    if(s_equal(s_first(v), &string)) {
        printf("const char *");
    } else {
        printf("%s", buf);
    }

    if(array) {
        printf(")");
    }
    printf(" ");
    s_print(s_second(v));
    printf(";\n");
}

void generate_serialize_field_statement(Sexpr * field, int i, void * ctx) {
    Sexpr * t = s_first(field);
    Sexpr * n = s_second(field);
    const char * type = t->sym.v;
    const char * name = n->sym.v;
    assert(t->tag == S_SYM);
    assert(n->tag == S_SYM);

    printf("    /*%s*/\n", name);
    printf("    fprintf(stream, \" :%s \");\n", name);
    if(streql(type, "float")) {
        printf("    fprintf(stream, \"%%f\", value->%s);\n", name);
    } else if(streql(type, "int")) {
        printf("    fprintf(stream, \"%%d\", value->%s);\n", name);
    } else if(streql(type, "long")) {
        printf("    fprintf(stream, \"%%ld\", value->%s);\n", name);
    } else if(streql(type, "string")) {
        printf("    fprintf(stream, \"\\\"%%s\\\"\", value->%s);\n", name);
    } else {
        printf("    serialize_%s(stream, &value->%s);\n", type, name);
    }
}

void generate_serialize_function(const char * name, Sexpr * fields) {
    printf("void serialize_%s(FILE * stream, %s * value) {\n", name, name);
    printf("    if(!stream || !value) return;\n");
    printf("    fprintf(stream, \"(%s\");\n", name);
    s_do_list(fields, generate_serialize_field_statement, NULL);
    printf("    fprintf(stream, \")\");\n");
    printf("}\n\n");
}

void handle_defstruct(Sexpr * v) {
    printf("typedef struct {\n");
    const char * name = s_car(v)->sym.v;
    s_do_list(s_cdr(v), handle_field_definition, NULL);
    printf("} %s;\n\n", name);

    generate_serialize_function(name, s_cdr(v));
}

void handle_toplevel_form(Sexpr * v, int i, void * ctx) {
    Sexpr def = s_sym("defrecord");
    if(s_equal(s_car(v), &def)) {
        handle_defstruct(s_cdr(v));
    } else {
        s_fprint(stderr, s_car(v));
        fprintf(stderr, "\n");
        FATAL_ERROR("Unexpected list header");
    }
}

int main() {
    Arena a = {0};
    Sexpr * s = s_read(&a, "structs.sexpr");
    printf("#include <stdio.h>\n");
    s_do_list(s, handle_toplevel_form, NULL);
    arena_free(&a);
    return 0;
}
