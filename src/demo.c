
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "libirig106.h"
#include "i106_decode_1553f1.h"
#include "util.h"


int main(int argc, char *argv[]){
    I106Status status;
    I106C10Header header;
    MS1553F1_Message msg;

    int fd = open("test.c10", 0);
    while (!(status = I106NextHeader(fd, &header))){

        if (header.DataType == I106CH10_DTYPE_1553_FMT_1){  // 0x19

            // Read data into buffer
            int buffer_size = GetDataLength(&header);
            char *buffer = (char *)malloc(buffer_size);
            int read_count = read(fd, buffer, buffer_size);

            // Read CSDW and first message
            status = I106_Decode_First1553F1(&header, buffer, &msg);

            // Step through 1553 messages
            while (!status){
                // Process message...

                // Read next message
                status = I106_Decode_Next1553F1(&msg);
            }

            free(buffer);
        }

        else
            lseek(fd, GetDataLength(&header), SEEK_CUR);
    }

    printf("Done\n");
    return 0;
}
