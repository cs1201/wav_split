#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sndfile.h>

#include "wav_split.h"

void print_usage(){
    puts("\nWAV_SPLIT - Simple WAV file splitting utility");
    puts("  Usage:");
    puts("      wav_split {INPUT_FILE} -v");
    puts("  Options:");
    puts("      -v: verbose mode (show input file info and output file names)");
}

void print_file_info(wav_split_file_t *file){

    if(!file){
        fprintf(stderr, "[ERROR] Passed invalid file info to print_file_info()");
    }

    printf("Input File: %s\n", file->name);
    printf("    Length: %.1f Seconds\n", (double)file->info.frames/file->info.samplerate);
    printf("    Channels: %d\n", file->info.channels);
    printf("    Sample Rate: %dHz\n", file->info.samplerate);
    if(file->bit_depth > 0)
        printf("    Bit-depth: %d\n", file->bit_depth);
    else
    {
        printf("    Bit-depth: unsupported\n");
    }
    
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

    free(new_filename);

    return out_filename;
}

wav_split_file_t* init_wav_split_file(){

    wav_split_file_t *temp;
    temp = (wav_split_file_t*)malloc(sizeof(wav_split_file_t));

    if(!temp){ return NULL; };

    temp->name = "";
    temp->handle = NULL;
    temp->bit_depth = -1;
    temp->wav_type = "";

    memset(&temp->info, 0, sizeof(SF_INFO));

    return temp;
}

int file_exists(const char *filename){
    FILE *tmp =  fopen(filename, "rb");
    int rtn = (tmp != NULL);
    fclose(tmp);
    return rtn;
}

int main(int argc, char** argv){

    int error = 0;
    int num_channels;
    int bit_depth_tag;
    int bit_depth = -1;
    int b_verbose = 1;
    wav_split_file_t *in_file = init_wav_split_file();

    if (argc < 2){
        fprintf(stderr, "[ERROR] Require at least one argument\n");
        print_usage();
        return 1;
    }
    for(int i=1; i<argc; i++){
        if(!strncmp(*(argv+i), "-v", 2)){
            b_verbose = 1;
        }else{
            if(file_exists(*(argv+i))){
                in_file->name = *(argv+i);
            }else{
                fprintf(stderr, "[ERROR] Could not access input file\n");
                return 1;
            }
            
        }
    }

#ifdef DEBUG
    puts("DEBUG_INFO");
    printf("Args: %s\n", in_file->name);
    printf("Verbose: %d\n\n", b_verbose);
#endif 

    /* Open input file */
    in_file->handle = sf_open(in_file->name, SFM_READ, &in_file->info);
    if((error = sf_error(in_file->handle))){
        printf("[ERROR] %s\n", sf_error_number(error));
        return 1;
    }

    /* Get bit-depth info for file */
    bit_depth_tag = in_file->info.format & 0xFFFF;

    switch(bit_depth_tag){
        case (SF_FORMAT_PCM_S8):
        case (SF_FORMAT_PCM_U8):
            bit_depth = 8;
            break;
        case (SF_FORMAT_PCM_16):
            bit_depth = 16;
            break;
        case (SF_FORMAT_PCM_24):
            bit_depth = 24;
            break;
        case (SF_FORMAT_PCM_32):
        case (SF_FORMAT_FLOAT):
            bit_depth = 32;
            break;
        case (SF_FORMAT_DOUBLE):
            bit_depth = 64;
            break;
        default:
            break;
    }
    in_file->bit_depth = bit_depth;

    if (b_verbose){
        print_file_info(in_file);
        puts("");
        printf("**Generating %d output files**\n", in_file->info.channels);
    }

    num_channels = in_file->info.channels;
    wav_split_file_t *out_files[num_channels];

    /* Open and format output files */
    for(int i=0; i < num_channels; i++){
        out_files[i] = init_wav_split_file();
        out_files[i]->name = generate_output_filename(in_file->name, i);
        out_files[i]->info.channels = 1; //MONO 1ch output
        out_files[i]->info.samplerate = in_file->info.samplerate;
        out_files[i]->info.format = in_file->info.format;

        if(b_verbose)
            printf("    [Output %d]: %s\n", i+1, out_files[i]->name);

        /* Open file */
        out_files[i]->handle = sf_open(out_files[i]->name, SFM_WRITE, &out_files[i]->info);
        if( (error = sf_error(out_files[i]->handle)) ){
            printf("[ERROR] %s\n", sf_error_number(error));
            return 1;
        }
    }

    int *data = malloc(sizeof(int) * num_channels);

    /* Extract each channel word to it's own file */
    while(sf_readf_int(in_file->handle, data, 1)){
        for(int ch=0; ch < num_channels; ch++){
            sf_write_int(out_files[ch]->handle, &(data[ch]), 1);
        }
    }

    /* get current file info for output files */ 
    for(int i=0; i < num_channels; i++){
        sf_command(out_files[i]->handle, SFC_GET_CURRENT_SF_INFO, &out_files[i]->info, sizeof(SF_INFO));
        if(sf_error(out_files[i]->handle)){
            printf("[ERROR] %s\n", sf_strerror(out_files[i]->handle));
            return 1;
        }
    }

    //Close output files
    for(int i=0; i < num_channels; i++){
        sf_close(out_files[i]->handle);
    }

    //Close input file
    if(in_file)
        sf_close(in_file->handle);

    if(data)
        free(data);
    
    return 0;
}