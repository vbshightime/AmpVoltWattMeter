#include <stdio.h>
#include <iostream.h>

struct Characters {
    char_a("Hello"), char_b("world")
    {

    }
    char char_a[10];
    char char_b[10]; 
}

typedef struct Characters Characters;
struct function(struct char_payload){
    char_payload.char_b = "earth"
    struct charcters_new = char_payload;
    return charcters_new;
}

int main(){
    Characters old_characters;
    Characters new_characters = function(old_characters);
    pritf("char_a is %s, char_b is %s",(const char*)new_characters.char_a, (const char*)new_characters.char_b);
}