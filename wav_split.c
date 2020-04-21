#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sndfile.h>

#include "wav_split.h"

void sf_err_print(int sf_err_code){
    if (sf_err_code != SF_ERR_NO_ERROR){
        switch(sf_err_code){
            case (SF_ERR_UNRECOGNISED_FORMAT):
                fprintf(stderr, "[ERROR] Unrecognised file format\n");
                break;
            case (SF_ERR_SYSTEM):
                fprintf(stderr, "[ERROR] System Error\n");
                break;
            case (SF_ERR_MALFORMED_FILE):
                fprintf(stderr, "[ERROR] Malformed File\n");
                break;
            case (SF_ERR_UNSUPPORTED_ENCODING):
                fprintf(stderr, "[ERROR] Unsupported Encoding\n");
                break;
            default:
                fprintf(stderr, "[ERROR] Unrecognised error\n");
        }
    }
}

void print_file_info(const char* file_name, const SF_INFO *sf_info){
    if(!file_name || !sf_info){
        fprintf(stderr, "[ERROR] Passed invalid file info to print_file_info()");
    }

    printf("Input File: %s\n", file_name);
    printf("    Length: %.1f Seconds\n", (double)sf_info->frames/sf_info->samplerate);
    printf("    Channels: %d\n", sf_info->channels);
    printf("    Sample Rate: %d\n", sf_info->samplerate);
    printf("    Samples: %d\n", (int)sf_info->frames);

}

const char* generate_output_filename(const char* filename, int index){

    const int filename_size = sizeof(char) * (strlen(filename) + 1);
    char *new_filename;
    
    new_filename = malloc(filename_size);
    strcpy(new_filename, filename);
    
    // Split extension
    char *base_filename = strtok(new_filename, ".");
    char *file_ext = strtok(NULL, "");
    char *out_filename = malloc(filename_size + (sizeof(char) * 2));
    snprintf(out_filename, filename_size+(sizeof(char) * 2), "%s_%d.%s",  base_filename, index, file_ext);

    return out_filename;
}

int main(){

    //Use libsndfile to split multi-channel wav file into mono wavs
    int err = 0;
    SNDFILE *in_file;
    SF_INFO in_file_info;
    memset(&in_file_info, 0, sizeof(SF_INFO));

    const char* in_file_name = "/Users/cston/Developer/libsndfile_ex/2_chan_id.wav";
    //Open input file
    in_file = sf_open(in_file_name, SFM_READ, &in_file_info);
    if((err = sf_error(in_file))){
        printf("[ERROR] %s\n", sf_error_number(err));
        return 1;
    }
    
    //Show input file info
    print_file_info(in_file_name, &in_file_info);

    int num_outfiles= in_file_info.channels;
    int wav_format = in_file_info.format & 0xffff0000;
    int bit_depth_tag = in_file_info.format & 0xffff;

    if(num_outfiles > MAX_NUM_CHANNELS){
        fprintf(stderr, "[ERROR] Cannot process. Input file contains more than %d channels\n", MAX_NUM_CHANNELS);
        return 1;
    }
    if(bit_depth_tag > 7){
        fprintf(stderr, "[ERROR] Un-supported bit-depth format\n");
        return 1;
    }
    if(!(wav_format == SF_FORMAT_WAV || wav_format == SF_FORMAT_WAV)){
        fprintf(stderr, "[ERROR] Cannot process. Non- WAV or WAVEX format file\n");
        return 1;
    }

    const char *output_file_names[num_outfiles];
    SNDFILE *output_files[num_outfiles];    
    SF_INFO output_files_info[num_outfiles];

    generate_output_filename(in_file_name, 1);

    // Open output files
    for(int i=0; i < num_outfiles; i++){

        output_file_names[i] = generate_output_filename(in_file_name, i);
        // printf("Output file %d: \"%s\"\n", i, output_file_names[i]);

        memset(&output_files_info[i], 0, sizeof(SF_INFO));
        output_files_info[i].channels = 1;
        output_files_info[i].samplerate = in_file_info.samplerate;
        output_files_info[i].format = in_file_info.format;
        output_files_info[i].sections = in_file_info.sections;
        output_files_info[i].seekable = in_file_info.seekable;

        output_files[i] = sf_open(output_file_names[i], SFM_WRITE, &output_files_info[i]);
        if((err = sf_error(output_files[i]))){
             printf("[ERROR] %s\n", sf_error_number(err));
            return 1;
        }
    }

    // Loop through input file
    int read_count = 0;
    int *data = malloc(sizeof(int) * MAX_NUM_CHANNELS);
    while( (read_count = sf_readf_int(in_file, data, 1)) ){
        // Read each channels word into it's own file
        for(int ch=0; ch < num_outfiles; ch++){
            sf_write_int(output_files[ch], &(data[ch]), 1);
        }
    }

    // Finalise output files
    for(int i=0; i < num_outfiles; i++){
        sf_close(output_files[i]);
    }

    // cleanup:
    //Close input file
    if(in_file){
        sf_close(in_file);
    }
    if(data){
        free(data);
    }
    
    return 0;
}