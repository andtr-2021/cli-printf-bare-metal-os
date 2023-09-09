// -----------------------------------mbox.c -------------------------------------
#include "mbox.h"
#include "gpio.h"
#include "uart.h"
#include "../gcclib/stdarg.h"

/* Mailbox Data Buffer (each element is 32-bit)*/
/*
 * The keyword attribute allows you to specify special attributes
 *
 * The aligned(N) attribute aligns the current data item on an address
 * which is a multiple of N, by inserting padding bytes before the data item
 *
 * __attribute__((aligned(16)) : allocate the variable on a 16-byte boundary.
 *
 *
 * We must ensure that our our buffer is located at a 16 byte aligned address,
 * so only the high 28 bits contain the address
 * (last 4 bits is ZERO due to 16 byte alignment)
 *
 */
volatile unsigned int __attribute__((aligned(16))) mBuf[36];
/**
 * Read from the mailbox
 */
uint32_t mailbox_read(unsigned char channel)
{
    // Receiving message is buffer_addr & channel number
    uint32_t res;
    // Make sure that the message is from the right channel
    do
    {
        // Make sure there is mail to receive
        do
        {
            asm volatile("nop");
        } while (MBOX0_STATUS & MBOX_EMPTY);
        // Get the message
        res = MBOX0_READ;
    } while ((res & 0xF) != channel);
    return res;
}
/**
 * Write to the mailbox
 */
void mailbox_send(uint32_t msg, unsigned char channel)
{
    // Sending message is buffer_addr & channel number
    //  Make sure you can send mail
    do
    {
        asm volatile("nop");
    } while (MBOX1_STATUS & MBOX_FULL);
    // send the message
    MBOX1_WRITE = msg;
}
/**
 * Make a mailbox call. Returns 0 on failure, non-zero on success
 */
int mbox_call(unsigned int buffer_addr, unsigned char channel)
{
    // Check Buffer Address
    // uart_puts("Buffer Address: ");
    // uart_hex(buffer_addr);
    // uart_sendc('\n');
    // Prepare Data (address of Message Buffer)
    unsigned int msg = (buffer_addr & ~0xF) | (channel & 0xF);
    mailbox_send(msg, channel);
    /* now wait for the response */
    /* is it a response to our message (same address)? */
    if (msg == mailbox_read(channel))
    {
        /* is it a valid successful response (Response Code) ? */
        if (mBuf[1] == MBOX_RESPONSE)
            uart_puts("Got successful response! \n");
        return (mBuf[1] == MBOX_RESPONSE);
    }
    return 0;
}

void mbox_buffer_setup(unsigned int buffer_addr, unsigned int tag_identifier,
                       unsigned int **res_data, unsigned int res_length,
                       unsigned int req_length, unsigned int num_args, ...) {
    // Set up a va_list to access the variable arguments
    va_list args;
    va_start(args, num_args);
    
    // Calculate the total length needed for the mailbox message
    unsigned int total_length = 3 + num_args + (req_length >> 2);
    
    // Populate the mailbox buffer with the message
    unsigned int *mbox = (unsigned int *)buffer_addr;
    mbox[0] = total_length << 2;        // Total message length
    mbox[1] = 0;                         // Request/response code
    mbox[2] = tag_identifier;            // Tag identifier
    mbox[3] = req_length;                // Request value buffer size in bytes
    mbox[4] = res_length;                // Response value buffer size in bytes
    
    // Populate the request values
    for (int i = 0; i < num_args; ++i) {
        mbox[5 + i] = va_arg(args, unsigned int);
    }
    
    // Terminate the va_list
    va_end(args);
    
    // Set the response data pointer
    *res_data = &mbox[5];
}