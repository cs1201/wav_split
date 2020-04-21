#ifndef WAV_SPLIT_H
#define WAV_SPLIT_H

#include "sndfile.h"

typedef struct{
    const char *name;
    SNDFILE *handle;
    SF_INFO info;
    int bit_depth;
    char *wav_type;
}wav_split_file_t;

/* Function prototypes*/
void sf_err_print(int sf_err_code);
void print_file_info(wav_split_file_t *file);
wav_split_file_t* init_wav_split_file();
const char* generate_output_filename(const char* filename, int index);

#endif