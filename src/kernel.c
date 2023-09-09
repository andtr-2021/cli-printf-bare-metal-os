#include "mbox.h"
#include "uart.h"
#include "printf.h"
#include "../gcclib/stdbool.h"
#include "../gcclib/stddef.h"
#include "../gcclib/stdarg.h"
#define MAX_CMD_SIZE 100
#define MAX_HISTORY_SIZE 10 

const char* available_commands[] = {"help", "clear", "showinfo", "setcolor"};


static char history[MAX_HISTORY_SIZE][MAX_CMD_SIZE];
static int history_index = 0;
static int history_count = 0;

#define ADDR(x) (unsigned int)&(x)

// Definitions for TAG identifiers and mailbox channel
#define MBOX_TAG_SETPHYWH 0x48003
#define MBOX_CH_PROP 8




void welcome()
{
    uart_puts("\n");
    uart_puts("\n");
   
    uart_puts("oooooooooo      o      oooooooooo  ooooooooooo \n");
    uart_puts(" 888    888    888      888    888  888    88  \n");
    uart_puts(" 888oooo88    8  88     888oooo88   888ooo8    \n");
    uart_puts(" 888    888  8oooo88    888  88o    888    oo  \n");
    uart_puts("o888ooo888 o88o  o888o o888o  88o8 o888ooo8888 \n");
    uart_puts("-----------------------------------------------\n");
    uart_puts("oooo     oooo ooooooooooo ooooooooooo   o      ooooo          ooooooo    oooooooo8            \n");
    uart_puts(" 8888o   888   888    88  88  888  88  888      888         o888   888o 888                   \n");
    uart_puts(" 88 888o8 88   888ooo8        888     8  88     888         888     888  888oooooo            \n");
    uart_puts(" 88  888  88   888    oo      888    8oooo88    888      o  888o   o888         888           \n");
    uart_puts("o88o  8  o88o o888ooo8888    o888o o88o  o888o o888ooooo88    88ooo88   o88oooo888            \n");
    uart_puts("-----------------------------------------------\n");
    uart_puts("Developed by: <Do Truong An> ------- <s3878698>\n");
    uart_puts("\n");
    uart_puts("\n");
}

// CUSTOM FUNCTIONS 
int custom_strcmp(const char* s1, const char* s2) {
    while (*s1 != '\0' && *s2 != '\0' && *s1 == *s2) {
        s1++;
        s2++;
    }
    return (*s1 - *s2);
}

int custom_strncmp(const char* str1, const char* str2, size_t n) {
    while (n > 0 && *str1 && (*str1 == *str2)) {
        str1++;
        str2++;
        n--;
    }
    if (n == 0) return 0;
    return (unsigned char)*str1 - (unsigned char)*str2;
}

// Function to calculate the length of a string
int custom_strlen(const char* str) {
    int length = 0;
    while (str[length] != '\0') {
        length++;
    }
    return length;
}

// Function to check if a string starts with a specific prefix
int custom_startsWith(const char* str, const char* prefix) {
    while (*prefix != '\0') {
        if (*prefix != *str) {
            return 0; // False, not matching
        }
        prefix++;
        str++;
    }
    return 1; // True, matching
}

const char* custom_strstr(const char* haystack, const char* needle) {
    while (*haystack != '\0') {
        const char* h = haystack;
        const char* n = needle;

        while (*n != '\0' && *h == *n) {
            h++;
            n++;
        }

        if (*n == '\0') {
            return haystack; // Substring found
        }

        haystack++;
    }

    return NULL; // Substring not found
}


void clear_screen() {
    uart_puts("\033[H\033[J");
}

void help() {
  uart_puts("Available commands:\n");
  uart_puts("  help: this command prints the list of available commands\n");
  uart_puts("  help <command>: this command prints the help message of the given command\n");
  uart_puts("  clear: this command clears the screen\n");
  uart_puts("  setcolor -t <text color>: this command sets the text color\n");
  uart_puts("  setcolor -b <background color>: this command sets the background color\n");
  uart_puts("  showinfo: this command prints the information of the system\n");
  uart_puts("\n");
}

bool is_command(const char* command, const char* check) {
    while (*check != '\0') {
        if (*command != *check) {
            return false;
        }
        command++;
        check++;
    }
    return (*command == '\0');
}

bool is_help_command_with_arg(const char* input, const char* command, const char* arg) {
    while (*input != '\0' && *command != '\0') {
        if (*input != *command) {
            return false;
        }
        input++;
        command++;
    }
    return (*command == '\0' && *input == ' ' && is_command(input + 1, arg));
}

void custom_strncpy(char* destination, const char* source, size_t n) {
    size_t i = 0;
    while (i < n && source[i] != '\0') {
        destination[i] = source[i];
        i++;
    }
    while (i < n) {
        destination[i] = '\0';
        i++;
    }
}

void macAddressFormatter(unsigned int num1, unsigned int num2) {
   // convert from this hex format to 0x000057340x12005452 this format: 00-00-57-34-40-12-00-54-52. use sendc to print out each digit and use - to separate each digit
   
   // process num 2
    for (int pos = 28; pos >= 0; pos = pos - 4) {
        // Get highest 4-bit nibble
        char digit = (num2 >> pos) & 0xF;
        /* Convert to ASCII code */
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        digit += (digit > 9) ? (-10 + 'A') : '0';
        uart_sendc(digit);

        if (pos % 8 == 0 && pos != 0) {
            uart_sendc('-');
        }
    }

    uart_sendc('-');
   
   // process num 1
    for (int pos = 28; pos >= 0; pos = pos - 4) {
        // Get highest 4-bit nibble
        char digit = (num1 >> pos) & 0xF;
        /* Convert to ASCII code */
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        digit += (digit > 9) ? (-10 + 'A') : '0';
        uart_sendc(digit);

        if (pos % 8 == 0 && pos != 0) {
            uart_sendc('-');
        }
    }    
}


void showinfo() {

    // get board divion 
    mBuf[0] = 7 * 4;        // Message Buffer Size in bytes (8 elements * 4 bytes (32 bit) each)
    mBuf[1] = MBOX_REQUEST; // Message Request Code (this is a request message)
    mBuf[2] = 0x00010002;   // TAG Identifier: Get Board Revision
    mBuf[3] = 4;            // Value buffer size in bytes (max of request and response lengths)
    mBuf[4] = 0;            // REQUEST CODE = 0
    mBuf[5] = 0;            // clear output buffer (response data are mBuf[5] & mBuf[6])
    mBuf[6] = MBOX_TAG_LAST;

    if (mbox_call(ADDR(mBuf), MBOX_CH_PROP)) {
        uart_puts("Board Revision: ");
        uart_hex(mBuf[5]);
        uart_puts("\n");
    } else {
        uart_puts("Unable to query!\n");
    }

    // get board MAC address
    mBuf[0] = 8 * 4;        // Message Buffer Size in bytes (8 elements * 4 bytes (32 bit) each)
    mBuf[1] = MBOX_REQUEST; // Message Request Code (this is a request message)
    mBuf[2] = 0x00010003;   // TAG Identifier: Get Board MAC Address
    mBuf[3] = 6;            // Value buffer size in bytes (max of request and response lengths)
    mBuf[4] = 0;            // REQUEST CODE = 0
    mBuf[5] = 0;
    mBuf[6] = 0;            // clear output buffer (response data are mBuf[5] & mBuf[6])
    mBuf[7] = MBOX_TAG_LAST;

    if (mbox_call(ADDR(mBuf), MBOX_CH_PROP)) {
        uart_puts("Board MAC Address: ");
        uart_hex(mBuf[6]);
        uart_hex(mBuf[5]);
        uart_puts("\n");
        uart_puts("Correct Format of MAC Address: ");
        macAddressFormatter(mBuf[5], mBuf[6]);
        uart_puts("\n");
    } else {
        uart_puts("Unable to query!\n");
    }

}
   


int setTextColor(const char* command) {
    const char* flag = "-t ";
    const int flagLen = custom_strlen(flag);

    const char* colorStart = NULL;
    for (int i = 0; command[i] != '\0'; i++) {
        if (custom_startsWith(command + i, flag)) {
            colorStart = command + i + flagLen;
            break;
        }
    }

    if (colorStart != NULL) {
        const char* colorName = colorStart;

        if (custom_strcmp(colorName, "black") == 0) {
            uart_puts("\033[30m");
        } else if (custom_strcmp(colorName, "red") == 0) {
            uart_puts("\033[31m");
        } else if (custom_strcmp(colorName, "green") == 0) {
            uart_puts("\033[32m");
        } else if (custom_strcmp(colorName, "yellow") == 0) {
            uart_puts("\033[33m");
        } else if (custom_strcmp(colorName, "blue") == 0) {
            uart_puts("\033[34m");
        } else if (custom_strcmp(colorName, "purple") == 0) {
            uart_puts("\033[35m");
        } else if (custom_strcmp(colorName, "cyan") == 0) {
            uart_puts("\033[36m");
        } else if (custom_strcmp(colorName, "white") == 0) {
            uart_puts("\033[37m");
        } else {
            uart_puts("Invalid text color. Available colors: black, red, green, yellow, blue, purple, cyan, white\n");
            return 0;
        }

        uart_puts("Text color set.\n");
        return 1;
    } 

    return 0;
}

int setBackgroundColor(const char* command) {
    const char* flag = "-b ";
    const int flagLen = custom_strlen(flag);

    const char* colorStart = NULL;
    for (int i = 0; command[i] != '\0'; i++) {
        if (custom_startsWith(command + i, flag)) {
            colorStart = command + i + flagLen;
            break;
        }
    }

    if (colorStart != NULL) {
        const char* colorName = colorStart;

        if (custom_strcmp(colorName, "black") == 0) {
            uart_puts("\033[40m");
        } else if (custom_strcmp(colorName, "red") == 0) {
            uart_puts("\033[41m");
        } else if (custom_strcmp(colorName, "green") == 0) {
            uart_puts("\033[42m");
        } else if (custom_strcmp(colorName, "yellow") == 0) {
            uart_puts("\033[43m");
        } else if (custom_strcmp(colorName, "blue") == 0) {
            uart_puts("\033[44m");
        } else if (custom_strcmp(colorName, "purple") == 0) {
            uart_puts("\033[45m");
        } else if (custom_strcmp(colorName, "cyan") == 0) {
            uart_puts("\033[46m");
        } else if (custom_strcmp(colorName, "white") == 0) {
            uart_puts("\033[47m");
        } else {
            uart_puts("Invalid background color. Available colors: black, red, green, yellow, blue, purple, cyan, white\n");
            return 0;
        }

        uart_puts("Background color set.\n");
        return 2; 
    }

    return 0;
}

bool hasBothFlags(const char* command) {
    const char* textFlag = "-t ";
    const char* bgColorFlag = "-b ";

    return (custom_strstr(command, textFlag) != NULL && custom_strstr(command, bgColorFlag) != NULL);
}

void setColorsForText(const char* command) {
    const char* textFlag = "-t ";
    const char* bgColorFlag = "-b ";

    const int textFlagLen = custom_strlen(textFlag);

    const char* textStart = custom_strstr(command, textFlag);
    const char* bgColorStart = custom_strstr(command, bgColorFlag);

    if (textStart != NULL && bgColorStart != NULL && textStart < bgColorStart) {
        
        const char* textName = textStart + textFlagLen;

        if (custom_strcmp(textName, "yellow")){
            uart_puts("\033[33m");
            uart_puts("Yellow. ");
        } else if (custom_strcmp(textName, "blue")){
            uart_puts("\033[34m");
            uart_puts("Blue. ");
        }

        uart_puts("Text color set.\n");
    } 
}

void setColorsForBackground(const char* command) {
    const char* textFlag = "-t ";
    const char* bgColorFlag = "-b ";

    const int bgColorFlagLen = custom_strlen(bgColorFlag);

    const char* textStart = custom_strstr(command, textFlag);
    const char* bgColorStart = custom_strstr(command, bgColorFlag);

    if (textStart != NULL && bgColorStart != NULL && textStart < bgColorStart) {
        
        const char* bgColorName = bgColorStart + bgColorFlagLen;

        if (custom_strcmp(bgColorName, "blue") == 0){
            uart_puts("\033[44m");
            uart_puts("Blue. ");
        } else if (custom_strcmp(bgColorName, "yellow") == 0){
            uart_puts("\033[43m");
            uart_puts("Yellow. ");
        }

        uart_puts("Background color set.\n");
    } 
}


void handle_command(const char* command) {

    if (hasBothFlags(command)) {
        setColorsForText(command);
        setColorsForBackground(command);
        return ;
    }

    int textColor = setTextColor(command);
    int bgColor = setBackgroundColor(command);

    if (textColor == 1 || bgColor == 2) {
       return ;
    }
    
    if (is_help_command_with_arg(command, "help", "help")) {
        uart_puts("Definition: help - Print the list of available commands\n");
        uart_puts("Usage: Prints the list of available commands.\n");

    } else if (is_help_command_with_arg(command, "help", "clear")) {
        uart_puts("Definition: clear - Clear the screen\n");
        uart_puts("Usage: Clears the screen.\n");

    } else if (is_help_command_with_arg(command, "help", "setcolor")) {
        uart_puts("Definition: setcolor - Set text and/or background color\n");
        uart_puts("Usage: Sets the text color and/or background color of the console.\n");

    } else if (is_help_command_with_arg(command, "help", "showinfo")) {
        uart_puts("Definition: showinfo - Show board revision and board MAC address\n");
        uart_puts("Usage: Displays the board revision and MAC address.\n");
    } else if (is_command(command, "help")) {
        help();
    } else if (is_command(command, "clear")) {
        clear_screen();
    } else if (is_command(command, "showinfo")) {
        showinfo();
    } else {
        uart_puts("Unknown command. Type 'help' to see the list of available commands.\n");
    }
}

void add_to_history(const char* command) {
    if (history_count < MAX_HISTORY_SIZE) {
        custom_strncpy(history[history_count], command, MAX_CMD_SIZE);
        history_count++;
    } else {
        for (int i = 0; i < MAX_HISTORY_SIZE - 1; i++) {
            custom_strncpy(history[i], history[i + 1], MAX_CMD_SIZE);
        }
        custom_strncpy(history[MAX_HISTORY_SIZE - 1], command, MAX_CMD_SIZE);
    }
}

void print_history() {
    for (int i = 0; i < history_count; i++) {
        uart_puts(history[i]);
        uart_puts("\n");
    }
}

void clear_line(int start_index) {
    // Move the cursor to the starting position
    for (int i = 0; i < start_index; i++) {
        uart_sendc('\b');
    }

    // Clear characters on the line
    for (int i = start_index; i < MAX_CMD_SIZE; i++) {
        uart_sendc(' ');
    }

    // Move the cursor back to the starting position
    for (int i = 0; i < MAX_CMD_SIZE; i++) {
        uart_sendc('\b');
    }
}

void autocomplete(const char* partial_command, int* index, char* cli_buffer, int buffer_size) {
    int matches = 0;
    int match_index = -1;

    for (int i = 0; i < sizeof(available_commands) / sizeof(available_commands[0]); i++) {
        if (custom_startsWith(available_commands[i], partial_command)) {
            matches++;
            match_index = i;
        }
    }

    if (matches == 1) {
        clear_line(MAX_CMD_SIZE + 7);
        uart_puts("MyOS> ");
        uart_puts(available_commands[match_index]);
        custom_strncpy(cli_buffer, available_commands[match_index], buffer_size);
        *index = custom_strlen(cli_buffer); // Update the index using pointer
    } else if (matches > 1) {
        clear_line(MAX_CMD_SIZE + 7);
        uart_puts("MyOS> ");
        uart_puts(partial_command);
        uart_puts("\n");
        uart_puts("Available commands:");
        
        for (int i = 0; i < sizeof(available_commands) / sizeof(available_commands[0]); i++) {
            if (custom_startsWith(available_commands[i], partial_command)) {
                uart_puts("\n");
                uart_puts(available_commands[i]);
            }
        }
        
       uart_puts("\n");
       uart_puts("Please press enter and type more characters to autocomplete nexttime.\n");
    }
}


void cli() {
    uart_puts("MyOS> ");
    
    static char cli_buffer[MAX_CMD_SIZE];
    static int index = 0;
    
    while (true) {
        char c = uart_getc();
        uart_sendc(c);

        if (c == '\b') {
            if (index > 0) {
                uart_sendc(' ');
                uart_sendc('\b');
                index--;
            }
        } else if (c == '+' && history_index < history_count - 1) {
            history_index++;
            custom_strncpy(cli_buffer, history[history_index], MAX_CMD_SIZE);
            index = custom_strlen(cli_buffer);
            // Clear the current command line
            clear_line(index);
            // Print the retrieved command line
            clear_line(MAX_CMD_SIZE + 7); // Clear from the beginning of the command line
            // Print the prompt
            uart_puts("MyOS> ");
            uart_puts(cli_buffer);
        } else if (c == '_' && history_index > 0) {
            history_index--;
            custom_strncpy(cli_buffer, history[history_index], MAX_CMD_SIZE);
            index = custom_strlen(cli_buffer);
            clear_line(index);
            // Clear the current command line
            clear_line(MAX_CMD_SIZE + 10); 
            uart_puts("MyOS> ");
            // Print the retrieved command line
            uart_puts(cli_buffer);
        } else if (c == '\t') { // Handle tab for autocompletion
            cli_buffer[index] = '\0'; // Null-terminate the command buffer
            autocomplete(cli_buffer, &index, cli_buffer, MAX_CMD_SIZE);
            index = custom_strlen(cli_buffer);
        } else if (c != '\n') {
            cli_buffer[index] = c;
            index++;
        } else {
            cli_buffer[index] = '\0';
            
            add_to_history(cli_buffer); // Store command in history
            history_index = history_count - 1; // Set history index to the most recent command

            handle_command(cli_buffer);

            uart_puts("MyOS> ");
            index = 0;
        }
    }
}

void printf_demo() {

    uart_puts("\n");
    uart_puts("This is a demo of printf function: \n");
    printf("- Hexadecimal: %x\n", 0x143abc);
    printf("- Decimals: %d\n", 1977);
    printf("- Character: %c\n", 'A');
    printf("- String: %s\n", "This is a string");
    printf("- Float: %f\n", 3.141);
    printf("- Percentage: %%\n");
    printf("- 0 Flag: %05d\n", 1977);
    printf("- Precision: %.2f\n", 3.14159265359);

    uart_puts("\n");
}

void mailbox_demo() {

    unsigned int *physize = 0; //pointer to response data  

    unsigned int mBox[100]; 
    uart_puts("\nMailbox Demo: \n");

    mbox_buffer_setup(ADDR(mBox), MBOX_TAG_SETPHYWH, &physize, 8, 8, 2, 1024, 768);
    mbox_call(ADDR(mBuf), MBOX_CH_PROP); 
    uart_puts("\nGot Actual Physical Width: "); 
    uart_dec(physize[0]); 
    uart_puts("\nGot Actual Physical Height: "); 
    uart_dec(physize[1]); 

}

void main()
{  
    // set up serial console
    uart_init();

    printf_demo();

    mailbox_demo();

    // print welcom msg 
    welcome();

    // echo everything back
    while(1) {
        // cli function
        cli();
    }
}