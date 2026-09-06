# MP3 Tag Reader

## Project Description

MP3 Tag Reader is a command-line application developed in C to read and modify ID3v2.3 metadata stored in MP3 files.

The project demonstrates binary file handling, structures, pointers, dynamic memory allocation, command-line arguments, string handling, and ID3 tag/frame parsing.

## Features

### View MP3 Metadata

The program can read and display:

- Title
- Artist
- Album
- Year
- Track
- Genre
- Comment
- Recording Time
- Encoder
- User-defined Text (TXXX)

### Modify MP3 Metadata

The following tags can be modified:

- Title
- Artist
- Album
- Year
- Track
- Genre
- Comment

## Technologies Used

- C Programming
- File Handling
- Binary File Handling
- Structures
- Pointers
- Dynamic Memory Allocation
- Command-Line Arguments
- String Handling
- Bitwise Operations
- ID3v2.3 Tag Parsing

## Compilation

```bash
gcc -Wall -Wextra -std=c11 main.c mp3_tag_reader.c edit.c -o mp3_tag_reader
Display Help
./mp3_tag_reader -h
Display Version
./mp3_tag_reader -v
View MP3 Tags
./mp3_tag_reader test3.mp3
Modify Title
./mp3_tag_reader -t "Ranju_collection" test3.mp3 title_test.mp3
Modify Artist
./mp3_tag_reader -a "Ranjitha_KS" test3.mp3 artist_test.mp3
Modify Album
./mp3_tag_reader -A "Hanu_Man_Updated" test3.mp3 album_test.mp3
Modify Year
./mp3_tag_reader -y "2026" test3.mp3 year_test.mp3
Modify Track
./mp3_tag_reader -T "7" test3.mp3 track_test.mp3
Modify Genre
./mp3_tag_reader -g "Kannada" test3.mp3 genre_test.mp3
Modify Comment
./mp3_tag_reader -c "Modified by Ranjitha" test3.mp3 comment_test.mp3
Supported ID3 Frames
Frame	Metadata
TIT2	Title
TPE1	Artist
TALB	Album
TYER	Year
TRCK	Track
TCON	Genre
COMM	Comment
TDRC	Recording Time
TXXX	User-defined Text
TSSE	Encoder
Project Structure
MP3TagReader/
├── main.c
├── mp3_tag_reader.c
├── mp3_tag_reader.h
├── edit.c
├── README.md
└── Output_snaps/
Limitations
Currently supports ID3v2.3.
UTF-16 COMM comment editing is not supported.
The project focuses on ID3 metadata rather than MPEG audio properties such as bitrate and duration.
Author

Ranjitha K S
