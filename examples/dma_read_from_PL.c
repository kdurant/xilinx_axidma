/**
only read data from to PL
 **/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <fcntl.h>      // Flags for open()
#include <sys/stat.h>   // Open() system call
#include <sys/types.h>  // Types for open()
#include <unistd.h>     // Close() system call
#include <string.h>     // Memory setting and copying
#include <getopt.h>     // Option parsing
#include <errno.h>      // Error codes

#include "libaxidma.h"  // Interface ot the AXI DMA library

/*----------------------------------------------------------------------------
 * Internal Definitions
 *----------------------------------------------------------------------------*/

// A convenient structure to carry information around about the transfer
struct dma_transfer
{
    int   input_channel;  // The channel used to send the data to PL
    int   input_size;     // The amount of data to send
    void *input_buf;      // The buffer to hold the input data

    int   output_channel;  // The channel used to receive the data from PL
    int   output_size;     // The amount of data to receive
    void *output_buf;      // The buffer to hold the output
};

int len = 0;

int main(int argc, char **argv)
{
    int                 rc;
    struct dma_transfer trans;
    memset(&trans, 0, sizeof(trans));

    axidma_dev_t   axidma_dev;
    const array_t *tx_chans, *rx_chans;

    axidma_dev = axidma_init();
    if(axidma_dev == NULL)
    {
        fprintf(stderr, "Error: Failed to initialize the AXI DMA device.\n");
        rc = 1;
    }

    // Get the tx and rx channels if they're not already specified
    tx_chans = axidma_get_dma_tx(axidma_dev);
    if(tx_chans->len < 1)
    {
        fprintf(stderr, "Error: No transmit channels were found.\n");
        rc = -ENODEV;
        goto destroy_axidma;
    }
    rx_chans = axidma_get_dma_rx(axidma_dev);
    if(rx_chans->len < 1)
    {
        fprintf(stderr, "Error: No receive channels were found.\n");
        rc = -ENODEV;
        goto destroy_axidma;
    }

    /* If the user didn't specify the channels, we assume that the transmit and
     * receive channels are the lowest numbered ones. */
    // if(trans.input_channel == -1 && trans.output_channel == -1)
    {
        trans.input_channel  = tx_chans->data[0];
        trans.output_channel = rx_chans->data[0];
    }
    printf("\tTransmit Channel: %d\n", trans.input_channel);
    printf("\tReceive Channel: %d\n", trans.output_channel);

    trans.output_size = 1024;
    trans.output_buf  = axidma_malloc(axidma_dev, trans.output_size);

    while(1)
    {
        rc = axidma_oneway_transfer(axidma_dev, trans.output_channel, trans.output_buf, len, true);
        rc = (rc < 0) ? -rc : 0;
    }

destroy_axidma:
    axidma_destroy(axidma_dev);
ret:
    return rc;
}
