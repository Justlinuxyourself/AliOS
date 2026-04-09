/* 
Copyright (c) 2026 Ali  
All rights reserved.
*/
#include "shell.h"
#include "../section1_cpu/heap.h"
#include "../section1_cpu/io.h"
#include "aliscr.h"
volatile int is_sleeping = 0;
static command_node_t* command_list = 0;
extern unsigned int get_heap_usage();
extern unsigned int get_uptime_seconds();
extern unsigned int get_total_ram_bytes();
extern void lock_system_hardened();
extern int timezone_offset_seconds;
extern void vga_draw_status_bar();
extern void play_sound();
extern void nosound();
extern void vga_set_cursor();
extern void sleep();
/* --- String Helpers --- */
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, int n) {
    while (n--) {
        if (*s1 != *s2++) return *(unsigned char*)s1 - *(unsigned char*)--s2;
        if (*s1++ == 0) break;
    }
    return 0;
}

int strlen(const char* s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

void strcpy(char* dest, const char* src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}
/* Helper: Reverse a string in place */
void reverse(char* str, int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

/* itoa: Convert integer to string (Base 10 only) */
char* itoa(int value, char* str) {
    int i = 0;
    int isNegative = 0;

    /* Handle 0 explicitly */
    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    /* Handle negative numbers */
    if (value < 0) {
        isNegative = 1;
        value = -value;
    }

    /* Process individual digits in Base 10 */
    while (value != 0) {
        int rem = value % 10;
        str[i++] = rem + '0';
        value = value / 10;
    }

    if (isNegative) {
        str[i++] = '-';
    }

    str[i] = '\0';
    reverse(str, i);

    return str;
}
// Simple atoi implementation to convert string to integer
int atoi_custom(char* str) {
    int res = 0;
    int sign = 1;
    int i = 0;

    if (str[0] == '-') {
        sign = -1;
        i++;
    } else if (str[0] == '+') {
        i++; // Just skip the plus sign and stay positive
    }

    for (; str[i] != '\0'; ++i) {
        if (str[i] < '0' || str[i] > '9') break;
        res = res * 10 + str[i] - '0';
    }
    return sign * res;
}
/* itohex: Convert integer to Hexadecimal string */
char* itohex(unsigned long value, char* str) {
    char* hex_chars = "0123456789ABCDEF";
    int i = 0;
    
    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    while (value > 0) {
        str[i++] = hex_chars[value % 16];
        value /= 16;
    }
    str[i] = '\0';
    reverse(str, i);
    return str;
}

/* --- Built-in Commands --- */
void cmd_help(char* args) {
    vga_write("\nAliOS 4 Commands:");
    command_node_t* curr = command_list;
    while (curr) {
        vga_write("\n  ");
        vga_write(curr->name);
        vga_write(" - ");
        vga_write(curr->description);
        curr = curr->next;
    }
}

void cmd_cls(char* args) {
    vga_clear();
}

void cmd_echo(char* args) {
    if (args) {
//        vga_write("\n");
        vga_write(args);
    }
}

void cmd_neofetch(char* args) {
    unsigned int a, b, c, d;
    char vendor[13];
    // Get CPU Vendor String (e.g., "GenuineIntel" or "AuthenticAMD")
    cpuid(0, &a, (unsigned int*)&vendor[0], (unsigned int*)&vendor[8], (unsigned int*)&vendor[4]);
    vendor[12] = '\0';

    char mem_str[16];
    itoa(get_heap_usage(), mem_str); // Get dynamic memory stats

    vga_write("   ______      AliOS 4.0\n");
    vga_write("  / ____/      ----------\n");
    vga_write(" / /  __       CPU: "); vga_write(vendor); vga_write("\n");
    vga_write("/ /__/ /       MEM: "); vga_write(mem_str); vga_write(" bytes used\n");
    vga_write("\\____ /        HEAP: 0x200000\n");
    vga_write("               MODE: 64-bit Long Mode\n");
}
void cmd_uptime(char* args) {
    char sec_str[16];
    itoa(get_uptime_seconds(), sec_str); // Use the real timer data
    
    vga_write("uptime: ");
    vga_write(sec_str);
    vga_write(" seconds.\n");
}
void cmd_free(char* args) {
    unsigned int total = (unsigned int)get_total_ram_bytes();
    unsigned int used = get_heap_usage();
    unsigned int free = total - used;

    char t_str[16], u_str[16], f_str[16];
    
    // No more dividing by 1024!
    itoa(total, t_str);
    itoa(used, u_str);
    itoa(free, f_str);

    vga_write("\nMemory Usage (Bytes):");
    vga_write("\n  Total: "); vga_write(t_str);
    vga_write("\n  Used:  "); vga_write(u_str);
    vga_write("\n  Free:  "); vga_write(f_str);
    vga_write("\n");
}
 
void shell_cmd_timezone(char* arg) {
    if (arg == 0 || arg[0] == '\0') {
        vga_write("Usage: timezone [hours] [seconds]\n");
        return;
    }

    int h = 0, s = 0;
    char* second_part = 0;

    // Split "hours" and "seconds"
    for (int i = 0; arg[i]; i++) {
        if (arg[i] == ' ') {
            arg[i] = '\0';
            second_part = &arg[i+1];
            break;
        }
    }

    h = atoi_custom(arg); 
    if (second_part) s = atoi_custom(second_part);

    // Logic: If hours are negative, seconds should usually be subtracted too
    // Example: GMT-5:30 means -5 hours AND -30 minutes
    int total;
    if (h < 0) {
        total = (h * 3600) - s; 
    } else {
        total = (h * 3600) + s;
    }

    timezone_offset_seconds = total;

    vga_write("Timezone offset set to ");
    char buf[16];
    vga_write(itoa(timezone_offset_seconds, buf));
    vga_write(" seconds.\n");

    // CRITICAL: Refresh the screen so you see the change!
    vga_draw_status_bar(); 
}

void shell_lock() {
    lock_system_hardened();
}
void cmd_test(char* args) {
    vga_write("Calibrating timer (5s wait)...\n");
    for(int i = 5; i > 0; i--) {
        char buf[4];
        itoa(i, buf);
        vga_write(buf);
        vga_write("... ");
        sleep(1);
    }
    vga_write("\nTest complete.\n");
}
void cmd_beep(){
    vga_write("Beeping...");
    play_sound(1000);
    sleep(1);
    nosound();
}
void cmd_about_dev() {
	vga_write("Hi my name is ali, my age is 12, and i like planes :3");
}
void draw_custom_plane() {
    int col = 25; // Center it
    
    vga_set_cursor(col, 5);  vga_write("            __\\/__");
    vga_set_cursor(col, 6);  vga_write("           `==/\\==` ");
    vga_set_cursor(col, 7);  vga_write(" ____________/__\\____________");
    vga_set_cursor(col, 8);  vga_write("/____________________________\\");
    vga_set_cursor(col, 9);  vga_write("  __||__||__/.--.\\__||__||__");
    vga_set_cursor(col, 10); vga_write(" /__|___|___( >< )___|___|__\\");
    vga_set_cursor(col, 11); vga_write("           _/`--`\\_");
    vga_set_cursor(col, 12); vga_write("          (/------\\)");
}

void twins() {
    vga_write("APHRODITE\n");
    vga_write("SAKI (BEST SUNDAY GOONER)\n");
    vga_write("QOQO\n");
    vga_write("KEI\n");
    vga_write("LWAH\n");
    vga_write("O1\n");
    vga_write("O2\n");
    vga_write("RAYA THE KARAOKE QUEEN\n");
    vga_write("SEL\n");
    vga_write("ISHI\n");
    vga_write("ZAZA\n");
    vga_write("VANILLA & MAX\n");
    vga_write("SUSTUBE\n");
    vga_write("ADNAN\n");
    vga_write("ZIKE (FAF)\n");
    vga_write("MOLY\n");
    vga_write("ROSIE\n");
    vga_write("E\n");
    vga_write("Sillycat\n");
    vga_write("Kaisi\n");
    vga_write("ZANNNNNNNN\n");
    vga_write("ABDUALLAH\n");
}

void sys_sleep() {
    is_sleeping = 1;
    vga_clear();
    vga_write("SYSTEM SLEEPING... Press ANY key to wake.");

    // We CANNOT use hlt if we don't have ISRs set up.
    // Instead, we just loop and poll the keyboard.
    while(is_sleeping) {
        // Look at the keyboard status port (0x64)
        // Bit 0 is set if there is data in the buffer
        if (inb(0x64) & 1) {
            unsigned char scan = inb(0x60);
            if (scan < 0x80) { // Any key "Make" code
                is_sleeping = 0;
            }
        }
        
        // Give the CPU a tiny rest without fully halting
        __asm__ volatile("pause"); 
    }

    vga_clear();
    vga_draw_status_bar();
    lock_system_hardened();
}
void command_calc(char* args) {
    if (args == 0 || *args == '\0') {
        vga_write("Usage: calc [num1] [op] [num2]\n");
        vga_write("Example: calc 5 a 10\n");
        return;
    }

    char* part1 = args;
    char* part2 = 0;
    char* part3 = 0;

    // 1. Find the first space to get the operator
    for (int i = 0; args[i]; i++) {
        if (args[i] == ' ') {
            args[i] = '\0';     // Terminate num1
            part2 = &args[i+1]; // Start of operator
            break;
        }
    }

    if (!part2) { vga_write("Error: Missing operator.\n"); return; }

    // 2. Find the second space to get the second number
    for (int i = 0; part2[i]; i++) {
        if (part2[i] == ' ') {
            part2[i] = '\0';    // Terminate operator
            part3 = &part2[i+1]; // Start of num2
            break;
        }
    }

    if (!part3) { vga_write("Error: Missing second number.\n"); return; }

    int n1 = atoi_custom(part1);
    char op = part2[0]; 
    int n2 = atoi_custom(part3);
    int result = 0;

    if (op == 'a')      result = n1 + n2;
    else if (op == 's') result = n1 - n2;
    else if (op == 'm') result = n1 * n2;
    else if (op == 'd') {
        if (n2 == 0) { vga_write("Error: Div by 0\n"); return; }
        result = n1 / n2;
    } else {
        vga_write("Error: Use a/s/m/d\n");
        return;
    }

    char res_buffer[32];
    itoa(result, res_buffer);
    
    vga_write("Result: ");
    vga_write(res_buffer);
    vga_write("\n");
}

void cmd_peek(char* args) {
    if (args == 0 || *args == '\0') {
        vga_write("Usage: peek [hex_address]\nExample: peek 0xB8000\n");
        return;
    }

    // Simple hex string to long converter
    unsigned long addr = 0;
    int start = 0;
    if (args[0] == '0' && (args[1] == 'x' || args[1] == 'X')) start = 2;

    for (int i = start; args[i] != '\0'; i++) {
        addr *= 16;
        if (args[i] >= '0' && args[i] <= '9') addr += (args[i] - '0');
        else if (args[i] >= 'a' && args[i] <= 'f') addr += (args[i] - 'a' + 10);
        else if (args[i] >= 'A' && args[i] <= 'F') addr += (args[i] - 'A' + 10);
    }

    unsigned char* ptr = (unsigned char*)addr;
    vga_write("Memory at 0x");
    vga_write(args);
    vga_write(": ");

    // Show the next 8 bytes
    for (int i = 0; i < 8; i++) {
        char buf[3];
        itohex(ptr[i], buf);
        if (ptr[i] < 16) vga_write("0"); // Padding
        vga_write(buf);
        vga_write(" ");
    }
}
void cmd_poke(char* args) {
    if (args == 0 || *args == '\0') {
        vga_write("Usage: poke [addr] [val]\nExample: poke 0xB8000 0x41\n");
        return;
    }

    char* addr_str = args;
    char* val_str = 0;

    // 1. Split the string at the space
    for (int i = 0; args[i]; i++) {
        if (args[i] == ' ') {
            args[i] = '\0';
            val_str = &args[i+1];
            break;
        }
    }

    if (!val_str) {
        vga_write("Error: Missing value.\n");
        return;
    }

    // 2. Parse Address (Hex or Dec)
    unsigned long addr = 0;
    int i = 0;
    if (addr_str[0] == '0' && (addr_str[1] == 'x' || addr_str[1] == 'X')) {
        i = 2;
        while (addr_str[i]) {
            addr *= 16;
            if (addr_str[i] >= '0' && addr_str[i] <= '9') addr += (addr_str[i] - '0');
            else if (addr_str[i] >= 'a' && addr_str[i] <= 'f') addr += (addr_str[i] - 'a' + 10);
            else if (addr_str[i] >= 'A' && addr_str[i] <= 'F') addr += (addr_str[i] - 'A' + 10);
            i++;
        }
    } else {
        addr = (unsigned long)atoi_custom(addr_str);
    }

    // 3. Parse Value (Hex or Dec)
    unsigned char val = 0;
    if (val_str[0] == '0' && (val_str[1] == 'x' || val_str[1] == 'X')) {
        int j = 2;
        while (val_str[j]) {
            val *= 16;
            if (val_str[j] >= '0' && val_str[j] <= '9') val += (val_str[j] - '0');
            else if (val_str[j] >= 'a' && val_str[j] <= 'f') val += (val_str[j] - 'a' + 10);
            else if (val_str[j] >= 'A' && val_str[j] <= 'F') val += (val_str[j] - 'A' + 10);
            j++;
        }
    } else {
        val = (unsigned char)atoi_custom(val_str);
    }

    // 4. The Poke: Write to raw memory
    unsigned char* ptr = (unsigned char*)addr;
    *ptr = val;

    vga_write("Memory modified at 0x");
    vga_write(addr_str);
    vga_write("\n");
}

/* --- Shell Logic --- */
void shell_register_command(const char* name, const char* desc, command_func func) {
    command_node_t* new_node = (command_node_t*)kmalloc(sizeof(command_node_t));
    
    int i = 0;
    while(name[i] && i < 31) { new_node->name[i] = name[i]; i++; }
    new_node->name[i] = '\0';

    i = 0;
    while(desc[i] && i < 63) { new_node->description[i] = desc[i]; i++; }
    new_node->description[i] = '\0';

    new_node->function = func;
    new_node->next = command_list;
    command_list = new_node;
}

void shell_init() {
    shell_register_command("help", "List all available commands", cmd_help);
    shell_register_command("cls",  "Clear the notebook screen",   cmd_cls);
    shell_register_command("echo", "Print text to the screen",    cmd_echo);
    shell_register_command("neofetch", "Display dynamic system info", cmd_neofetch);
    shell_register_command("uptime", "Show how long AliOS has been running", cmd_uptime);
    shell_register_command("free", "Check dynamic RAM usage", cmd_free);
    shell_register_command("timezone", "Adjust the status bar clock offset", shell_cmd_timezone);
    shell_register_command("lock", "Locks the system", shell_lock);
    shell_register_command("test",     "Verify timer calibration",    cmd_test);
    shell_register_command("beep", "Play a system alert sound", cmd_beep);
    shell_register_command("about_dev", "About Dev", cmd_about_dev);
    shell_register_command("plane", "Show a art of a plane", draw_custom_plane);
    shell_register_command("twins", "Shows my twins names", twins);
    shell_register_command("sleep", "Sleep", sys_sleep);
    shell_register_command("calc", "Calculator", command_calc);
    shell_register_command("peek", "Inspect raw memory addresses", cmd_peek);
    shell_register_command("poke", "Write to memory addrs", cmd_poke);
    shell_register_command("run", "Execute AliScript code", cmd_run_script);
}

}

/* src/section4_shell/shell.c */

void shell_dispatch(char* buffer) {
    // If the user just hits enter, just print a new prompt on a new line
    if (strlen(buffer) == 0) {
        vga_write("\n> ");
        return;
    }

    char* args = 0;
    for (int i = 0; buffer[i]; i++) {
        if (buffer[i] == ' ') {
            buffer[i] = '\0';
            args = &buffer[i+1];
            break;
        }
    }

    command_node_t* curr = command_list;
    while (curr) {
        if (strcmp(curr->name, buffer) == 0) {
            vga_write("\n"); // Move to new line before command output
            curr->function(args);
            vga_write("\n> "); // Move to new line for the next prompt
            return;
        }
        curr = curr->next;
    }

    // If command not found
    vga_write("\nAliOS: '");
    vga_write(buffer);
    vga_write("' not found. Type 'help'.\n> ");
}
void shell_tab_complete(char* buffer, int* len) {
    command_node_t* curr = command_list;
    while (curr) {
        if (strncmp(curr->name, buffer, *len) == 0) {
            char* rest = curr->name + *len;
            vga_write(rest);
            while (*rest) buffer[(*len)++] = *rest++;
            buffer[*len] = '\0';
            return;
        }
        curr = curr->next;
    }
}
