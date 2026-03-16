#define CORE_IMPLEMENTATION
#include "core.h"

#define CONS CORE_TAG_CONS
#define NIL CORE_TAG_NIL
#define STR CORE_TAG_STRING
#define SYM CORE_TAG_SYMBOL

void process_record(core_Sexpr s) {
    assert(s.tag == CONS);
    char * name = core_sexpr_first(s).as.string;
    s = s.as.cons->cdr;

    printf("typedef struct {\n");
    while(item) {
        
        char * field_type = core_sexpr_first(item).as.symbol;
        char * field_name = core_sexpr_second(item).as.symbol;
        printf("    ");
        if(strcmp(field_type, "string") == 0) {
            printf("char * ");
        } else {
            printf("%s ", field_type);
        }
        printf("%s;\n", field_name);
                    
    }
    printf("} %s;\n\n", name);
    
}

void process(core_Sexpr s) {
    assert(s.tag == CONS);
    core_Sexpr car = s.as.cons->car;
    core_Sexpr cdr = s.as.cons->cdr;
    assert(car.tag == SYM);
    if(strcmp(car.as.string, "defrecord") == 0) {
        process_record(cdr);
    }
}

int main() {
    core_Arena a = {0};
    core_Sexpr s = {0};
    char * str;
    int i = 0;
    str = core_file_read_all_arena(&a, "structs.sexpr");
    while (1) {
        int j = core_sexpr_read(&a, str + i, &s);
        if(j == 0) break;
        process(s);
        i += j;
    }
    return 0;
}
