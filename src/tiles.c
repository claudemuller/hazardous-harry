#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    const uint32_t vga_data_addr = 0x120f0;
    const uint32_t vga_pal_addr = 0x26b0a;

    char *exe = "res/DDAVE/DAVE.EXENEW";
    FILE *fin;
    fin = fopen(exe, "rb");
    if (!fin) {
        fprintf(stderr, "Error opening file %s\n", exe);
        return 1;
    }

    fseek(fin, vga_data_addr, SEEK_SET);

    // Get the file length from the first 4 bytes
    uint32_t final_len = 0;
    final_len |= fgetc(fin);
    final_len |= fgetc(fin) << 8;
    final_len |= fgetc(fin) << 16;
    final_len |= fgetc(fin) << 24;

    // Decode each byte
    uint32_t cur_len = 0;
    unsigned char out_data[150000];
    uint8_t byte_buf;

    memset(&out_data, 0, sizeof(out_data));

    while (cur_len < final_len) {
        byte_buf = fgetc(fin);
        if (byte_buf & 0x80) {
            byte_buf &= 0x7f;
            byte_buf++;
            while (byte_buf) {
                out_data[cur_len++] = fgetc(fin);
                byte_buf--;
            }
        } else {
            byte_buf += 3;
            char next = fgetc(fin);
            while (byte_buf) {
                out_data[cur_len++] = next;
                byte_buf--;
            }
        }
    }

    // Read 256 colours from 3 (RGB) bytes for VGA
    fseek(fin, vga_pal_addr, SEEK_SET);
    uint8_t palette[768];

    for (uint32_t i = 0; i < 256; i++) {
        for (int j = 0; j < 3; j++) {
            palette[i * 3 + j] = fgetc(fin);
            palette[i * 3 + j] <<= 2;
        }
    }

    fclose(fin);

    // Get the tile count from the first 4 bytes
    uint32_t tile_count = 0;
    tile_count |= out_data[3] << 24;
    tile_count |= out_data[2] << 16;
    tile_count |= out_data[1] << 8;
    tile_count |= out_data[0];
    printf("Tile count: %d\n", tile_count - 1);

    // Read in offest of each tile
    uint32_t tile_index[500] = {0};
    for (uint32_t i = 0; i < tile_count; i++) {
        tile_index[i] |= out_data[i * 4 + 4];
        tile_index[i] |= out_data[i * 4 + 5] << 8;
        tile_index[i] |= out_data[i * 4 + 6] << 16;
        tile_index[i] |= out_data[i * 4 + 7] << 24;
        // printf("Tile offset: %d\n", tile_index[i]);
    }

    // Last tile is EOF
    tile_index[tile_count - 1] = final_len;

    // Save each tile as its own file
    uint16_t tile_width;
    uint16_t tile_height;
    uint32_t cur_byte;
    uint32_t cur_tile_byte;
    uint8_t cur_tile;

    for (cur_tile = 0; cur_tile < tile_count; cur_tile++) {
        cur_tile_byte = 0;
        cur_byte = tile_index[cur_tile];
        tile_width = 16;
        tile_height = 16;

        // Skip weird byte
        if (cur_byte > 65280) {
            cur_byte++;
        }

        if (out_data[cur_byte + 1] == 0 && out_data[cur_byte + 3] == 0) {
            if (out_data[cur_byte] > 0 && out_data[cur_byte] < 0xbf && out_data[cur_byte + 2] > 0 &&
                out_data[cur_byte + 2] < 0x64) {
                tile_width = out_data[cur_byte];
                tile_height = out_data[cur_byte + 2];
                cur_byte += 4;
            }
        }

        uint8_t *dst_byte;
        SDL_Surface *surface = SDL_CreateRGBSurface(0, tile_width, tile_height, 32, 0, 0, 0, 0);
        dst_byte = (uint8_t *)surface->pixels;

        // Match bytes to palette and write to surface
        uint8_t src_byte;
        uint8_t r, g, b;

        for (; cur_byte < tile_index[cur_tile + 1]; cur_byte++) {
            src_byte = out_data[cur_byte];
            r = palette[src_byte * 3];
            g = palette[src_byte * 3 + 1];
            b = palette[src_byte * 3 + 2];

            dst_byte[cur_tile_byte * 4] = b;
            dst_byte[cur_tile_byte * 4 + 1] = g;
            dst_byte[cur_tile_byte * 4 + 2] = r;
            dst_byte[cur_tile_byte * 4 + 3] = 0xff;

            cur_tile_byte++;
        }

        char file_num[4] = {0};
        char fout[16] = {0};

        fout[0] = '\0';
        strncat(fout, "res/tile", sizeof(fout) - strlen(fout) - 1);
        snprintf(file_num, sizeof(file_num), "%u", cur_tile);
        strncat(fout, file_num, sizeof(fout) - strlen(fout) - 1);
        strncat(fout, ".bmp", sizeof(fout) - strlen(fout) - 1);
        printf("Writing out to: %s (%d x %d)\n", fout, tile_width, tile_height);

        SDL_SaveBMP(surface, fout);
        SDL_FreeSurface(surface);
    }

    return 0;
}
