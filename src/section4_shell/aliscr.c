#include "aliscr.h"
#include "vga.h"
#include "kbd.h"
#include "string.h"

#define MAX_LINES 100
#define LINE_SIZE 64

// --- STACK MACHINE DATA ---
static int stack[64];
static int sp = 0;
static int vars[26];

// --- RECORDING DATA ---
static char script_buffer[MAX_LINES][LINE_SIZE];
static int current_line = 0;

static void push(int v) { if (sp < 64) stack[sp++] = v; }
static int pop() { return (sp > 0) ? stack[--sp] : 0; }

/**
 * THE VM CORE: This is your original logic, 
 * now formatted to handle one line at a time.
 */
void execute_aliscript_line(char* args) {
    if (!args || *args == '\0') return;
    char* ptr = args;
    
    while (*ptr) {
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        if (*ptr == '\0') break;
        
        char token[32]; int t = 0;
        while (*ptr != ' ' && *ptr != '\0' && t < 31) token[t++] = *ptr++;
        token[t] = '\0';

        // Number Logic
        if ((token[0] >= '0' && token[0] <= '9') || (token[0] == '-' && token[1] >= '0')) {
            push(atoi_custom(token));
        } 
        // Math
        else if (strcmp(token, "add") == 0) push(pop() + pop());
        else if (strcmp(token, "sub") == 0) { int b = pop(); push(pop() - b); }
        else if (strcmp(token, "mul") == 0) push(pop() * pop());
        
        // Variables (set_a, get_a, etc)
        else if (strncmp(token, "set_", 4) == 0) vars[token[4] - 'a'] = pop();
        else if (strncmp(token, "get_", 4) == 0) push(vars[token[4] - 'a']);
        
        // Memory Access (PEEK/POKE)
        else if (strcmp(token, "poke") == 0) { 
            int v = pop(); unsigned char* a = (unsigned char*)pop(); *a = (unsigned char)v; 
        }
        else if (strcmp(token, "peek") == 0) push(*(unsigned char*)pop());
        
        // Output
        else if (strcmp(token, "print") == 0) { 
            char b[16]; vga_write(itoa(pop(), b)); vga_write(" "); 
        }
        else if (strcmp(token, "cls") == 0) vga_clear();
        else if (strcmp(token, "plane") == 0) draw_custom_plane();
    }
}

/**
 * THE MULTI-LINE INTERFACE
 */
void cmd_run_script() {
    char input[LINE_SIZE];
    int is_recording = 1;
    current_line = 0;
    sp = 0; // Reset stack for fresh run

    vga_write("--- AliScript Stack VM ---\n");
    vga_write("Type code, then 'doner' to run.\n\n");

    while (is_recording) {
        vga_write("Aliscr> ");
        
        // Blocking Readline
        int i = 0;
        while (i < LINE_SIZE - 1) {
            char c = kbd_get_char();
            if (c == '\n') { input[i] = '\0'; vga_write("\n"); break; }
            else if (c == '\b' && i > 0) { i--; vga_write_char('\b'); }
            else { input[i++] = c; vga_write_char(c); }
        }

        if (strcmp(input, "doner") == 0) {
            is_recording = 0;
            vga_write("[!] Running Sequence...\n");
            for (int j = 0; j < current_line; j++) {
                execute_aliscript_line(script_buffer[j]);
            }
            vga_write("\n[Script Success]\n");
        } else if (current_line < MAX_LINES) {
            strcpy(script_buffer[current_line++], input);
        }
    }
}
