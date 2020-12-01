
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "irig106ch10.h"
#include "i106stat_args.c"


const int MAX_CHANNELS = 100;

typedef struct {
    int id;
    unsigned int type;
    int packets;
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
    static Channel * channels[0x10000][255];

    // Open the source file and get the total size.
    I106Status status = I106C10Open(&input_handle, argv[1], READ);
    if (status != I106_OK){
        printf("Error opening file %s", argv[1]);
        return 1;
    }
    int64_t length = (int64_t)lseek(handles[input_handle].File, 0, SEEK_END);
    lseek(handles[input_handle].File, 0, SEEK_SET);

    printf("Scanning %s of data\n", pretty_size(length));

    // Iterate over packets.
    while (1){

        // Exit once file ends.
        if ((status = I106C10ReadNextHeader(input_handle, &header)))
            break;

        int64_t pos = lseek(handles[input_handle].File, 0, SEEK_CUR);
        show_progress((float)pos / (float)length);

        // Increment overall size and packet counts.
        packets++;

        // Create a listing if none exists.
        if (channels[header.ChannelID][header.DataType] == NULL){
            Channel channel = {header.ChannelID, header.DataType, 1};
            channels[header.ChannelID][header.DataType] = malloc(sizeof(Channel));
            memcpy(channels[header.ChannelID][header.DataType], &channel, sizeof(Channel));
        }

        // Increment packet count if not
        else
            channels[header.ChannelID][header.DataType]->packets++;
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

    // Print file summary.
    printf("--------------------------------------------------------------------------------\n");
    printf("Summary for %s:\n", argv[1]);
    printf("    Size: %s\n", pretty_size(length));
    printf("    Packets: %d\n", packets);
    printf("    Channels: %d\n", channel_count);
}
