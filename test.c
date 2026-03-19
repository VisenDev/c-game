#define CORE_STRIP_PREFIX
#define CORE_IMPLEMENTATION
#include "core.h"
#include "generated.c"

int main() {
    entity foo = {0};
    Arena a = {0};
    vec2 pos = {1.0, 2.0};
    foo.name = "bar";
    foo.health = 100;
    vec_append(&foo.path, &a, pos);
    vec_append(&foo.path, &a, pos);
    vec_append(&foo.path, &a, pos);
    vec_append(&foo.path, &a, pos);
    serialize_entity(stdout, &foo);
    printf("\n");
    return 0;
}
