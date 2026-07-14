#include "font.h"

// Inline assembly to pull the raw background image payload safely into memory
__asm__(
    ".global background_data\n"
    ".section .rodata\n"\
    ".align 4096\n"\
    "background_data:\n"
    ".incbin \"src/background.raw\"\n"
);
extern unsigned char background_data[];

// Crucial: Framebuffer address is an 8-byte pointer in 64-bit architectures
unsigned char* fb_addr = (unsigned char*)0xFD000000;
int cursor_x = 20; 
int cursor_y = 40; 
int shift_pressed = 0;

char current_username[32] =  "tux";
char command_buffer[64];
int command_length = 0;

// Upgraded x86_64 safe hardware I/O ports macros
static inline unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outw(unsigned short port, unsigned short data) {
    __asm__ volatile("outw %0, %1" : : "a"(data), "Nd"(port));
}

// Queries the CPU's hardware noise circuit to generate a true random integer
unsigned int hardware_rand() {
    unsigned int val;
    __asm__ volatile("rdrand %0" : "=r"(val));
    return val;
}

int mystrcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int mystrncmp(const char* s1, const char* s2, int n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

void draw_char(char c, int start_x, int start_y, unsigned int text_color) {
    if (c < 0) return;
    for (int row = 0; row < 16; row++) {
        unsigned char row_pixels = font_bitmap[(int)c][row];
        for (int col = 0; col < 8; col++) {
            if ((row_pixels & (0x80 >> col))) {
                int pixel_offset = ((start_y + row) * 800 + (start_x + col)) * 3;
                fb_addr[pixel_offset]     = (text_color >> 16) & 0xFF; 
                fb_addr[pixel_offset + 1] = (text_color >> 8)  & 0xFF; 
                fb_addr[pixel_offset + 2] = text_color         & 0xFF; 
            }
        }
    }
}

void erase_char_pixels(int start_x, int start_y) {
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 8; col++) {
            int pixel_offset = ((start_y + row) * 800 + (start_x + col)) * 3;
            fb_addr[pixel_offset]     = background_data[pixel_offset];
            fb_addr[pixel_offset + 1] = background_data[pixel_offset + 1];
            fb_addr[pixel_offset + 2] = background_data[pixel_offset + 2];
        }
    }
}

void graphics_print(const char* str, unsigned int color) {
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '\n') {
            cursor_x = 20;
            cursor_y += 20; 
            if (cursor_y >= 560) { 
                cursor_y = 40;
            }
        } else {
            draw_char(str[i], cursor_x, cursor_y, color);
            cursor_x += 10; 
            if (cursor_x >= 760) { 
                cursor_x = 20;
                cursor_y += 20;
                if (cursor_y >= 560) cursor_y = 40;
            }
        }
        i++;
    }
}

void print_prompt() {
    graphics_print(current_username, 0x00FF00); 
    graphics_print("@kernel:/# ", 0x00FF00);
}

void execute_clear() {
    for (int i = 0; i < 800 * 600 * 3; i++) {
        fb_addr[i] = background_data[i];
    }
    cursor_x = 20;
    cursor_y = 40;
}

void execute_neofetch() {
    graphics_print("---------------------------------\n", 0x00FFFF);
    graphics_print("OS:       Tux-OS v1.3.0 (64-Bit Mode Conversion)\n", 0xFFFFFF);
    graphics_print("Kernel:   Custom Freestanding x86_64 Long Mode Matrix\n", 0xFFFFFF);
    graphics_print("Shell:    Vedal-Shell-v1\n", 0xFFFFFF);
    graphics_print("Theme:    Neuro-Sama & Evil Boot Canvas\n", 0xFFFFFF);
    graphics_print("WM/DE:    Raw Framebuffer Grid (800x600)\n", 0xFFFFFF);
    graphics_print("Uptime:   Immortal (64-bit hardware pipelines active)\n", 0xFFFFFF);
    graphics_print("---------------------------------\n", 0x00FFFF);
}

void execute_help() {
    graphics_print("TUX-OS Core Shell Commands Layout:\n", 0x00FFFF);
    graphics_print("  help                 - Display this hardware command suite\n", 0xFFFFFF);
    graphics_print("  clear                - Blit-restore wallpaper pixels\n", 0xFFFFFF);
    graphics_print("  neofetch             - Output kernel specification summary\n", 0xFFFFFF);
    graphics_print("  vedal                - Execute spaghettified AI code streams\n", 0xFFFFFF);
    graphics_print("  shutdown             - Retract and terminate system session\n", 0xFFFFFF);
}

void execute_changename(const char* args) {
    int i = 0;
    while (args[i] == ' ') i++;
    int j = 0;
    while (args[i] != '\0' && j < 31) {
        current_username[j] = args[i];
        i++;
        j++;
    }
    current_username[j] = '\0';
    graphics_print("Username successfully migrated!\n", 0x00FFFF);
}

void hardware_delay(int ms) {
    for (int i = 0; i < ms * 10; i++) {
        __asm__ volatile("outb %%al, $0x43" : : "a"(0x00));
        unsigned char low = inb(0x40);
        unsigned char high = inb(0x40);
        unsigned short last_count = (high << 8) | low;
        while (1) {
            __asm__ volatile("outb %%al, $0x43" : : "a"(0x00));
            low = inb(0x40);
            high = inb(0x40);
            unsigned short current_count = (high << 8) | low;
            int diff = (int)last_count - (int)current_count;
            if (diff < 0) diff += 65536;
            if (diff >= 119) break;
        }
    }
}

void execute_vedal_spam() {
    graphics_print("Launching Vedal Python Refactoring Simulator...\n", 0x00FFFF);
    graphics_print("Press ANY KEY to stop the spaghettification!\n\n", 0xFFFF00);
    hardware_delay(1500);

    // In 64-bit architecture, these string literals are now handled cleanly as 64-bit array offsets
    const char* spaghetti_dictionary[12] = {
        "Refactoring Neuro memory structures... [SUB-OPTIMAL]\n",
        "Error: Turtle filter tripped. Auto-patching core blocks...\n",
        "for i in range(infinity): print(spaghetti_strings)\n",
        "Gym streams are canceled. Reason: Coding deadlock inside loops.\n",
        "Evil Neuro is bypassing memory pointer boundaries again.\n",
        "Merging un-reviewed firmware directly into active branch...\n",
        "while(true) { allocation_leak = malloc(9999); }\n",
        "Vedal: It worked perfectly on my machine local box frame.\n",
        "Fixing layout bugs by adding another nested conditional loop...\n",
        "Warning: Coffee reserves depleted. Execution speed dropping.\n",
        "Neuro-Sama: Wow Vedal, this code structure is actual garbage!\n",
        "Baking raw terminal color escape codes directly into production.\n"
    };

    if ((inb(0x64) & 1) == 1) inb(0x60);

    while (1) {
        hardware_delay(1000);

        if ((inb(0x64) & 1) == 1) {
            inb(0x60); 
            graphics_print("\nSpam sequence terminated by developer escape block.\n", 0x00FF00);
            break;
        }

        unsigned int random_index = hardware_rand() % 12;
        graphics_print(spaghetti_dictionary[random_index], 0xFFFFFF);
    }
}

void execute_snake_shutdown() {
    execute_clear();
    graphics_print("Initiating system retraction sequence...\n", 0xFF0000);
    hardware_delay(500);

    int top = 0; int bottom = 599; int left = 0; int right = 799;

    while (top <= bottom && left <= right) {
        for (int x = left; x <= right; x++) {
            int offset = (top * 800 + x) * 3;
            fb_addr[offset] = 0; fb_addr[offset+1] = 0; fb_addr[offset+2] = 0;
        }
        top++; hardware_delay(2); 

        for (int y = top; y <= bottom; y++) {
            int offset = (y * 800 + right) * 3;
            fb_addr[offset] = 0; fb_addr[offset+1] = 0; fb_addr[offset+2] = 0;
        }
        right--;

        if (top <= bottom) {
            for (int x = right; x >= left; x--) {
                int offset = (bottom * 800 + x) * 3;
                fb_addr[offset] = 0; fb_addr[offset+1] = 0; fb_addr[offset+2] = 0;
            }
            bottom--;
        }

        if (left <= right) {
            for (int y = bottom; y >= top; y--) {
                int offset = (y * 800 + left) * 3;
                fb_addr[offset] = 0; fb_addr[offset+1] = 0; fb_addr[offset+2] = 0;
            }
            left++;
        }
    }

    // ACPI/QEMU Shutdown execution sequence via IO port write
    outw(0x604, 0x2000); 
    while (1) { __asm__ volatile("hlt"); }
}

void process_shell_command() {
    command_buffer[command_length] = '\0'; 
    cursor_x = 20;
    cursor_y += 20;

    if (command_length == 0) {
        print_prompt();
        return;
    }

    if (mystrcmp(command_buffer, "clear") == 0) {
        execute_clear();
    } 
    else if (mystrcmp(command_buffer, "neofetch") == 0) {
        execute_neofetch();
    } 
    else if (mystrcmp(command_buffer, "help") == 0) {
        execute_help();
    }
    else if (mystrcmp(command_buffer, "vedal") == 0) {
        execute_vedal_spam();
    }
    else if (mystrcmp(command_buffer, "shutdown") == 0) {
        execute_snake_shutdown();
    }
    else if (mystrncmp(command_buffer, "changename ", 11) == 0) {
        execute_changename(&command_buffer[11]); 
    } 
    else {
        graphics_print("tuxshell: command not found: ", 0xFF0000);
        graphics_print(command_buffer, 0xFF0000);
        graphics_print("\n", 0xFF0000);
    }

    if (mystrcmp(command_buffer, "clear") != 0 && mystrcmp(command_buffer, "shutdown") != 0) {
        print_prompt();
    } else if (mystrcmp(command_buffer, "clear") == 0) {
        print_prompt();
    }
    
    command_length = 0;
}

unsigned char scancode_to_ascii_lower[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

unsigned char scancode_to_ascii_upper[] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, 0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};

void read_keyboard_input() {
    unsigned char last_scancode = 0;
    while (1) {
        if ((inb(0x64) & 1) == 1) {
            unsigned char scancode = inb(0x60);
            if (scancode != last_scancode) {
                if (scancode == 0x2A || scancode == 0x36) {
                    shift_pressed = 1;
                }
                else if (scancode == 0xAA || scancode == 0xB6) {
                    shift_pressed = 0;
                }
                else if ((scancode & 0x80) == 0) {
                    if (scancode == 0x0E) {
                        if (command_length > 0) {
                            command_length--;
                            cursor_x -= 10;
                            erase_char_pixels(cursor_x, cursor_y);
                        }
                    } else {
                        char ascii = shift_pressed ? scancode_to_ascii_upper[scancode] : scancode_to_ascii_lower[scancode];
                        if (ascii != 0) {
                            if (ascii == '\n') {
                                process_shell_command();
                            } else {
                                if (command_length < 63) {
                                    command_buffer[command_length++] = ascii;
                                    char str[2] = {ascii, '\0'};
                                    graphics_print(str, 0xFF0000);
                                }
                            }
                        }
                    }
                }
                last_scancode = scancode;
            }
        }
    }
}

// Fixed parameter: multiboot_info arrives via the 64-bit register as an unsigned long pointer
void kernel_main(unsigned long multiboot_info_addr) {
    unsigned int* multiboot_info = (unsigned int*)multiboot_info_addr;
    unsigned int total_size = *multiboot_info;
    unsigned int offset = 8;
    
    while (offset < total_size) {
        unsigned int* tag = (unsigned int*)((unsigned char*)multiboot_info + offset);
        if (*tag == 8) {
            // Tag type 8 is the Framebuffer Info Tag.
            // Grabs the 64-bit structural memory offset address accurately:
            unsigned long long* fb_addr_ptr = (unsigned long long*)((unsigned char*)tag + 8);
            fb_addr = (unsigned char*)((unsigned long)(*fb_addr_ptr));
            break;
        }
        offset = (offset + *(tag + 1) + 7) & ~7;
    }

    // Refresh entire canvas using background wallpaper matrix bytes
    for (int i = 0; i < 800 * 600 * 3; i++) {
        fb_addr[i] = background_data[i];
    }

    graphics_print("TUX-OS Graphical Command Engine Terminal Active (64-Bit Mode Loaded)\n", 0x00FFFF);
    print_prompt();

    read_keyboard_input();
}

