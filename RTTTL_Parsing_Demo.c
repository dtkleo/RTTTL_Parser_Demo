/*
 * RTTTL_Parsing_Demo.c
 *
 * Author: David Leo
 */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// RTTTL note frequencies for frequencies A0 to C8 with A4 being 440 Hz
// Adapated from: https://en.wikipedia.org/wiki/Piano_key_frequencies
uint16_t frequencies[] = {0, 28, 29, 31, 33, 35, 37, 39, 41, 44, 46, 49, 52, 55, \
	58, 62, 65, 69, 73, 78, 82, 87, 92, 98, 104, 110, 117, 123, 131, 139, 147, \
	156, 165, 175, 185, 196, 208, 220, 233, 247, 262, 277, 294, 311, 330, 349, \
	370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831, \
	880, 932, 988, 1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, \
	1865, 1976, 2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, \
	3729, 3951, 4186};

// A character array for RTTTL song
// Adapted from: http://arcadetones.emuunlim.com
const char song_1[] = "pacman:d=4,o=5,b=112:32b,32p,32b6,32p,32f#6,32p,\
32d#6,32p,32b6,32f#6,16p,16d#6,16p,32c6,32p,32c7,32p,32g6,32p,32e6,32p,32c7,\
32g6,16p,16e6,16p,32b,32p,32b6,32p,32f#6,32p,32d#6,32p,32b6,32f#6,16p,16d#6,\
16p,32d#6,32e6,32f6,32p,32f6,32f#6,32g6,32p,32g6,32g#6,32a6,32p,32b.6";

// A character array for RTTTL song
// Adapted from: http://arcadetones.emuunlim.com
const char song_2[] = "pacinter:d=4,o=5,b=160:8f#6,8f#6,8f#6,16d#6,16c#6,\
16f#6,8f#6,8a.6,8p,8f#6,8f#6,8f#6,16d#6,16c#6,16f#6,8f#6,8d#.6,8p,8f#6,8f#6,\
8f#6,16d#6,16c#6,16f#6,8f#6,8a6,8b6,8c7,8b6,8a6,8f#6,8a.6,8f#6,8p,8f#6,8f#6,\
8f#6,16d#6,16c#6,16f#6,8f#6,8a.6,8p,8f#6,8f#6,8f#6,16d#6,16c#6,16f#6,8f#6,\
8d#.6,8p,8f#6,8f#6,8f#6,16d#6,16c#6,16f#6,8f#6,8a6,8b6,8c7,8b6,8a6,8f#6,\
8a.6,8f#6";

// A character array for RTTTL song
// Adapted from: http://arcadetones.emuunlim.com
const char song_3[] = "smbdeath:d=4,o=5,b=90:32c6,32c6,32c6,8p,16b,16f6,16p,\
16f6,16f.6,16e.6,16d6,16c6,16p,16e,16p,16c";

// Default settings for each song are stored in the RTTTL header
// and within the song_structure.
struct song_structure {
    // Pointer to current song
    char * pointer;
    // Default duration of a note in the musical score in whole notes
    // This is stored as a 8 bit unsigned integer where (1<<7) represents
    // a whole note; (1<<6) a half note and (1<<5) a quarter note etc.
    uint8_t default_duration;
    // Default octave of a note in the musical score
    uint8_t default_octave;
    // Default beats per minute of the musical score
    uint8_t default_bpm;
    // Index of the first note in the musical score
    uint8_t index_of_first_note;
    // Total current_position of notes in the musical score
    uint8_t total_number_of_notes;
    // Floating number of milli seconds for the shortest possible note
    float ms_measure;
};
struct note_structure {
    // The note's current position in the song.total_number_of_notes
    uint8_t current_position;
    // The number of milliseconds the current note is required to play
    uint8_t ms_duration;
    // The frequency of the current note
    uint16_t frequency;
    // The note name and it's modifier if applicable
    char name[2];
    // The note.location in the song array where the note's information begins
    uint16_t location;
    // The octave of the current note
    uint8_t octave;
};

// Musical structures
struct song_structure song;
struct note_structure note;

//////////////////////////////////////////////////////////////////////////
// Header
//

char *ptr_baftera(const char *str, uint16_t a, char b);
uint16_t index_baftera(const char *str, uint16_t a, char b);
uint8_t reverse_byte(uint8_t byte);
float determine_ms_measure(void);

//////////////////////////////////////////////////////////////////////////
// Private functions
//

// Returns pointer immediately after the first char b found after a in str.
char *ptr_baftera(const char *str, uint16_t a, char b) {
    char *ptr;
    do {
        a++;
    } while (str[a] != b);
    ptr = (char *) &str[a+1];
    return ptr;
}

// Returns the current_position immediately after the first char b found after a in str.
uint16_t index_baftera(const char *str, uint16_t a, char b) {
    do {
        a++;
    } while (str[a] != b);
    return a + 1;
}

// Returns the reversed byte of a given unsigned integer
uint8_t reverse_byte(uint8_t byte) {
    uint8_t result = 0;
    for (int i = 0; i <= 7; i++) {
        if (byte & (1<<i)) {
            result |= (1<<(7-i));
        }
    }
    return result;
}

// Determine the smallest measure of time in milli seconds given the
// song.default_bpm as a float
float determine_ms_measure(void) {
    return (float) ((60000) / (512 * (float) (song.default_bpm)));
}

// Get the note information given it's position within the song
void set_note_information(uint8_t number) {
    note.current_position = number;
    // The first note's location should already be stored
    if (number == 1) {
        note.location = song.index_of_first_note;
    } else {
        // The each note is separated using a comma and so counting the number
        // of commas from the first note gives the note's location.
        int j = number - 1;
        for (int i = song.index_of_first_note; j != 0; note.location = i) {
            i = index_baftera(song.pointer, i, ',');
            j -= 1;
        }
    }

    // Note duration information is located given immediately following the
    // comma unless it is of default length
    uint8_t d = atoi(&(song.pointer[note.location]));
    if (d != 0) {
        note.ms_duration =  reverse_byte(d) * song.ms_measure;
    } else {
        note.ms_duration = song.default_duration * song.ms_measure;
    }

    // Locate and set the note letter and modifier if present
    int y = note.location;
    do {
        y += 1;
    } while (!(isalpha(song.pointer[y])));
    // The note letter is at location y in the song array
    note.name[0] = toupper(song.pointer[y]);
    // The next char after the note letter
    if (song.pointer[y + 1] == '#' || song.pointer[y + 1] == '.') {
        note.name[1] = song.pointer[y + 1];
        note.octave = atoi(&song.pointer[y + 2]);
        if (song.pointer[y + 2] == '.') {
            note.ms_duration += (note.ms_duration / 2);
            note.octave = atoi(&song.pointer[y + 3]);
        }
    } else {
        note.name[1] ='\0';
        note.octave = atoi(&song.pointer[y + 1]);
    }
    // Apply the default octave is none is given
    if (note.octave == 0) {
        note.octave = song.default_octave;
    }

    // Determine the frequency of the note
    int scale = 0;
    switch (note.name[0]) {
        case 'B':
            scale = 12;
            break;
        case 'A':
            scale = 10;
            break;
        case 'G':
            scale = 8;
            break;
        case 'F':
            scale = 6;
            break;
        case 'E':
            scale = 5;
            break;
        case 'D':
            scale = 3;
            break;
        case 'C':
            scale = 1;
            break;
        default:
            scale = 0;
            break;
    }

    // Apply the note modifiers
    switch (note.name[1]) {
        case '#':
            scale += 1;
            break;
        case '.':
            note.ms_duration += (note.ms_duration / 2);
            break;
        default:
            break;
    }

    // Set the note frequency according to the supplied frequency array
    if (scale != 0) {
        note.frequency = frequencies[scale + (12 * note.octave) - 9];
    } else {
        note.frequency = 0;
    }
}

//////////////////////////////////////////////////////////////////////////
// Public functions
//

// Set the song information given the track number
void set_song(uint8_t track) {
    int j = 0;
    int k = 0;

    // Select the correct song
    switch (track) {
        case 1:
            song.pointer = (char *) song_1;
            j = sizeof(song_1);
            break;
        case 2:
            song.pointer  = (char *) song_2;
            j = sizeof(song_2);
            break;
        case 3:
            song.pointer  = (char *) song_3;
            j = sizeof(song_3);
            break;
    }

    // Count the current_position of notes in the selected song
    for (int i = 0; i < j; i++) {
        if (song.pointer [i] == ',') {
            song.total_number_of_notes += 1;
        }
    }
    song.total_number_of_notes -= 1;  // To adjust for the song information

    // Song information is extracted from the RTTTL header
    song.default_duration = reverse_byte(atoi((ptr_baftera(song.pointer , 0, '='))));
    k = index_baftera(song.pointer , 0, '=');
    song.default_octave = atoi((ptr_baftera(song.pointer , k, '=')));
    k = index_baftera(song.pointer , k, '=');
    song.default_bpm = atoi((ptr_baftera(song.pointer , k, '=')));
    song.index_of_first_note = index_baftera(song.pointer , k, ':');

    // When the song is first loaded the note current_position is set to 0
    note.current_position = 0;
    // Determine the milli seconds for each measure based on song information
    song.ms_measure = determine_ms_measure();
}

// Plays the note as required
void play_note(void) {
//        disable_sounds();
    if (note.current_position == 0 ||
            note.current_position == song.total_number_of_notes) {
        note.current_position = 1;
        set_note_information(1);
    } else {
        // The next note is set to play
        set_note_information(note.current_position + 1);
//        enable_sounds();
    }

}

//////////////////////////////////////////////////////////////////////////
// Main
//
int main() {

    // Set song to track 2
    set_song(2);
    // Set up the 67th note in song 2
    set_note_information(67);

    // RTTTL Parsing demo output
    printf("------------------------\n");
    printf("RTTTL Parsing\n");
    printf("------------------------\n");
    printf("song.default_duration = %d\n", song.default_duration);
    printf("song.default_octave = %d\n", song.default_octave);
    printf("song.default_bpm = %d\n", song.default_bpm);
    printf("song.index_of_first_note = %d\n", song.index_of_first_note);
    printf("song.total_number_of_notes = %d\n", song.total_number_of_notes);
    printf("song.ms_measure = %f\n", song.ms_measure);
    printf("------------------------\n");
    printf("note.location = %d\n", note.location);
    printf("note.ms_duration = %d\n", note.ms_duration);
    printf("note.octave = %d\n", note.octave);
    printf("note.frequency = %d\n", note.frequency);
    printf("note.name = %c%c\n", note.name[0],note.name[1]);
    printf("note.current_position = %d\n", note.current_position);
    printf("------------------------\n");

    return 0;
}