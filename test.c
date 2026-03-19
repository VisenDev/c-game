#include "generated.c"

int main() {
    entity foo = {0};
    foo.name = "bar";
    serialize_entity(stdout, &foo);
    printf("\n");
    return 0;
}
