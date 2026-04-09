#include "aliscr.h"
#include "shell.h"

static int stack[64];
static int sp = 0;
static int vars[26];

static void push(int v) { if (sp < 64) stack[sp++] = v; }
static int pop() { return (sp > 0) ? stack[--sp] : 0; }

void cmd_run_script(char* args) {
    if (!args || *args == '\0') { vga_write("AliScript Error: No code.\n"); return; }
    char* ptr = args;
    while (*ptr) {
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        if (*ptr == '\0') break;
        char token[32]; int t = 0;
        while (*ptr != ' ' && *ptr != '\0' && t < 31) token[t++] = *ptr++;
        token[t] = '\0';

        if ((token[0] >= '0' && token[0] <= '9') || (token[0] == '-' && token[1] >= '0')) push(atoi_custom(token));
        else if (token[0] == '0' && token[1] == 'x') { /* Hex Logic */
            int hval = 0; for(int j=2; token[j]; j++) {
                hval *= 16;
                if(token[j] >= '0' && token[j] <= '9') hval += token[j] - '0';
                else if(token[j] >= 'a' && token[j] <= 'f') hval += token[j] - 'a' + 10;
                else if(token[j] >= 'A' && token[j] <= 'F') hval += token[j] - 'A' + 10;
            } push(hval);
        }
        else if (strcmp(token, "add") == 0) push(pop() + pop());
        else if (strcmp(token, "sub") == 0) { int b = pop(); push(pop() - b); }
        else if (strcmp(token, "mul") == 0) push(pop() * pop());
        else if (strncmp(token, "set_", 4) == 0) vars[token[4] - 'a'] = pop();
        else if (strncmp(token, "get_", 4) == 0) push(vars[token[4] - 'a']);
        else if (strcmp(token, "poke") == 0) { int v = pop(); unsigned char* a = (unsigned char*)pop(); *a = (unsigned char)v; }
        else if (strcmp(token, "peek") == 0) push(*(unsigned char*)pop());
        else if (strcmp(token, "print") == 0) { char b[16]; vga_write(itoa(pop(), b)); vga_write(" "); }
        else if (strcmp(token, "cls") == 0) vga_clear();
        else if (strcmp(token, "plane") == 0) draw_custom_plane();
    }
    vga_write("\n[Script Success]\n"); sp = 0;
}
