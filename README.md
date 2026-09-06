# MP3 Tag Editor

## Brief of the Project

**MP3 Tag Editor** is a command-line-based C project developed for **ID3v2.3 MP3 files**. The application supports reading and displaying MP3 metadata and modifying selected ID3 tag frames using **Command-Line Arguments (CLA)**.

The project demonstrates practical concepts such as file handling, binary file processing, structures, pointers, dynamic memory allocation, string handling, bitwise operations, and modular programming.

During modification, only the requested metadata is changed. The remaining metadata and audio data are preserved, and the modified content is written to a new MP3 file with the user-specified output filename.

## How the Project Works

### Display MP3 Metadata

The application reads and parses the ID3v2.3 tag information from an MP3 file and displays the available metadata, including:

- Title
- Artist
- Album
- Year
- Track
- Genre
- Comment
- Recording Time
- Encoder
- User-defined Text

### Modifying MP3 Tags

The application allows selected metadata fields to be modified using command-line arguments.

The required information is:

**Tag Data, Input MP3 File, Output MP3 File**

Example:

```bash
./mp3_tag_reader -a "New Artist" input.mp3 output.mp3
