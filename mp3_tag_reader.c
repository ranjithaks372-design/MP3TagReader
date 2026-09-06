#include "mp3_tag_reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* =========================================================
   READ ID3 HEADER
   ========================================================= */

int read_id3_header(FILE *fp, unsigned char header[])
{
    if (fread(header, 1, 10, fp) != 10)
    {
        printf("Unable to read ID3 header\n");
        return 0;
    }

    return 1;
}


/* =========================================================
   VALIDATE ID3 HEADER
   ========================================================= */

int validate_id3_header(unsigned char header[])
{
    /*
     * First 3 bytes must be:
     *
     * I D 3
     */

    if (header[0] != 'I' ||
        header[1] != 'D' ||
        header[2] != '3')
    {
        return 0;
    }


    /*
     * This project supports ID3v2.3.
     *
     * Byte 3 = major version
     * Byte 4 = revision
     */

    if (header[3] != 3)
    {
        printf("Unsupported ID3 version: 2.%u\n",
               header[3]);

        return 0;
    }


    return 1;
}


/* =========================================================
   GET ID3 TAG SIZE
   ========================================================= */

unsigned int get_tag_size(unsigned char header[])
{
    unsigned int tag_size;

    /*
     * ID3 tag size is a 28-bit synchsafe integer.
     *
     * Every byte uses only 7 bits.
     */

    tag_size = ((unsigned int)(header[6] & 0x7F) << 21) |
               ((unsigned int)(header[7] & 0x7F) << 14) |
               ((unsigned int)(header[8] & 0x7F) << 7)  |
               (unsigned int)(header[9] & 0x7F);

    return tag_size;
}


/* =========================================================
   SAFE TEXT COPY
   ========================================================= */

void copy_text(char destination[],
               size_t destination_size,
               unsigned char source[],
               unsigned int size)
{
    unsigned int i;
    unsigned int dest_index = 0;


    if (destination_size == 0)
    {
        return;
    }


    destination[0] = '\0';


    /*
     * Text frames contain:
     *
     * byte 0 -> encoding
     * byte 1 onwards -> actual text
     */

    if (size <= 1)
    {
        return;
    }


    for (i = 1;
         i < size && dest_index < destination_size - 1;
         i++)
    {
        /*
         * Stop at NULL terminator.
         */

        if (source[i] == '\0')
        {
            break;
        }

        destination[dest_index++] =
            (char)source[i];
    }


    destination[dest_index] = '\0';
}


/* =========================================================
   FIND SINGLE BYTE TERMINATOR
   ========================================================= */

static unsigned int find_null_position(unsigned char data[],
                                       unsigned int start,
                                       unsigned int size)
{
    unsigned int i;

    for (i = start; i < size; i++)
    {
        if (data[i] == '\0')
        {
            return i;
        }
    }

    return size;
}


/* =========================================================
   PROCESS TXXX FRAME
   ========================================================= */
/* =========================================================
   PROCESS TXXX FRAME
   ========================================================= */

static void process_txxx(unsigned char frame_data[],
                         unsigned int frame_size,
                         MP3Tags *tags)
{
    unsigned int i;
    unsigned int pos;
    unsigned int value_start;
    unsigned int out = 0;

    /*
     * TXXX structure:
     *
     * Encoding
     * Description
     * NULL
     * Value
     *
     * Our test file uses:
     *
     * Encoding = 1 (UTF-16)
     */

    if (frame_size <= 1)
    {
        return;
    }

    tags->txxx[0] = '\0';

    /*
     * -------------------------------------------------
     * UTF-16
     * -------------------------------------------------
     */

    if (frame_data[0] == 1 || frame_data[0] == 2)
    {
        /*
         * UTF-16 description starts at byte 1.
         *
         * Skip BOM:
         *
         * FF FE = UTF-16 little endian
         * FE FF = UTF-16 big endian
         */

        pos = 1;

        if (pos + 1 < frame_size &&
            ((frame_data[pos] == 0xFF &&
              frame_data[pos + 1] == 0xFE) ||
             (frame_data[pos] == 0xFE &&
              frame_data[pos + 1] == 0xFF)))
        {
            pos += 2;
        }

        /*
         * Find UTF-16 NULL terminator.
         *
         * UTF-16 NULL = 00 00
         */

        while (pos + 1 < frame_size)
        {
            if (frame_data[pos] == 0x00 &&
                frame_data[pos + 1] == 0x00)
            {
                break;
            }

            pos += 2;
        }

        /*
         * Description ended here.
         *
         * Skip UTF-16 NULL terminator.
         */

        if (pos + 1 >= frame_size)
        {
            return;
        }

        value_start = pos + 2;

        /*
         * Skip BOM of the value if present.
         */

        if (value_start + 1 < frame_size &&
            ((frame_data[value_start] == 0xFF &&
              frame_data[value_start + 1] == 0xFE) ||
             (frame_data[value_start] == 0xFE &&
              frame_data[value_start + 1] == 0xFF)))
        {
            value_start += 2;
        }

        /*
         * Convert UTF-16 ASCII-range characters
         * into normal C string.
         *
         * This is enough for the current MP3,
         * whose characters are in the ASCII range.
         */

        i = value_start;

        while (i + 1 < frame_size &&
               out < MAX_TXXX - 1)
        {
            unsigned short unicode_value;

            if (frame_data[i] == 0x00 &&
                frame_data[i + 1] == 0x00)
            {
                break;
            }

            if (frame_data[0] == 1)
            {
                /*
                 * UTF-16 little endian
                 */

                unicode_value =
                    (unsigned short)frame_data[i] |
                    ((unsigned short)frame_data[i + 1] << 8);
            }
            else
            {
                /*
                 * UTF-16 big endian
                 */

                unicode_value =
                    ((unsigned short)frame_data[i] << 8) |
                    (unsigned short)frame_data[i + 1];
            }

            /*
             * For normal printable ASCII characters,
             * store directly.
             */

            if (unicode_value >= 32 &&
                unicode_value <= 126)
            {
                tags->txxx[out++] =
                    (char)unicode_value;
            }
            else if (unicode_value == 10 ||
                     unicode_value == 13)
            {
                tags->txxx[out++] =
                    (char)unicode_value;
            }
            else
            {
                /*
                 * For characters outside ASCII,
                 * use '?' rather than corrupting memory.
                 */

                tags->txxx[out++] = '?';
            }

            i += 2;
        }

        tags->txxx[out] = '\0';

        return;
    }

    /*
     * -------------------------------------------------
     * ISO-8859-1 / UTF-8 style TXXX
     * -------------------------------------------------
     */

    pos = 1;

    while (pos < frame_size &&
           frame_data[pos] != '\0')
    {
        pos++;
    }

    if (pos >= frame_size)
    {
        return;
    }

    value_start = pos + 1;

    out = 0;

    while (value_start < frame_size &&
           frame_data[value_start] != '\0' &&
           out < MAX_TXXX - 1)
    {
        tags->txxx[out++] =
            (char)frame_data[value_start++];

    }

    tags->txxx[out] = '\0';
}

static void process_comm(unsigned char frame_data[],
                         unsigned int frame_size,
                         MP3Tags *tags)
{
    unsigned int pos;
    unsigned int comment_start;
    unsigned int i = 0;


    /*
     * COMM structure:
     *
     * Encoding      1 byte
     * Language      3 bytes
     * Description   variable
     * NULL
     * Comment       variable
     */

    if (frame_size <= 4)
    {
        return;
    }


    pos = 4;


    /*
     * Find description terminator.
     *
     * This handles the common ISO-8859-1 / UTF-8
     * representation where NULL is one byte.
     */

    pos = find_null_position(frame_data,
                             pos,
                             frame_size);


    if (pos >= frame_size)
    {
        return;
    }


    comment_start = pos + 1;


    while (comment_start < frame_size &&
           i < MAX_COMMENT - 1)
    {
        if (frame_data[comment_start] == '\0')
        {
            break;
        }

        tags->comment[i++] =
            (char)frame_data[comment_start++];

    }


    tags->comment[i] = '\0';
}


/* =========================================================
   PROCESS FRAME
   ========================================================= */

void process_frame(char frame_id[],
                   unsigned char frame_data[],
                   unsigned int frame_size,
                   MP3Tags *tags)
{
    if (strcmp(frame_id, "TIT2") == 0)
    {
        copy_text(tags->title,
                  sizeof(tags->title),
                  frame_data,
                  frame_size);
    }

    else if (strcmp(frame_id, "TPE1") == 0)
    {
        copy_text(tags->artist,
                  sizeof(tags->artist),
                  frame_data,
                  frame_size);
    }

    else if (strcmp(frame_id, "TALB") == 0)
    {
        copy_text(tags->album,
                  sizeof(tags->album),
                  frame_data,
                  frame_size);
    }

    else if (strcmp(frame_id, "TRCK") == 0)
    {
        copy_text(tags->track,
                  sizeof(tags->track),
                  frame_data,
                  frame_size);
    }

    else if (strcmp(frame_id, "TYER") == 0)
    {
        copy_text(tags->year,
                  sizeof(tags->year),
                  frame_data,
                  frame_size);
    }

    else if (strcmp(frame_id, "TDRC") == 0)
    {
        copy_text(tags->recording_time,
                  sizeof(tags->recording_time),
                  frame_data,
                  frame_size);
    }

    else if (strcmp(frame_id, "TCON") == 0)
    {
        copy_text(tags->genre,
                  sizeof(tags->genre),
                  frame_data,
                  frame_size);
    }

    else if (strcmp(frame_id, "COMM") == 0)
    {
        process_comm(frame_data,
                     frame_size,
                     tags);
    }

    else if (strcmp(frame_id, "TXXX") == 0)
    {
        process_txxx(frame_data,
                     frame_size,
                     tags);
    }

    else if (strcmp(frame_id, "TSSE") == 0)
    {
        copy_text(tags->encoder,
                  sizeof(tags->encoder),
                  frame_data,
                  frame_size);
    }
}


/* =========================================================
   READ ALL FRAMES
   ========================================================= */

void read_frames(FILE *fp,
                 long tag_end,
                 MP3Tags *tags)
{
    char frame_id[5];

    unsigned char size[4];
    unsigned char flags[2];

    unsigned int frame_size;

    long current_position;


    while (1)
    {
        current_position = ftell(fp);


        if (current_position < 0)
        {
            break;
        }


        /*
         * Need at least 10 bytes for a frame header.
         */

        if (current_position + 10 > tag_end)
        {
            break;
        }


        /*
         * Read frame ID.
         */

        if (fread(frame_id, 1, 4, fp) != 4)
        {
            break;
        }

        frame_id[4] = '\0';


        /*
         * Zero-filled padding means
         * there are no more frames.
         */

        if (frame_id[0] == '\0')
        {
            break;
        }


        /*
         * Read frame size.
         */

        if (fread(size, 1, 4, fp) != 4)
        {
            break;
        }


        /*
         * ID3v2.3 frame size = normal
         * big-endian 32-bit integer.
         */

        frame_size = ((unsigned int)size[0] << 24) |
                     ((unsigned int)size[1] << 16) |
                     ((unsigned int)size[2] << 8)  |
                     (unsigned int)size[3];


        /*
         * Read flags.
         */

        if (fread(flags, 1, 2, fp) != 2)
        {
            break;
        }


        printf("Frame ID   : %s\n",
               frame_id);

       // printf("Frame Size : %u bytes\n",
              // frame_size);


        /*
         * Prevent unreasonable memory allocation.
         */

        if (frame_size > MAX_FRAME_SIZE)
        {
            printf("Frame too large. Stopping.\n");
            break;
        }


        /*
         * Make sure frame doesn't go
         * beyond the ID3 tag.
         */

        if (ftell(fp) < 0 ||
            ftell(fp) + frame_size > tag_end)
        {
            printf("Invalid frame size\n");
            break;
        }


        /*
         * Zero-size frame.
         */

        if (frame_size == 0)
        {
            continue;
        }


        unsigned char *frame_data =
            malloc(frame_size);


        if (frame_data == NULL)
        {
            printf("Memory allocation failed\n");
            break;
        }


        if (fread(frame_data,
                  1,
                  frame_size,
                  fp) != frame_size)
        {
            free(frame_data);
            break;
        }


        /*
         * Interpret the frame.
         */

        process_frame(frame_id,
                      frame_data,
                      frame_size,
                      tags);


        free(frame_data);
    }
}


/* =========================================================
   DISPLAY TAGS
   ========================================================= */

/* =========================================================
   DISPLAY TAGS
   ========================================================= */

void display_tags(MP3Tags *tags)
{
    printf("\n");
    printf("----------------------------------------\n");
    printf("             ID3 V2.3 TAGS\n");
    printf("----------------------------------------\n");

    printf("Title   : %s\n",
           tags->title);

    printf("Artist  : %s\n",
           tags->artist);

    printf("Album   : %s\n",
           tags->album);

    printf("Year    : %s\n",
           tags->year);

    printf("Track   : %s\n",
           tags->track);

    printf("Genre   : %s\n",
           tags->genre);

    printf("Comment : %s\n",
           tags->comment);

    printf("\n");

    /*
     * Additional information supported
     * by our project.
     */

    if (tags->txxx[0] != '\0')
    {
        printf("TXXX    : %s\n",
               tags->txxx);
    }

    if (tags->recording_time[0] != '\0')
    {
        printf("TDRC    : %s\n",
               tags->recording_time);
    }

    if (tags->encoder[0] != '\0')
    {
        printf("Encoder : %s\n",
               tags->encoder);
    }

    printf("----------------------------------------\n");
}