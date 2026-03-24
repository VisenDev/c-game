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
    (void)i;
    (void)ctx;
    printf("    ");
    print_type(s_first(v), CORE_FALSE);
    printf(" ");
    s_print(s_second(v));
    printf(";\n");
}

void generate_serialize_value_statement(Sexpr * stype, const char * accessor) {
    if(stype->tag == S_SYM) {
        const char * type = stype->sym.v;

        if(streql(type, "float")) {
            printf("    if(%s - (long)%s <= FLT_EPSILON) {\n", accessor, accessor);
            printf("        fprintf(stream, \" %%ld.0\", (long)%s);\n", accessor);
            printf("    } else {\n");
            printf("        fprintf(stream, \" %%f\", (double)%s);\n", accessor);
            printf("    }\n");
        } else if(streql(type, "double")) {
            printf("    if(%s - (long)%s <= DBL_EPSILON) {\n", accessor, accessor);
            printf("        fprintf(stream, \" %%ld.0\", (long)%s);\n", accessor);
            printf("    } else {\n");
            printf("        fprintf(stream, \" %%f\", %s);\n", accessor);
            printf("    }\n");
        } else if(streql(type, "int")) {
            printf("    fprintf(stream, \" %%d\", %s);\n", accessor);
        } else if(streql(type, "long")) {
            printf("    fprintf(stream, \" %%ld\", %s);\n", accessor);
        } else if(streql(type, "string")) {
            printf("    fprintf(stream, \" \\\"%%s\\\"\", %s);\n", accessor);
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
    (void)i;
    (void)ctx;
    core_snprintf(buf, sizeof(buf), "value->%s", name);
    printf("    /*%s*/\n", name);
    generate_serialize_value_statement(stype, buf);
} 

void generate_serialize_function(Sexpr * sname, Sexpr * fields) {
    const char * name = sname->str.v;
    printf("void serialize_%s(FILE * stream, %s * value) {\n", name, name);
    printf("    if(!stream || !value) return;\n");
    printf("    fprintf(stream, \" (%s\");\n", name);
    s_do_list(fields, generate_serialize_field_statement, NULL);
    printf("    fprintf(stream, \")\");\n");
    printf("}\n\n");

    printf("void serialize_%s_array(FILE * stream, %s * items, size_t n) {\n", name, name);
    printf("    size_t i;\n");
    printf("    if(!stream || !items) return;\n");
    printf("    fprintf(stream, \" (list \");\n");
    printf("    for(i = 0; i < n; ++i) {\n");
    printf("    "); generate_serialize_value_statement(sname, "items[i]");
    printf("    }\n");
    printf("    fprintf(stream, \")\");\n");
    printf("}\n\n");

}

void generate_deserialize_value_statement(Sexpr * type, const char * dst) {
    if(type->tag == S_SYM) {
        if(streql(type->sym.v, "float")) {
            printf("    if(!tmp || tmp->tag != CORE_SEXPR_REAL) return CORE_FALSE;\n");
            printf("    %s = (float)tmp->f.v;\n", dst);
        } else if(streql(type->sym.v, "double")) {
            printf("    if(!tmp || tmp->tag != CORE_SEXPR_REAL) return CORE_FALSE;\n");
            printf("    %s = (float)tmp->f.v;\n", dst);
        } else if(streql(type->sym.v, "int")) {
            printf("    if(!tmp || tmp->tag != CORE_SEXPR_INT) return CORE_FALSE;\n");
            printf("    %s = (int)tmp->i.v;\n", dst);
        } else if(streql(type->sym.v, "long")) {
            printf("    if(!tmp || tmp->tag != CORE_SEXPR_INT) return CORE_FALSE;\n");
            printf("    %s = tmp->i.v;\n", dst);
        } else if(streql(type->sym.v, "string")) {
            printf("    if(!tmp || tmp->tag != CORE_SEXPR_STR) return CORE_FALSE;\n");
            printf("    %s = core_arena_strdup(arena, tmp->str.v);\n", dst);
        } else {
            printf("    if(!deserialize_");
            print_type(type, CORE_TRUE);
            printf("(arena, tmp, &%s)) return CORE_FALSE;\n", dst);
        }
    } else {
        printf("    if(core_sexpr_car(tmp)->tag != CORE_SEXPR_SYM) return CORE_FALSE;\n");
        printf("    if(!core_streql(core_sexpr_car(tmp)->sym.v, \"list\")) return CORE_FALSE;\n");
        printf("    memset(&%s, 0,sizeof(%s));\n", dst, dst);
        printf("    {\n");
        printf("        core_Sexpr * next = core_sexpr_cdr(tmp);\n");
        printf("        for(;next->tag == CORE_SEXPR_CONS; next = next->cons.cdr) {\n");
        printf("            "); print_type(s_second(type), CORE_FALSE); printf(" item_tmp;\n");        
        printf("            tmp = core_sexpr_car(next);\n");
        printf("        "); generate_deserialize_value_statement(s_second(type), "item_tmp");
        printf("            core_vec_append(&%s, arena, item_tmp);\n", dst);
        printf("         }\n");
        printf("     }\n");
        /* printf("            core_vec_append(&output->%s, arena, deserialize_", name); */
        /* print_type(type, CORE_TRUE); */
        /* printf("(arena, /\*TODO*\/, /\*TODO*\/));\n"); */
        /* printf("        }\n    }\n"); */
        /* /\* printf *\/("    /\*TODO array types*\/\n") */;
        /* ARRAY TYPE*/

    }
    
}

void generate_deserialize_field_statement(Sexpr * field, int i, void * ctx) {
    Sexpr * type = s_first(field);
    Sexpr * sname = s_second(field);
    const char * name;
    char buf[1024];
    (void)ctx;
    assert(sname->tag == S_SYM);
    name = sname->sym.v;
    printf("    /*%s*/\n", name);
    printf("    tmp = core_sexpr_nth(fields, %d);\n", i);
    core_snprintf(buf, sizeof(buf), "output->%s", name);
    generate_deserialize_value_statement(type, buf);
}

void generate_deserialize_function(Sexpr * sname, Sexpr * fields) {
    {
        printf("core_Bool deserialize_");
        print_type(sname, CORE_TRUE);
        printf("(core_Arena * arena, Sexpr * input, ");
        print_type(sname, CORE_FALSE);
        printf(" * output) {\n");
    }
    printf("    core_Sexpr * fields;\n");
    printf("    core_Sexpr * tmp;\n");
    printf("    if(!output || !input || !arena) return CORE_FALSE;\n");
    printf("    if(input->tag != CORE_SEXPR_CONS) return CORE_FALSE;\n");
    printf("    fields = core_sexpr_cdr(input);\n");
    printf("    if(!input->cons.car) return CORE_FALSE;\n");
    printf("    if(input->cons.car->tag != CORE_SEXPR_SYM) return CORE_FALSE;\n");
    {
        printf("    if(!core_streql(input->cons.car->sym.v, \"");
        print_type(sname, CORE_TRUE);
        printf("\")) return CORE_FALSE;\n");
    }
    s_do_list(fields, generate_deserialize_field_statement, NULL);
    printf("    return CORE_TRUE;\n");
    printf("}\n\n");
}

void handle_defstruct(Sexpr * v) {
    const char * name = s_car(v)->sym.v;
    printf("typedef struct {\n");
    s_do_list(s_cdr(v), handle_field_definition, NULL);
    printf("} %s;\n\n", name);

    generate_serialize_function(s_car(v), s_cdr(v));
    generate_deserialize_function(s_car(v), s_cdr(v));
}

void handle_toplevel_form(Sexpr * v, int i, void * ctx) {
    Sexpr def = s_sym("defrecord");
    (void)i;
    (void)ctx;
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
    puts("/*AUTOGENERATED FILE: DO NOT EDIT*/\n");
    printf("#include <stdio.h>\n");
    printf("#include <assert.h>\n");
    printf("#include <float.h>\n");
    printf("#include \"core.h\"\n");
    puts("");
    s_do_list(s, handle_toplevel_form, NULL);
    arena_free(&a);
    return 0;
}
