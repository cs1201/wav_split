#ifndef WAV_SPLIT_H
#define WAV_SPLIT_H

#define MAX_NUM_CHANNELS (8)

void sf_err_print(int sf_err_code);
void print_file_info(const char* file_name, const SF_INFO *sf_info);
const char* generate_output_filename(const char* filename, int index);

#endif