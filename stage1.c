#define CORE_SEXPR_STRIP_PREFIX
#define CORE_STRIP_PREFIX
#define CORE_IMPLEMENTATION
#include "core.h"

void print_type(Sexpr * type, Bool readably) {
    if(type->tag == S_CONS) {
        if(streql(s_first(type)->sym.v, "array")) {
            if(readably) {
                print_type(s_second(type), readably);
                printf("_array");
            } else {
                printf("core_Vec(");
                print_type(s_second(type), readably);
                printf(")");
            }
        } else {
            FATAL_ERROR("Unknown compound type");
        }
    } else if(type->tag == S_SYM) {
        const char * s = type->sym.v;
        if(streql(s, "string")) {
            if(readably) {
                printf("string");
            } else {
                printf("const char * ");
            } 
        } else {
            printf("%s", type->sym.v);
        }
    } else UNREACHABLE;
}

void handle_field_definition(Sexpr * v, int i, void * ctx) {
    printf("    ");
    print_type(s_first(v), CORE_FALSE);
    printf(" ");
    s_print(s_second(v));
    printf(";\n");
}

void generate_serialize_value_statement(Sexpr * stype, char * accessor) {
    if(stype->tag == S_SYM) {
        const char * type = stype->sym.v;

        if(streql(type, "float")) {
            printf("    fprintf(stream, \"%%f\", %s);\n", accessor);
        } else if(streql(type, "int")) {
            printf("    fprintf(stream, \"%%d\", %s);\n", accessor);
        } else if(streql(type, "long")) {
            printf("    fprintf(stream, \"%%ld\", %s);\n", accessor);
        } else if(streql(type, "string")) {
            printf("    fprintf(stream, \"\\\"%%s\\\"\", %s);\n", accessor);
        } else {
            printf("    serialize_");
            print_type(stype, CORE_TRUE);
            printf("(stream, &%s);\n", accessor);
        }
    } else {
        printf("    serialize_");
        print_type(stype, CORE_TRUE);
        printf("(stream, %s.items, %s.len);\n", accessor, accessor);
    }
}


void generate_serialize_field_statement(Sexpr * field, int i, void * ctx) {
    Sexpr * stype = s_first(field);
    const char * name = s_second(field)->sym.v;
    char buf[1024];
    core_snprintf(buf, sizeof(buf), "value->%s", name);
    /* printf("    /\*%s*\/\n", name); */
    printf("    fprintf(stream, \" :%s \");\n", name);
    generate_serialize_value_statement(stype, buf);
} 

void generate_serialize_function(Sexpr * sname, Sexpr * fields) {
    const char * name = sname->str.v;
    printf("void serialize_%s(FILE * stream, %s * value) {\n", name, name);
    printf("    if(!stream || !value) return;\n");
    printf("    fprintf(stream, \" #S(%s\");\n", name);
    s_do_list(fields, generate_serialize_field_statement, NULL);
    printf("    fprintf(stream, \")\");\n");
    printf("}\n\n");

    printf("void serialize_%s_array(FILE * stream, %s * items, size_t n) {\n", name, name);
    printf("    size_t i;\n");
    printf("    if(!stream || !items) return;\n");
    printf("    fprintf(stream, \"(list \");\n");
    printf("    for(i = 0; i < n; ++i) {\n");
    generate_serialize_value_statement(sname, "items[i]");
    printf("    }");
    printf("    fprintf(stream, \")\");\n");
    printf("}\n\n");

}

void generate_deserialize_function(Sexpr * sname, Sexpr * fields) {
    printf("core_Bool deserialize_%s(FILE * stream, Sexpr * input, %s * output) {\n"
           print_type(sname, CORE_TRUE), print_);
    printf("    if(!stream || !input) return CORE_FALSE;\n");
    printf("    if(input->tag != CORE_SEXPR_CONS) return CORE_FALSE;\n");
    printf("    
}

void handle_defstruct(Sexpr * v) {
    printf("typedef struct {\n");
    const char * name = s_car(v)->sym.v;
    s_do_list(s_cdr(v), handle_field_definition, NULL);
    printf("} %s;\n\n", name);

    generate_serialize_function(s_car(v), s_cdr(v));
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
    printf("#include \"core.h\"\n");
    s_do_list(s, handle_toplevel_form, NULL);
    arena_free(&a);
    return 0;
}
