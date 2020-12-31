
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <cmath>

#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#endif


extern "C" {
    #include "libirig106.h"
}


// Possible data types
std::unordered_map<int, std::string> TYPES = {
    {0x0, "Computer F0 - User Defined"},
    {0x1, "Computer F1 - Setup/TMATS"},
    {0x2, "Computer F2 - Events"},
    {0x3, "Computer F3 - Index"},
    {0x4, "Computer F3 - Streaming"},
    {0x9, "PCM F1"},
    {0x11, "Time F1 - RCC/GPS/RTC"},
    {0x12, "Time F2 - Network"},
    {0x19, "1553 F1 - 1553B Data"},
    {0x20, "1553 F2 - 16PP194 Data"},
    {0x21, "Analog F1"},
    {0x29, "Discrete F1"},
    {0x30, "Message F0"},
    {0x38, "ARINC-429 F0"},
    {0x40, "Video F0 - MPEG-2/H.264"},
    {0x41, "Video F1 - 13818-1 MP2"},
    {0x42, "Video F2 - MPEG-4"},
    {0x43, "Video F3 - MJPEG"},
    {0x44, "Video F4 - MJPEG-2000"},
    {0x48, "Image F0 - Image Data"},
    {0x49, "Image F1 - Still"},
    {0x4a, "Image F2 - Dynamic"},
    {0x50, "UART F0"},
    {0x58, "1394 F0 - Transaction"},
    {0x59, "1394 F1 - Physical"},
    {0x60, "Parallel F0"},
    {0x68, "Ethernet F0"},
    {0x69, "Ethernet F1 - UDP"},
    {0x70, "TSPI/CTS F0 - GPS"},
    {0x71, "TSPI/CTS F1 - EAG ACMI"},
    {0x72, "TSPI/CTS F2 - ACTTS"},
    {0x78, "CAN Bus"},
    {0x79, "Fibre F0"},
    {0x7a, "Fibre F1"},
};


const int MAX_CHANNELS = 0xff;

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


// Print a more human-readable size
char* pretty_size(int64_t size){
    char units[][3] = {"b", "kb", "mb", "gb", "tb"};
    int unit = 0;
    char *result = (char *)malloc(20);
    while (size > 1024){
        unit++;
        size /= 1024;
    }
    sprintf(result, "%.*ld%s", 2, size, units[unit]);
    return result;
}

// Print summary for a channel.
void print_channel(int id, int type, int packets){
    std::string type_name = TYPES[type];
    if (type_name == "")
        type_name = "Unknown";
    printf("| Channel%3d ", id);
    printf("| 0x%02d %-40s ", type, type_name.c_str());
    printf("| %7d packets |", packets);
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
    int packets = 0;
    Channel channels[MAX_CHANNELS];
    memset(channels, 0, MAX_CHANNELS * sizeof(Channel));

    // Open the source file and get the total size.
    I106Status status;
    int fd = open(argv[1], 0);
    off_t length = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    off_t pos = 0;

    printf("Scanning %s of data\n", pretty_size(length));
    float progress, last_progress;

    // Iterate over packets.
    while (1){

        // Exit once file ends.
        if ((status = I106NextHeader(fd, &header)))
            break;

        pos += header.PacketLength;

        // Skip the packet body and trailer
        lseek(fd, pos, SEEK_SET);

        std::cout << "\r" << std::ceil((pos/length) * 100) << "%" << std::flush;

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
    printf("\n| Channel ID | Data Type%36s | Packets%9s|\n", "", "");
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
