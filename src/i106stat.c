
#include <stdio.h>

#include "irig106ch10.h"
#include "i106stat_args.c"


typedef struct {
    unsigned int type;
    int id;
    int packets;
} ChanSpec;


int main(int argc, char *argv[]){

    // Get commandline args
    DocoptArgs args = docopt(argc, argv, 1, "1");
    if (args.help){
        printf(args.help_message);
        return 0;
    }
    else if (argc < 2){
        printf(args.usage_pattern);
        printf("\n");
        return 0;
    }

    // Channel info
	I106C10Header header;
    int packets = 0, input_handle;
    float byte_size = 0.0;
    static ChanSpec * channels[0x10000];

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

        // Find the channel info based on ID and type.
        int i;
        for (i = 0; i < 0x10000; i++){

            // Create a listing if none exists.
            if (channels[i] == NULL){
                channels[i] = (ChanSpec *)malloc(sizeof(ChanSpec));
                memset(channels[i], 0, sizeof(ChanSpec));
                channels[i]->id = (unsigned int)header.ChannelID;
                channels[i]->type = (unsigned int)header.DataType;
                channels[i]->packets = 0;
            }

            // 
            if (channels[i]->id == (unsigned int)header.ChannelID && channels[i]->type == (unsigned int)header.DataType){
                break;
            }
        }

        channels[i]->packets++;
    }

	// Print details for each channel.
    printf("Channel ID      Data Type%35sPackets\n", "");
    printf("--------------------------------------------------------------------------------\n");
    int i = 0;
    while (channels[i] != NULL){
        printf("Channel%3d", channels[i]->id);
        printf("%6s", "");
        printf("0x%-34x", channels[i]->type);
        printf("%7d packets", channels[i]->packets);
        printf("\n");
        i++;
    }

    // Find a more readable size unit than bytes.
    char *unit = "b";
    if (byte_size > 1024){
        byte_size /= 1024;
        unit = "kb";
    }
    if (byte_size > 1024){
        byte_size /= 1024;
        unit = "mb";
    }
    if (byte_size > 1024){
        byte_size /= 1024;
        unit = "gb";
    }

    // Print file summary.
    printf("--------------------------------------------------------------------------------\n");
    printf("Summary for %s:\n", argv[1]);
    printf("    Size: %.*f%s\n", 2, byte_size, unit);
    printf("    Packets: %d\n", packets);
    printf("    Channels: %d\n", i);
}
