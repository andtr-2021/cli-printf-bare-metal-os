#include "printf.h"
#include "uart.h"

#define MAX_PRINT_SIZE 256
int precision = 6;

// This function is called by printf to print out a string
void printf(char *string,...) {

	//Initialize the variable argument list
	va_list ap;
	va_start(ap, string);

	//Create a buffer to store the formated string
	char buffer[MAX_PRINT_SIZE];

	for (int i = 0; i < MAX_PRINT_SIZE; i++) {
		buffer[i] = 0;
	}

	int buffer_index = 0;

	//Create a temp buffer to store the integer value
	char temp_buffer[MAX_PRINT_SIZE];

	//Parse the string
	while(1) {
		// Check if we reach the end of the string
		if (*string == 0)
			break;

		// Check if we have a format specifier
		if (*string == '%') {
			string++;

			// check for precision
			int precisionF = 0;
			int isPrecision = 0;
			// Parse precision
			if (*string == '.') {
				string++;
				precisionF = 0;
				isPrecision = 1;
				while (*string >= '0' && *string <= '9') {
					precisionF = precisionF * 10 + (*string - '0');
					string++;
				}
			}
			
            int zero_padding = 0;
            if (*string == '0') {
                zero_padding = 1;
                string++;
            }

            // Parse width
            int width = 0;
            while (*string >= '0' && *string <= '9') {
                width = width * 10 + (*string - '0');
                string++;
			}

			if (*string == 'd') {
				string++;
				int x = va_arg(ap, int);
				int temp_index = MAX_PRINT_SIZE - 1;

				do {
					temp_buffer[temp_index] = (x % 10) + '0';
					temp_index--;
					x /= 10;
				} while(x != 0);

				// Zero padding
				if (zero_padding) {
					while (temp_index >= MAX_PRINT_SIZE - width) {
						temp_buffer[temp_index] = '0';
						temp_index--;
					}
				}

				for(int i = temp_index + 1; i < MAX_PRINT_SIZE; i++) {
					buffer[buffer_index] = temp_buffer[i];
					buffer_index++;
				}
			}
			else if (*string == 'x') {

				string++;
				unsigned int x = va_arg(ap, unsigned int);
				int temp_index = MAX_PRINT_SIZE - 1;

				// Special case: handle x = 0
				if (x == 0) {
					temp_buffer[temp_index] = '0';
					temp_index--;
				} else {
					while (x != 0) {
						int digit = x % 16;
						temp_buffer[temp_index] = (digit < 10) ? (digit + '0') : (digit - 10 + 'a');
						temp_index--;
						x /= 16;
					}
				}

				// Copy the hexadecimal prefix "0x" to the main buffer
				buffer[buffer_index] = '0';
				buffer_index++;
				buffer[buffer_index] = 'x';
				buffer_index++;

				// Copy the hexadecimal digits from temp_buffer to the main buffer
				for (int i = temp_index + 1; i < MAX_PRINT_SIZE; i++) {
					buffer[buffer_index] = temp_buffer[i];
					buffer_index++;
				}
				
			} else if (*string == 'c') {
				string++;
				char c = (char)va_arg(ap, int);
				buffer[buffer_index] = c;
				buffer_index++;
			}
			else if (*string == 's') {
				string++;
				char *str = va_arg(ap, char*);
				while (*str != '\0' && buffer_index < MAX_PRINT_SIZE - 1) {
					buffer[buffer_index] = *str;
					buffer_index++;
					str++;
				}
			} 
			else if (*string == '%') {
				string++;
				buffer[buffer_index] = '%';
				buffer_index++;
			} else if (*string == 'f') {
				
				// with . and width
				// printf("%.2f", float); 
				if (isPrecision == 1) {
					
					string++;
					double f = va_arg(ap, double);
					int temp_index = MAX_PRINT_SIZE - 1;

					// Handle negative numbers
					if (f < 0) {
						buffer[buffer_index] = '-';
						buffer_index++;
						f = -f;
					}

					// Calculate the integer and fractional parts
					long long int_part = (long long)f;
					double fractional_part = f - (double)int_part;

					// Convert the integer part
					do {
						temp_buffer[temp_index] = (int_part % 10) + '0';
						temp_index--;
						int_part /= 10;
					} while (int_part != 0);

					for (int i = temp_index + 1; i < MAX_PRINT_SIZE; i++) {
						buffer[buffer_index] = temp_buffer[i];
						buffer_index++;
					}

					// Decimal point
					buffer[buffer_index] = '.';
					buffer_index++;

					// Convert the fractional part with specified precision
					for (int i = 0; i < precisionF; i++) {
						fractional_part *= 10;
						int digit = (int)fractional_part;
						buffer[buffer_index] = digit + '0';

						buffer_index++;
						fractional_part -= digit;
					}

					// Null-terminate the buffer
					buffer[buffer_index] = '\0';
					
				}
				
				// without . and width 
				// printf("- Float: %f\n", 3.141);
				if (isPrecision == 0) {

					string++;
					double f = va_arg(ap, double);
					int temp_index = MAX_PRINT_SIZE - 1;

					// Handle negative numbers
					if (f < 0) {
						buffer[buffer_index] = '-';
						buffer_index++;
						f = -f;
					}

					// Calculate the integer and fractional parts
					long long int_part = (long long)f;
					double fractional_part = f - (double)int_part;

					// Convert the integer part
					do {
						temp_buffer[temp_index] = (int_part % 10) + '0';
						temp_index--;
						int_part /= 10;
					} while (int_part != 0);

					for (int i = temp_index + 1; i < MAX_PRINT_SIZE; i++) {
						buffer[buffer_index] = temp_buffer[i];
						buffer_index++;
					}

					// Decimal point
					buffer[buffer_index] = '.';
					buffer_index++;

					// Convert the fractional part with specified precision
					for (int i = 0; i < precision; i++) {
						fractional_part *= 10;
						int digit = (int)fractional_part;
						buffer[buffer_index] = digit + '0';

						buffer_index++;
						fractional_part -= digit;
					}

					// Null-terminate the buffer
					buffer[buffer_index] = '\0'; 

				}
			}

			// done
		} else {
			buffer[buffer_index] = *string;
			buffer_index++;
			string++;
		}

		if (buffer_index == MAX_PRINT_SIZE - 1)
			break;
	}

	va_end(ap);


	//Print out formated string
	uart_puts(buffer);
}
