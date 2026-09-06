#include "mp3_tag_reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* =========================================================
   WRITE 32-BIT BIG ENDIAN INTEGER
   ========================================================= */

static void write_big_endian(unsigned char data[],
                             unsigned int value)
{
    data[0] = (value >> 24) & 0xFF;
    data[1] = (value >> 16) & 0xFF;
    data[2] = (value >> 8)  & 0xFF;
    data[3] = value & 0xFF;
}


/* =========================================================
   WRITE SYNCHSAFE TAG SIZE
   ========================================================= */

void update_tag_size(FILE *output_fp,
                     unsigned char header[],
                     unsigned int tag_size)
{
    /*
     * ID3 header stores tag size as
     * a 28-bit synchsafe integer.
     */

    header[6] =
        (tag_size >> 21) & 0x7F;

    header[7] =
        (tag_size >> 14) & 0x7F;

    header[8] =
        (tag_size >> 7) & 0x7F;

    header[9] =
        tag_size & 0x7F;


    /*
     * Header bytes 6-9 contain the
     * updated tag size.
     */

    if (fseek(output_fp,
              6,
              SEEK_SET) != 0)
    {
        return;
    }


    fwrite(&header[6],
           1,
           4,
           output_fp);
}


/* =========================================================
   BUILD NORMAL TEXT FRAME DATA
   ========================================================= */

static unsigned char *create_text_data(char new_text[],
                                        unsigned int *new_size)
{
    unsigned int text_size;

    unsigned char *data;


    text_size = strlen(new_text);


    /*
     * One encoding byte + text.
     *
     * Encoding 0 = ISO-8859-1.
     */

    *new_size = text_size + 1;


    data = malloc(*new_size);


    if (data == NULL)
    {
        return NULL;
    }


    data[0] = 0;


    memcpy(&data[1],
           new_text,
           text_size);


    return data;
}


/* =========================================================
   CREATE COMM FRAME DATA
   ========================================================= */

static unsigned char *create_comm_data(unsigned char old_data[],
                                        unsigned int old_size,
                                        char new_text[],
                                        unsigned int *new_size)
{
    unsigned int description_end;
    unsigned int description_length;

    unsigned int new_comment_length;

    unsigned char *data;


    /*
     * We support the common one-byte encoded
     * COMM representation here.
     *
     * Existing:
     *
     * Encoding
     * Language
     * Description
     * NULL
     * Comment
     */

    if (old_size < 5)
    {
        return NULL;
    }


    /*
     * UTF-16 COMM needs different handling
     * because its terminator is two bytes.
     *
     * For this project we preserve and edit
     * ISO-8859-1 / UTF-8 style COMM frames.
     */

    if (old_data[0] == 1 ||
        old_data[0] == 2)
    {
        printf("UTF-16 COMM editing is not supported.\n");
        return NULL;
    }


    /*
     * Find end of description.
     */

    description_end = 4;


    while (description_end < old_size &&
           old_data[description_end] != '\0')
    {
        description_end++;
    }


    if (description_end >= old_size)
    {
        return NULL;
    }


    description_length =
        description_end - 4;


    new_comment_length =
        strlen(new_text);


    /*
     * New size:
     *
     * Encoding      = 1
     * Language      = 3
     * Description   = existing
     * NULL          = 1
     * New comment   = new length
     */

    *new_size =
        1 +
        3 +
        description_length +
        1 +
        new_comment_length;


    data = malloc(*new_size);


    if (data == NULL)
    {
        return NULL;
    }


    /*
     * Preserve encoding.
     */

    data[0] = old_data[0];


    /*
     * Preserve language.
     */

    memcpy(&data[1],
           &old_data[1],
           3);


    /*
     * Preserve description.
     */

    if (description_length > 0)
    {
        memcpy(&data[4],
               &old_data[4],
               description_length);
    }


    /*
     * Description terminator.
     */

    data[4 + description_length] = '\0';


    /*
     * New comment.
     */

    memcpy(&data[5 + description_length],
           new_text,
           new_comment_length);


    return data;
}


/* =========================================================
   EDIT FRAME
   ========================================================= */

int edit_frame(FILE *input_fp,
               FILE *output_fp,
               long tag_end,
               unsigned int *tag_size,
               char target_id[],
               char new_text[])
{
    char frame_id[5];

    unsigned char size[4];
    unsigned char flags[2];

    unsigned int frame_size;
    unsigned int new_frame_size;

    unsigned char *old_data;
    unsigned char *new_data;

    int found = 0;


    while (1)
    {
        long current_position = ftell(input_fp);


        if (current_position < 0)
        {
            break;
        }


        /*
         * Need a complete frame header.
         */

        if (current_position + 10 > tag_end)
        {
            break;
        }


        /*
         * Read frame ID.
         */

        if (fread(frame_id,
                  1,
                  4,
                  input_fp) != 4)
        {
            break;
        }

        frame_id[4] = '\0';


        /*
         * Padding.
         */

        if (frame_id[0] == '\0')
        {
            /*
             * Copy the padding byte and
             * everything after it.
             */

            fputc('\0',
                  output_fp);

            int ch;

            while ((ch = fgetc(input_fp)) != EOF)
            {
                fputc(ch, output_fp);
            }

            break;
        }


        /*
         * Read frame size.
         */

        if (fread(size,
                  1,
                  4,
                  input_fp) != 4)
        {
            break;
        }


        frame_size =
            ((unsigned int)size[0] << 24) |
            ((unsigned int)size[1] << 16) |
            ((unsigned int)size[2] << 8)  |
            (unsigned int)size[3];


        /*
         * Read flags.
         */

        if (fread(flags,
                  1,
                  2,
                  input_fp) != 2)
        {
            break;
        }


        /*
         * Protect against invalid frame sizes.
         */

        if (frame_size > MAX_FRAME_SIZE)
        {
            printf("Frame too large.\n");
            return 0;
        }


        if (ftell(input_fp) < 0 ||
            ftell(input_fp) + frame_size > tag_end)
        {
            printf("Invalid frame size.\n");
            return 0;
        }


        /*
         * Read old frame data.
         */

        old_data = NULL;


        if (frame_size > 0)
        {
            old_data = malloc(frame_size);


            if (old_data == NULL)
            {
                printf("Memory allocation failed\n");
                return 0;
            }


            if (fread(old_data,
                      1,
                      frame_size,
                      input_fp) != frame_size)
            {
                free(old_data);
                return 0;
            }
        }


        /*
         * Check whether this is the frame
         * requested by the user.
         */

        if (strcmp(frame_id,
                   target_id) == 0)
        {
            found = 1;


            printf("\n");
            printf("Frame found : %s\n",
                   frame_id);

            printf("Old size    : %u bytes\n",
                   frame_size);
                   printf("Old data    : ");

if (frame_size > 1)
{
    unsigned int i;

    for (i = 1; i < frame_size; i++)
    {
        if (old_data[i] == '\0')
        {
            break;
        }

        if (old_data[i] >= 32 &&
            old_data[i] <= 126)
        {
            putchar(old_data[i]);
        }
        else
        {
            putchar('?');
        }
    }
}

printf("\n");


            new_data = NULL;


            /*
             * COMM requires special handling.
             */

            if (strcmp(target_id,
                       "COMM") == 0)
            {
                if (old_data == NULL)
                {
                    printf("Invalid COMM frame.\n");
                    return 0;
                }


                new_data =
                    create_comm_data(old_data,
                                     frame_size,
                                     new_text,
                                     &new_frame_size);
            }

            else
            {
                /*
                 * All supported normal text frames:
                 *
                 * TIT2
                 * TPE1
                 * TALB
                 * TRCK
                 * TYER
                 * TCON
                 */

                new_data =
                    create_text_data(new_text,
                                     &new_frame_size);
            }


            if (new_data == NULL)
            {
                free(old_data);

                printf("Unable to create new frame data.\n");

                return 0;
            }


            printf("New size    : %u bytes\n",
                   new_frame_size);
                   printf("New data    : %s\n",
       new_text);


            /*
             * Update total ID3 tag size.
             *
             * New tag size =
             *
             * old tag size
             * - old frame data size
             * + new frame data size
             */

            if (new_frame_size > frame_size)
            {
                *tag_size +=
                    new_frame_size - frame_size;
            }
            else
            {
                *tag_size -=
                    frame_size - new_frame_size;
            }


            /*
             * Create new frame size field.
             */

            write_big_endian(size,
                             new_frame_size);


            /*
             * Write frame ID.
             */

            fwrite(frame_id,
                   1,
                   4,
                   output_fp);


            /*
             * Write new frame size.
             */

            fwrite(size,
                   1,
                   4,
                   output_fp);


            /*
             * Preserve flags.
             */

            fwrite(flags,
                   1,
                   2,
                   output_fp);


            /*
             * Write new frame data.
             */

            fwrite(new_data,
                   1,
                   new_frame_size,
                   output_fp);


            free(old_data);
            free(new_data);


            /*
             * Copy everything after the edited frame.
             *
             * This includes:
             *
             * remaining ID3 frames
             * ID3 padding
             * MP3 audio data
             */

            int ch;

            while ((ch = fgetc(input_fp)) != EOF)
            {
                fputc(ch,
                      output_fp);
            }


            break;
        }


        /*
         * -------------------------------------------------
         * Frame is NOT the target.
         *
         * Copy it unchanged.
         * -------------------------------------------------
         */

        fwrite(frame_id,
               1,
               4,
               output_fp);

        fwrite(size,
               1,
               4,
               output_fp);

        fwrite(flags,
               1,
               2,
               output_fp);


        if (frame_size > 0)
        {
            fwrite(old_data,
                   1,
                   frame_size,
                   output_fp);
        }


        free(old_data);
    }


    if (!found)
    {
        printf("\nFrame %s not found.\n",
               target_id);

        return 0;
    }


    return 1;
}


/* =========================================================
   EDIT MP3
   ========================================================= */

int edit_mp3(char input_file[],
             char output_file[],
             char target_id[],
             char new_text[])
{
    FILE *input_fp;
    FILE *output_fp;

    unsigned char header[10];

    unsigned int tag_size;

    long tag_end;

    int result;


    /*
     * Input file.
     */

    input_fp =
        fopen(input_file,
              "rb");


    if (input_fp == NULL)
    {
        printf("Unable to open input file: %s\n",
               input_file);

        return 0;
    }


    /*
     * Read header.
     */

    if (!read_id3_header(input_fp,
                          header))
    {
        fclose(input_fp);

        return 0;
    }


    /*
     * Validate header.
     */

    if (!validate_id3_header(header))
    {
        printf("Invalid or unsupported ID3 file.\n");

        fclose(input_fp);

        return 0;
    }


    /*
     * Get original tag size.
     */

    tag_size =
        get_tag_size(header);


    tag_end =
        10 + tag_size;


    /*
     * Output file.
     */

    output_fp =
        fopen(output_file,
              "wb+");


    if (output_fp == NULL)
    {
        printf("Unable to create output file: %s\n",
               output_file);

        fclose(input_fp);

        return 0;
    }


    /*
     * Copy original 10-byte header.
     */

    if (fwrite(header,
               1,
               10,
               output_fp) != 10)
    {
        printf("Unable to write output file.\n");

        fclose(input_fp);
        fclose(output_fp);

        return 0;
    }


    /*
     * Move input to first frame.
     */

    if (fseek(input_fp,
              10,
              SEEK_SET) != 0)
    {
        fclose(input_fp);
        fclose(output_fp);

        return 0;
    }


    /*
     * Edit requested frame.
     */

    result =
        edit_frame(input_fp,
                   output_fp,
                   tag_end,
                   &tag_size,
                   target_id,
                   new_text);


    if (!result)
    {
        fclose(input_fp);
        fclose(output_fp);

        return 0;
    }


    /*
     * Update header with new tag size.
     */

    update_tag_size(output_fp,
                    header,
                    tag_size);


    fclose(input_fp);
    fclose(output_fp);


    printf("\nMP3 modification successful.\n");
    printf("New ID3 Tag Size : %u bytes\n",
           tag_size);

    printf("Output file      : %s\n",
           output_file);


    return 1;
}