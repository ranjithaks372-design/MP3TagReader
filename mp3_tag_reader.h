#ifndef MP3_TAG_READER_H
#define MP3_TAG_READER_H

#include <stdio.h>

#define MAX_TITLE          256
#define MAX_ARTIST         256
#define MAX_ALBUM          256
#define MAX_TRACK          64
#define MAX_YEAR           64
#define MAX_RECORDING_TIME 64
#define MAX_GENRE          256
#define MAX_COMMENT        512
#define MAX_TXXX           512
#define MAX_ENCODER        256

#define MAX_FRAME_SIZE     (10 * 1024 * 1024)

/* ---------------- MP3 TAG STRUCTURE ---------------- */

typedef struct
{
    char title[MAX_TITLE];
    char artist[MAX_ARTIST];
    char album[MAX_ALBUM];
    char track[MAX_TRACK];
    char year[MAX_YEAR];
    char recording_time[MAX_RECORDING_TIME];
    char genre[MAX_GENRE];
    char comment[MAX_COMMENT];
    char txxx[MAX_TXXX];
    char encoder[MAX_ENCODER];

} MP3Tags;


/* ---------------- READ FUNCTIONS ---------------- */

int read_id3_header(FILE *fp, unsigned char header[]);

int validate_id3_header(unsigned char header[]);

unsigned int get_tag_size(unsigned char header[]);

void read_frames(FILE *fp,
                 long tag_end,
                 MP3Tags *tags);

void process_frame(char frame_id[],
                   unsigned char frame_data[],
                   unsigned int frame_size,
                   MP3Tags *tags);

void copy_text(char destination[],
               size_t destination_size,
               unsigned char source[],
               unsigned int size);

void display_tags(MP3Tags *tags);


/* ---------------- EDIT FUNCTIONS ---------------- */

int edit_frame(FILE *input_fp,
               FILE *output_fp,
               long tag_end,
               unsigned int *tag_size,
               char target_id[],
               char new_text[]);

void update_tag_size(FILE *output_fp,
                     unsigned char header[],
                     unsigned int tag_size);

int edit_mp3(char input_file[],
             char output_file[],
             char target_id[],
             char new_text[]);


/* ---------------- HELP / VERSION ---------------- */

void display_help(void);

void display_version(void);

#endif