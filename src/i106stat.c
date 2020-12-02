
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "irig106ch10.h"


const int MAX_CHANNELS = 0x10000;

typedef struct {
    int id;
    unsigned int type;
    int packets;

    // Special case for channel 0 which may have multiple channels based on
    // data type
    int user_defined;
    int tmats;
    int events;
    int index;
    int streaming;
} Channel;


// Show progress on screen
void show_progress(float percent){
    printf("\r[");
    int stars = 78 * percent;
    for (int i=0;i<stars;i++)
        printf("#");
    for (int i=0;i<78 - stars;i++)
        printf("_");
    printf("]");
    fflush(stdout);
}


// Print a more human-readable size
char * pretty_size(int64_t size){
    char units[][3] = {"b", "kb", "mb", "gb", "tb"};
    int unit = 0;
    char *result = malloc(20);
    while (size > 1024){
        unit++;
        size /= 1024;
    }
    sprintf(result, "%.*ld%s", 2, size, units[unit]);
    return result;
}


// Print summary for a channel.
void print_channel(int id, int type, int packets){
    printf("Channel%3d", id);
    printf("%6s", "");
    printf("0x%-34x", type);
    printf("%7d packets", packets);
    printf("\n");
}


int main(int argc, char *argv[]){

    // Get commandline args
    if (argc < 2){
        printf("usage: i106stat <filename>\n");
        return 0;
    }

    // Channel info
	I106C10Header header;
    int packets = 0, input_handle;
    Channel channels[MAX_CHANNELS];
    memset(channels, 0, MAX_CHANNELS * sizeof(Channel));

    // Open the source file and get the total size.
    I106Status status = I106C10Open(&input_handle, argv[1], READ);
    if (status != I106_OK){
        printf("Error opening file %s", argv[1]);
        return 1;
    }
    long length = lseek(handles[input_handle].File, 0, SEEK_END);
    lseek(handles[input_handle].File, 0, SEEK_SET);
    long pos = 0;

    printf("Scanning %s of data\n", pretty_size(length));

    // Iterate over packets.
    while (1){

        // Exit once file ends.
        if ((status = I106C10ReadNextHeader(input_handle, &header)))
            break;

        pos += header.PacketLength;
        show_progress((float)pos / (float)length);

        // Increment overall size and packet counts.
        packets++;

        // Create a listing if none exists.
        if (channels[header.ChannelID].packets == 0){
            channels[header.ChannelID].id = header.ChannelID;
            channels[header.ChannelID].type = header.DataType;
        }

        // Increment packet count
        channels[header.ChannelID].packets++;

        // Special case for channel 0. Note different sub-types.
        if (header.ChannelID == 0){
            if (header.DataType == 0)
                channels[header.ChannelID].user_defined++;
            else if (header.DataType == 1)
                channels[header.ChannelID].tmats++;
            else if (header.DataType == 2)
                channels[header.ChannelID].events++;
            else if (header.DataType == 3)
                channels[header.ChannelID].index++;
            else if (header.DataType == 4)
                channels[header.ChannelID].streaming++;
        }
    }

	// Print details for each channel.
    printf("\r%*s\n", 80, "-");
    for (int i=0;i<80;i++)
        printf("-");
    printf("\nChannel ID      Data Type%35sPackets\n", "");
    for (int i=0;i<80;i++)
        printf("-");
    printf("\n");
    int channel_count = 0;
    for (int i=0;i<MAX_CHANNELS;i++){
        if (channels[i].packets == 0){
            if (i == 0)
                printf("Skipping channel %i\n", i);
            continue;
        }
        channel_count += 1;

        if (i != 0)
            print_channel(channels[i].id, channels[i].type, channels[i].packets);
        else {
            if (channels[i].user_defined)
                print_channel(0, 0, channels[i].user_defined);
            if (channels[i].tmats)
                print_channel(0, 1, channels[i].tmats);
            if (channels[i].events)
                print_channel(0, 2, channels[i].events);
            if (channels[i].index)
                print_channel(0, 3, channels[i].index);
            if (channels[i].streaming)
                print_channel(0, 4, channels[i].streaming);
        }
    }

    // Print file summary.
    printf("--------------------------------------------------------------------------------\n");
    printf("Summary for %s:\n", argv[1]);
    printf("    Size: %s\n", pretty_size(length));
    printf("    Packets: %d\n", packets);
    printf("    Channels: %d\n", channel_count);
}
