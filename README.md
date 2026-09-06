# MP3 Tag Editor

## Brief of the Project

**MP3 Tag Editor** is a command-line-based C project developed specifically for **ID3v2.3 MP3 files**. The application supports reading and displaying MP3 metadata and modifying selected ID3 tag frames using **Command-Line Arguments (CLA)**.

The project demonstrates how MP3 metadata is stored in ID3v2.3 tags and how individual metadata frames can be read, processed, and modified using C programming concepts.

During modification, only the requested tag data is changed. The remaining metadata and audio data are preserved, and the modified data is written to a new MP3 file with the user-specified output filename.

## How the Project Works

### Display MP3 Metadata

The application reads and parses the ID3v2.3 tag information from the input MP3 file and displays the available metadata.

The application supports reading metadata such as:

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

For modifying a tag, the application takes the following information through command-line arguments:

**Tag Data, Input MP3 File, Output MP3 File**

Example:

```bash
./mp3_tag_reader -a "New Artist" input.mp3 output.mp3
