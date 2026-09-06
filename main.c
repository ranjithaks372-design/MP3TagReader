#include "mp3_tag_reader.h"

#include <stdio.h>
#include <string.h>


/* =========================================================
   CHECK MP3 EXTENSION
   ========================================================= */

static int is_mp3_file(char filename[])
{
    const char *extension;

    extension =
        strrchr(filename, '.');


    if (extension == NULL)
    {
        return 0;
    }


    if (strcmp(extension,
               ".mp3") == 0 ||
        strcmp(extension,
               ".MP3") == 0)
    {
        return 1;
    }


    return 0;
}


/* =========================================================
   HELP
   ========================================================= */

void display_help(void)
{
    printf("\n");
    printf("========================================\n");
    printf("             MP3 TAG READER\n");
    printf("========================================\n");


    printf("\nUsage:\n\n");


    printf("View tags:\n");
    printf("./mp3_tag_reader input.mp3\n");


    printf("\nEdit tags:\n");

    printf("./mp3_tag_reader -t \"Title\" input.mp3 output.mp3\n");
    printf("./mp3_tag_reader -T \"Track\" input.mp3 output.mp3\n");
    printf("./mp3_tag_reader -a \"Artist\" input.mp3 output.mp3\n");
    printf("./mp3_tag_reader -A \"Album\" input.mp3 output.mp3\n");
    printf("./mp3_tag_reader -y \"Year\" input.mp3 output.mp3\n");
    printf("./mp3_tag_reader -c \"Comment\" input.mp3 output.mp3\n");
    printf("./mp3_tag_reader -g \"Genre\" input.mp3 output.mp3\n");


    printf("\nOptions:\n");

    printf("-t  Modify Title\n");
    printf("-T  Modify Track\n");
    printf("-a  Modify Artist\n");
    printf("-A  Modify Album\n");
    printf("-y  Modify Year\n");
    printf("-c  Modify Comment\n");
    printf("-g  Modify Genre\n");

    printf("-h  Display Help\n");
    printf("-v  Display Version\n");


    printf("\nSupported ID3 version: ID3v2.3\n");

    printf("\n");
}


/* =========================================================
   VERSION
   ========================================================= */

void display_version(void)
{
    printf("MP3 Tag Reader Version 1.0\n");
    printf("ID3v2.3 support\n");
}


/* =========================================================
   MAIN
   ========================================================= */

int main(int argc,
         char *argv[])
{
    unsigned char header[10];

    FILE *fp;

    unsigned int tag_size;

    long tag_end;

    MP3Tags tags = {0};


    /*
     * -----------------------------------------
     * No arguments
     * -----------------------------------------
     */

    if (argc == 1)
    {
        printf("\nMP3 Tag Reader\n");
        printf("Use -h for help\n");

        return 0;
    }


    /*
     * -----------------------------------------
     * HELP
     * -----------------------------------------
     */

    if (strcmp(argv[1],
               "-h") == 0)
    {
        if (argc != 2)
        {
            printf("Invalid arguments for -h\n");
            return 0;
        }

        display_help();

        return 0;
    }


    /*
     * -----------------------------------------
     * VERSION
     * -----------------------------------------
     */

    /*
 * -----------------------------------------
 * VERSION / VIEW
 * -----------------------------------------
 *
 * -v
 *      Display program version
 *
 * -v input.mp3
 *      Display MP3 tag information
 */

if (strcmp(argv[1], "-v") == 0)
{
    /*
     * Only -v
     */
    if (argc == 2)
    {
        display_version();

        return 0;
    }


    /*
     * -v input.mp3
     */
    if (argc == 3)
    {
        if (!is_mp3_file(argv[2]))
        {
            printf("Input file must have .mp3 extension.\n");

            return 0;
        }


        fp = fopen(argv[2], "rb");

        if (fp == NULL)
        {
            printf("Unable to open file: %s\n",
                   argv[2]);

            return 0;
        }


        if (!read_id3_header(fp, header))
        {
            fclose(fp);

            return 0;
        }


        if (!validate_id3_header(header))
        {
            printf("Invalid or unsupported ID3 file.\n");

            fclose(fp);

            return 0;
        }


        tag_size = get_tag_size(header);

        tag_end = 10 + tag_size;


        printf("\n");
        printf("========================================\n");
        printf("        Welcome to MP3 Tag Reader\n");
        printf("========================================\n");

        printf("\nFile : %s\n", argv[2]);

        printf("ID3 Version : 2.3\n");


        if (fseek(fp, 10, SEEK_SET) != 0)
        {
            fclose(fp);

            return 0;
        }


        read_frames(fp,
                    tag_end,
                    &tags);


        display_tags(&tags);


        fclose(fp);

        return 0;
    }


    printf("Usage: ./mp3_tag_reader -v [input.mp3]\n");

    return 0;
}


    /*
     * -----------------------------------------
     * EDIT OPTIONS
     * -----------------------------------------
     */

    if (strcmp(argv[1], "-t") == 0 ||
        strcmp(argv[1], "-T") == 0 ||
        strcmp(argv[1], "-a") == 0 ||
        strcmp(argv[1], "-A") == 0 ||
        strcmp(argv[1], "-y") == 0 ||
        strcmp(argv[1], "-c") == 0 ||
        strcmp(argv[1], "-g") == 0)
    {
        char *target_id = NULL;


        /*
         * Expected:
         *
         * argv[0] = program
         * argv[1] = option
         * argv[2] = new value
         * argv[3] = input file
         * argv[4] = output file
         */

        if (argc != 5)
        {
            printf("\nInvalid arguments.\n");

            display_help();

            return 0;
        }


        /*
         * Input must be MP3.
         */

        if (!is_mp3_file(argv[3]))
        {
            printf("Input file must have .mp3 extension.\n");

            return 0;
        }


        /*
         * Output must be MP3.
         */

        if (!is_mp3_file(argv[4]))
        {
            printf("Output file must have .mp3 extension.\n");

            return 0;
        }


        /*
         * Do not allow same input and output file.
         */

        if (strcmp(argv[3],
                   argv[4]) == 0)
        {
            printf("Input and output files must be different.\n");

            return 0;
        }


        /*
         * Convert command-line option
         * into ID3 frame ID.
         */

        if (strcmp(argv[1],
                   "-t") == 0)
        {
            target_id = "TIT2";
        }

        else if (strcmp(argv[1],
                        "-T") == 0)
        {
            target_id = "TRCK";
        }

        else if (strcmp(argv[1],
                        "-a") == 0)
        {
            target_id = "TPE1";
        }

        else if (strcmp(argv[1],
                        "-A") == 0)
        {
            target_id = "TALB";
        }

        else if (strcmp(argv[1],
                        "-y") == 0)
        {
            target_id = "TYER";
        }

        else if (strcmp(argv[1],
                        "-c") == 0)
        {
            target_id = "COMM";
        }

        else if (strcmp(argv[1],
                        "-g") == 0)
        {
            target_id = "TCON";
        }


        /*
         * Perform modification.
         */

        return edit_mp3(argv[3],
                        argv[4],
                        target_id,
                        argv[2]);
    }


    /*
     * -----------------------------------------
     * VIEW OPERATION
     * -----------------------------------------
     *
     * ./mp3_tag_reader song.mp3
     */

    if (argc == 2)
{
    /*
     * Check whether the argument is a valid option.
     */

    if (argv[1][0] == '-')
    {
        printf("Invalid option: %s\n", argv[1]);
        printf("Use -h for help.\n");

        return 0;
    }

    /*
     * Validate extension.
     */

    if (!is_mp3_file(argv[1]))
        {
            printf("Input file must have .mp3 extension.\n");

            return 0;
        }


        /*
         * Open MP3.
         */

        fp =
            fopen(argv[1],
                  "rb");


        if (fp == NULL)
        {
            printf("Unable to open file: %s\n",
                   argv[1]);

            return 0;
        }


        /*
         * Read ID3 header.
         */

        if (!read_id3_header(fp,
                             header))
        {
            fclose(fp);

            return 0;
        }


        /*
         * Validate ID3.
         */

        if (!validate_id3_header(header))
        {
            printf("Invalid or unsupported ID3 file.\n");

            fclose(fp);

            return 0;
        }


        /*
         * Get tag size.
         */

        tag_size =
            get_tag_size(header);


        tag_end =
            10 + tag_size;


        printf("\n");
        printf("ID3 Tag Size : %u bytes\n",
               tag_size);

        printf("ID3 Tag Ends : %ld\n",
               tag_end);


        /*
         * Move to first frame.
         */

        if (fseek(fp,
                  10,
                  SEEK_SET) != 0)
        {
            fclose(fp);

            return 0;
        }


        /*
         * Read all frames.
         */

        read_frames(fp,
                    tag_end,
                    &tags);


        /*
         * Display extracted information.
         */

        display_tags(&tags);


        fclose(fp);

        return 0;
    }


    /*
     * -----------------------------------------
     * INVALID COMMAND
     * -----------------------------------------
     */

    printf("\nInvalid command.\n");

    display_help();

    return 0;
}