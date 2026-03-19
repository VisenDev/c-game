#define CORE_SEXPR_STRIP_PREFIX
#define CORE_IMPLEMENTATION
#include "core.h"

void handle_field_definition(Sexpr * v, int i, void * ctx) {
    Sexpr string = s_sym("string");
    printf("    ");
    
    if(s_equal(s_first(v), &string)) {
        printf("const char *");
    } else {
        s_print(s_first(v));
    }
    printf(" ");
    s_print(s_second(v));
    printf(";\n");
}

void handle_defstruct(Sexpr * v) {
    printf("typedef struct {\n");
    Sexpr * name = s_car(v);
    s_do_list(s_cdr(v), handle_field_definition, NULL);
    printf("} %s;\n", name->str.v);
}

void handle_toplevel_form(Sexpr * v, int i, void * ctx) {
    Sexpr def = s_sym("defrecord");
    if(s_equal(s_car(v), &def)) {
        handle_defstruct(s_cdr(v));
    } else {
        s_fprint(stderr, s_car(v));
        fprintf(stderr, "\n");
        CORE_FATAL_ERROR("Unexpected list header");
    }
}

int main() {
    core_Arena a = {0};
    Sexpr * s = s_read(&a, "structs.sexpr");
    s_do_list(s, handle_toplevel_form, NULL);
    core_arena_free(&a);
    return 0;
}
