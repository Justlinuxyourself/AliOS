(/* 
Copyright (c) 2026 Ali  
All rights reserved.
*/
#include "shell.h"
#include "../section1_cpu/heap.h"
#include "../section1_cpu/io.h"
#include "aliscr.h"
#define NOTEBOOK_YELLOW 0x1E
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
extern void vga_set_color(unsigned char color);

typedef struct {
    char key[16];
    char value[32];
    int active;
} env_var_t;

env_var_t env_table[10]; // Store up to 10 variables in RAM
// some structs are down with the code that uses it bc i didnt plan for it, it just popped ip in my head
typedef struct {
    char task[48];
    int done;
    int active;
} todo_t;

todo_t my_list[10]; // 10 slots for your daily goals

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
	vga_write("Hi my name is ali, my age is 13, and i like planes :3");
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
    vga_write("MIKAY (BATTERY EATER TWINIES)\n");
    vga_write("ALIYAH\n");
    vga_write("AYAH\n");
    vga_write("DANIEL\n");
    vga_write("ASEEL (MY SIS)\n");
    vga_write("KHAILD\n");
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
typedef struct {
    int surah;
    int ayah;
    const char* text;
} ayah_t;

void cmd_ayah() {
    // Array of Ayahs stored in the Kernel Data Segment
    ayah_t quran_db[] = {
        {94, 5, "For indeed, with hardship [will be] ease."},
        {2, 152, "So remember Me; I will remember you."},
        {3, 139, "So do not weaken and do not grieve."},
        {2, 286, "Allah does not charge a soul except with that within its capacity."},
        {50, 16, "And We are closer to him than [his] jugular vein."}
    };

    // Calculate total entries in the database
    int db_size = sizeof(quran_db) / sizeof(ayah_t);

    // Use CMOS seconds to pick a random index
    int r = cmos_get_sec() % db_size;

    // Buffers for itoa conversion
    char s_str[8], a_str[8];

    vga_write("\n");
    // Print Format -> (SurahNum):(AyahNum) (Text)
    vga_write(itoa(quran_db[r].surah, s_str));
    vga_write(":");
    vga_write(itoa(quran_db[r].ayah, a_str));
    vga_write(" ");
    vga_write(quran_db[r].text);
    vga_write("\n");
}
typedef struct {
    const char* book;
    int chapter;
    int verse;
    const char* text;
} bible_t;

void cmd_verse() {
    bible_t bible_db[] = {
        {"Psalms", 23, 1, "The Lord is my shepherd; I shall not want."},
        {"John", 1, 5, "The light shines in the darkness, and the darkness has not overcome it."},
        {"Philippians", 4, 13, "I can do all things through Christ who strengthens me."},
        {"Matthew", 5, 9, "Blessed are the peacemakers, for they shall be called sons of God."},
        {"Proverbs", 3, 5, "Trust in the Lord with all your heart and lean not on your own understanding."}
    };

    int db_size = sizeof(bible_db) / sizeof(bible_t);
    int r = cmos_get_sec() % db_size;

    char c_str[8], v_str[8];

    vga_write("\n");
    // Format: Book Chapter:Verse - Text
    vga_write(bible_db[r].book);
    vga_write(" ");
    vga_write(itoa(bible_db[r].chapter, c_str));
    vga_write(":");
    vga_write(itoa(bible_db[r].verse, v_str));
    vga_write(" - ");
    vga_write(bible_db[r].text);
    vga_write("\n");
}
void cmd_set(char* args) {
    if (args == 0 || *args == '\0') {
        vga_write("Usage: set [key] [value]\n");
        return;
    }

    char* key = args;
    char* val = 0;
    
    for (int i = 0; args[i]; i++) {
        if (args[i] == ' ') {
            args[i] = '\0'; 
            val = &args[i+1]; 
            break;
        }
    }

    if (!val || *val == '\0') {
        vga_write("Error: Missing value for variable.\n");
        return;
    }

    // Save to the table
    for(int i = 0; i < 10; i++) {
        // Look for empty slot or overwrite existing key
        if(!env_table[i].active || strcmp(env_table[i].key, key) == 0) {
            strcpy(env_table[i].key, key);
            strcpy(env_table[i].value, val);
            env_table[i].active = 1;
            vga_write("Variable set.");
            return;
        }
    }
    vga_write("Error: Environment table full!");
}

void cmd_get(char* key) {
    // 1. SAFETY CHECK: If user just types 'get' with no name
    if (key == 0 || key[0] == '\0') {
        vga_write("Usage: get [key]\n");
        return;
    }

    for(int i = 0; i < 10; i++) {
        if(env_table[i].active) {
            // 2. Double check the stored key exists before comparing
            if (env_table[i].key != 0 && strcmp(env_table[i].key, key) == 0) {
                vga_write(env_table[i].value);
                vga_write("\n");
                return;
            }
        }
    }
    vga_write("Error: Variable not found.\n");
}

void todo_add(char* text) {
    for(int i = 0; i < 10; i++) {
        if(!my_list[i].active) {
            strcpy(my_list[i].task, text);
            my_list[i].done = 0;
            my_list[i].active = 1;
            vga_write("Task added to AliOS list.\n");
            return;
        }
    }
    vga_write("Error: Your brain (list) is full!\n");
}
void todo_show() {
    int found = 0;
    for(int i = 0; i < 10; i++) {
        // Only print if the slot is explicitly marked active
        if(my_list[i].active == 1 && my_list[i].task[0] != '\0') {
            vga_write("- ");
            vga_write(my_list[i].task);
            vga_write("\n");
            found = 1;
        }
    }
    if(!found) vga_write("No tasks found.\n");
}

void draw_menu_item(int id, int selected, const char* text) {
    if (id == selected) {
        // Highlighting logic
        vga_write(" > ");               // Arrow pointer
        vga_set_color(0x70);            // Invert: Black text on Light Gray background
        vga_write(text);
        vga_set_color(NOTEBOOK_YELLOW); // Reset to standard AliOS Yellow/Blue
    } else {
        vga_write("   ");               // Spacer for non-selected items
        vga_write(text);
    }
    vga_write("\n");
}
void cmd_menu(char* args) {
    int selected = 1;
    int total_options = 7; 
    int running = 1;

    vga_clear();

    while (running) {
        // Jump back to the top-left to overwrite, not scroll
        vga_set_cursor(0, 0); 

        // Header Section
        vga_set_color(0x1F); // White on Blue (Status Bar Style)
        vga_write("========================================\n");
        vga_write("          AliOS 4.0 - TOOLBOX           \n");
        vga_write("      (Use Arrows to Move, Enter)       \n");
        vga_write("========================================\n\n");
        vga_set_color(NOTEBOOK_YELLOW);

        // Draw all buttons
        draw_menu_item(1, selected, "[ 1. SYSTEM INFO (NEOFETCH) ]");
        draw_menu_item(2, selected, "[ 2. TO-DO LIST (TDSHW)     ]");
        draw_menu_item(3, selected, "[ 3. CALCULATOR (CALC)      ]");
        draw_menu_item(4, selected, "[ 4. QURAN AYAH             ]");
        draw_menu_item(5, selected, "[ 5. DRAW PLANE ART         ]");
        draw_menu_item(6, selected, "[ 6. LOCK SYSTEM            ]");
        draw_menu_item(7, selected, "[ 7. EXIT MENU              ]");
        
        vga_write("\n========================================\n");

        // Polling loop for Keyboard Input
        while (!(inb(0x64) & 0x01)) {
            vga_draw_status_bar(); // Keeps the clock ticking!
        }

        unsigned char scancode = inb(0x60);

        // Arrow and Input Logic
        if (scancode == 0x48) {        // UP ARROW
            if (selected > 1) selected--;
        } 
        else if (scancode == 0x50) {   // DOWN ARROW
            if (selected < total_options) selected++;
        } 
        else if (scancode == 0x1C) {   // ENTER KEY
            vga_clear(); 
            
            // Execute the selected tool
            if (selected == 1) cmd_neofetch(0); sleep_ms(1000); vga_clear();
            if (selected == 2) todo_show(); sleep_ms(1000); vga_clear();
            if (selected == 3) vga_write("Use it in terminal\n"); sleep_ms(1000); vga_clear();
            if (selected == 4) cmd_ayah(); sleep_ms(1000); vga_clear();
            if (selected == 5) draw_custom_plane(); sleep_ms(1000); vga_clear();
            if (selected == 6) shell_lock(); sleep_ms(1000); vga_clear();
            if (selected == 7) { running = 0; }

            // If we didn't exit, wait for a key before returning to menu
            if (running && selected != 7) {
                // deleted bc of sleep and this shit doesnt work
                while (!(inb(0x64) & 0x01)); 
                inb(0x60); // Flush the buffer
            }
        }
        else if (scancode == 0x01) {   // ESC KEY
            running = 0;
        }
    }

    vga_clear();
    vga_write("Returned to AliOS Shell.\n> ");
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
    shell_register_command("ayah", "Choose Random Quran Ayah and Print it (im turning into terry davis)", cmd_ayah);
    shell_register_command("verse", "Choose Random Bible Verse and Print it", cmd_verse);
    shell_register_command("set", "Set VAR", cmd_set);
    shell_register_command("get", "Get VAR", cmd_get);
    shell_register_command("tdadd", "Add to ToDo List", todo_add);
    shell_register_command("tdshw", "Show ToDo List", todo_show);
    shell_register_command("menu", "AliOS Menu", cmd_menu);
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
