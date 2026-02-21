#ifndef SHELL_H
#define SHELL_H

/* Function pointer for command execution */
typedef void (*command_func)(char* args);

/* Dynamic Command Node */
typedef struct command_node {
    char name[32];
    char description[64];
    command_func function;
    struct command_node* next;
} command_node_t;

/* Public API */
void shell_init();
void shell_register_command(const char* name, const char* desc, command_func func);
void shell_dispatch(char* buffer);
void shell_tab_complete(char* buffer, int* len);

/* External dependencies */
extern void vga_write(const char* data);
extern void vga_clear();
extern void vga_putchar(char c);

#endif
