#ifndef DISPLAY_H
#define DISPLAY_H

// drawing mode
#define MODE_UNCHAINED  (0)

// screen stuff
#define NUM_FRAMES      (4)
#define SCREEN_WIDTH    (320)
#define SCREEN_HEIGHT   (200)
#define SCREEN_PIX      (SCREEN_WIDTH*SCREEN_HEIGHT)
#define FRAME_PIX       ((SCREEN_WIDTH*SCREEN_HEIGHT)>>2)

/* VGA sequence controller */
#define SC_INDEX      0x03c4
#define SC_DATA       0x03c5
#define MAP_MASK      0x02

/* VGA CRT controller */
#define CRTC_INDEX    0x03d4
#define CRTC_DATA     0x03d5

/* CRT controller registers */
#define HIGH_ADDRESS 0x0C
#define LOW_ADDRESS  0x0D
#define INPUT_STATUS_1      0x03da

/* VGA input status bits */
#define DISPLAY_ENABLE      0x01
#define VRETRACE            0x08

#define MEMORY_MODE   0x04
#define UNDERLINE_LOC 0x14
#define MODE_CONTROL  0x17

int display_text();
int display_vga();
int display_draw();

// screen frame_buffer
#if MODE_UNCHAINED
char frame_buffer[NUM_FRAMES][FRAME_PIX];
#else
char frame_buffer[SCREEN_PIX];
#endif

#endif
