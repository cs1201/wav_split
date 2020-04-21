#ifndef WAV_SPLIT_H
#define WAV_SPLIT_H

#include "sndfile.h"

#define VERSION_MAJOR (1)
#define VERSION_MINOR (0)

typedef struct{
    const char *name;
    SNDFILE *handle;
    SF_INFO info;
    int bit_depth;
    char *wav_type;
}wav_split_file_t;

/* Function prototypes*/
void print_usage();
void print_file_info(wav_split_file_t *file);
wav_split_file_t* init_wav_split_file();
const char* generate_output_filename(const char* filename, int index);

#endif