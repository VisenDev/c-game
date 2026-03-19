#define CORE_SEXPR_STRIP_PREFIX
#define CORE_IMPLEMENTATION
#include "core.h"

/* void process_record(core_Sexpr s) { */
/*     char * name; */
/*     s_Cons item; */
/*     assert(s.tag == CONS); */
/*     name = core_sexpr_first(s).as.string; */
/*     item = s.as.cons->cdr; */

/*     printf("typedef struct {\n"); */
/*     for(;item.as.cons->cdr.tag == CONS; item = item->as.cons */
        
/*         char * field_type = core_sexpr_first(item).as.symbol; */
/*         char * field_name = core_sexpr_second(item).as.symbol; */
/*         printf("    "); */
/*         if(strcmp(field_type, "string") == 0) { */
/*             printf("char * "); */
/*         } else { */
/*             printf("%s ", field_type); */
/*         } */
/*         printf("%s;\n", field_name); */
                    
/*     } */
/*     printf("} %s;\n\n", name); */
    
/* } */

/* void process(core_Sexpr s) { */
/*     assert(s.tag == CONS); */
/*     core_Sexpr car = s.as.cons->car; */
/*     core_Sexpr cdr = s.as.cons->cdr; */
/*     assert(car.tag == SYM); */
/*     if(strcmp(car.as.string, "defrecord") == 0) { */
/*         process_record(cdr); */
/*     } */
/* } */

int main() {
    core_Arena a = {0};
    Sexpr * s = s_read(&a, "structs.sexpr");
    Sexpr * iter = s;
    Sexpr * item;
    Sexpr * sub_iter;
    Sexpr * sub_item;
    int i = 0;


    S_DO_LIST(item, iter) {
        Sexpr * name;


        sub_iter = item;
        i = 0;

        printf("typedef struct {\n");
        S_DO_LIST(sub_item, sub_iter) {
            if(i == 0) {
                if(!s_equal(*sub_item, s_sym("DEFRECORD"))) 
                    CORE_FATAL_ERROR("Expected DEFRECORD form");
            } else if(i == 1) {
                name = sub_item;
            } else {
                printf("    ");
            }
            s_print(sub_item);
            printf(" ");
            ++i;
        }
        puts("");
    }
    puts("");
    return 0;
}
