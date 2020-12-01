
#include <stdio.h>

#include "irig106ch10.h"
#include "i106stat_args.c"


const int MAX_CHANNELS = 100;

typedef struct {
    int id;
    unsigned int type;
    int packets;
} ChanSpec;


int main(int argc, char *argv[]){

    // Get commandline args
    DocoptArgs args = docopt(argc, argv, 1, "1");
    if (args.help){
        printf("%s", args.help_message);
        return 0;
    }
    else if (argc < 2){
        printf("%s\n", args.usage_pattern);
        return 0;
    }

    // Channel info
	I106C10Header header;
    int packets = 0, input_handle;
    float byte_size = 0.0;
    static ChanSpec * channels[0x10000][255];

    // Open the source file.
    I106Status status = I106C10Open(&input_handle, argv[1], READ);
    if (status != I106_OK){
        char msg[200] = "Error opening file ";
        strcat(msg, argv[1]);
        printf("%s", msg);
        return 1;
    }

    // Iterate over selected packets (based on args).
    while (1){

        // Exit once file ends.
        if ((status = I106C10ReadNextHeader(input_handle, &header)))
            break;

        // Increment overall size and packet counts.
        byte_size += header.PacketLength;
        packets++;

        // Create a listing if none exists.
        if (channels[header.ChannelID][header.DataType] == NULL){
            ChanSpec channel = {header.ChannelID, header.DataType, 1};
            channels[header.ChannelID][header.DataType] = malloc(sizeof(ChanSpec));
            memcpy(channels[header.ChannelID][header.DataType], &channel, sizeof(ChanSpec));
        }

        // Increment packet count if not
        else
            channels[header.ChannelID][header.DataType]->packets++;
    }

	// Print details for each channel.
    printf("Channel ID      Data Type%35sPackets\n", "");
    printf("--------------------------------------------------------------------------------\n");
    int channel_count = 0;
    for (int i=0;i<MAX_CHANNELS;i++){
        for (int j=0;j<=255;j++){
            if (channels[i][j] == NULL)
                continue;
            channel_count += 1;
            printf("Channel%3d", channels[i][j]->id);
            printf("%6s", "");
            printf("0x%-34x", channels[i][j]->type);
            printf("%7d packets", channels[i][j]->packets);
            printf("\n");
        }
    }

    // Find a more readable size unit than bytes.
    char units[][3] = {"b", "kb", "mb", "gb", "tb"};
    int unit = 0;
    while (byte_size > 1024){
        unit++;
        byte_size /= 1024;
    }

    // Print file summary.
    printf("--------------------------------------------------------------------------------\n");
    printf("Summary for %s:\n", argv[1]);
    printf("    Size: %.*f%s\n", 2, byte_size, units[unit]);
    printf("    Packets: %d\n", packets);
    printf("    Channels: %d\n", channel_count);
}
