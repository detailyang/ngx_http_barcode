/*
* @Author: detailyang
* @Date:   2016-05-18 08:53:55
* @Last Modified by:   detailyang
* @Last Modified time: 2016-05-18 16:49:44
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zint.h>
#include <png.h>


#define SSET    "0123456789ABCDEF"

/* structure to store PNG image bytes */
struct mem_encode
{
  char *buffer;
  size_t size;
};

struct mainprog_info_type {
    long width;
    long height;
    FILE *outfile;
    jmp_buf jmpbuf;
};

static void writepng_error_handler(png_structp png_ptr, png_const_charp msg)
{
    struct mainprog_info_type  *graphic;

    fprintf(stderr, "writepng libpng error: %s\n", msg);
    fflush(stderr);

    graphic = (struct mainprog_info_type*)png_get_error_ptr(png_ptr);
    if (graphic == NULL) {         /* we are completely hosed now */
        fprintf(stderr,
          "writepng severe error:  jmpbuf not recoverable; terminating.\n");
        fflush(stderr);
        return;
    }
    longjmp(graphic->jmpbuf, 1);
}

/* open output file with libpng */
// png_init_io(png_ptr, graphic->outfile);
struct mem_encode state = {
     NULL,
     0
};


void my_png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
  /* with libpng15 next line causes pointer deference error; use libpng12 */
  struct mem_encode* p=(struct mem_encode*)png_get_io_ptr(png_ptr); /* was png_ptr->io_ptr */
  size_t nsize = p->size + length;

  /* allocate or grow buffer */
  if(p->buffer)
    p->buffer = realloc(p->buffer, nsize);
  else
    p->buffer = malloc(nsize);

  if(!p->buffer)
    png_error(png_ptr, "Write Error");

  /* copy new bytes to end of buffer */
  memcpy(p->buffer + p->size, data, length);
  p->size += length;
}

int png_pixel_plot(struct zint_symbol *symbol, int image_height, int image_width, char *pixelbuf, int rotate_angle)
{
    struct mainprog_info_type wpng_info;
    struct mainprog_info_type *graphic;

    uint8_t outdata[image_width * 3];
    png_structp  png_ptr;
    png_infop  info_ptr;
    graphic = &wpng_info;
    uint8_t *image_data;
    int i, row, column, errn;
    int fgred, fggrn, fgblu, bgred, bggrn, bgblu;

    switch(rotate_angle) {
        case 0:
        case 180:
            graphic->width = image_width;
            graphic->height = image_height;
            break;
        case 90:
        case 270:
            graphic->width = image_height;
            graphic->height = image_width;
            break;
    }

    /* sort out colour options */
    to_upper((uint8_t*)symbol->fgcolour);
    to_upper((uint8_t*)symbol->bgcolour);

    if(strlen(symbol->fgcolour) != 6) {
        strcpy(symbol->errtxt, "Malformed foreground colour target");
        return ZERROR_INVALID_OPTION;
    }
    if(strlen(symbol->bgcolour) != 6) {
        strcpy(symbol->errtxt, "Malformed background colour target");
        return ZERROR_INVALID_OPTION;
    }
    errn = is_sane(SSET, (uint8_t*)symbol->fgcolour, strlen(symbol->fgcolour));
    if (errn == ZERROR_INVALID_DATA) {
        strcpy(symbol->errtxt, "Malformed foreground colour target");
        return ZERROR_INVALID_OPTION;
    }
    errn = is_sane(SSET, (uint8_t*)symbol->bgcolour, strlen(symbol->bgcolour));
    if (errn == ZERROR_INVALID_DATA) {
        strcpy(symbol->errtxt, "Malformed background colour target");
        return ZERROR_INVALID_OPTION;
    }

    fgred = (16 * ctoi(symbol->fgcolour[0])) + ctoi(symbol->fgcolour[1]);
    fggrn = (16 * ctoi(symbol->fgcolour[2])) + ctoi(symbol->fgcolour[3]);
    fgblu = (16 * ctoi(symbol->fgcolour[4])) + ctoi(symbol->fgcolour[5]);
    bgred = (16 * ctoi(symbol->bgcolour[0])) + ctoi(symbol->bgcolour[1]);
    bggrn = (16 * ctoi(symbol->bgcolour[2])) + ctoi(symbol->bgcolour[3]);
    bgblu = (16 * ctoi(symbol->bgcolour[4])) + ctoi(symbol->bgcolour[5]);

    /* Open output file in binary mode */
    if((symbol->output_options & BARCODE_STDOUT) != 0) {
        graphic->outfile = stdout;
    } else {
        if (!(graphic->outfile = fopen(symbol->outfile, "wb"))) {
            strcpy(symbol->errtxt, "Can't open output file");
            return ZERROR_FILE_ACCESS;
        }
    }

    /* Set up error handling routine as proc() above */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, graphic, writepng_error_handler, NULL);
    if (!png_ptr) {
        strcpy(symbol->errtxt, "Out of memory");
        return ZERROR_MEMORY;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, NULL);
        strcpy(symbol->errtxt, "Out of memory");
        return ZERROR_MEMORY;
    }

    /* catch jumping here */
    if (setjmp(graphic->jmpbuf)) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        strcpy(symbol->errtxt, "libpng error occurred");
        return ZERROR_MEMORY;
    }

    png_set_write_fn(png_ptr, &state, my_png_write_data, NULL);

    /* set compression */
    png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

    /* set Header block */
    png_set_IHDR(png_ptr, info_ptr, graphic->width, graphic->height,
             8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
       PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    /* write all chunks up to (but not including) first IDAT */
    png_write_info(png_ptr, info_ptr);

    /* set up the transformations:  for now, just pack low-bit-depth pixels
    into bytes (one, two or four pixels per byte) */
    png_set_packing(png_ptr);

    /* Pixel Plotting */

    switch(rotate_angle) {
        case 0: /* Plot the right way up */
            for(row = 0; row < image_height; row++) {
                for(column = 0; column < image_width; column++) {
                    i = column * 3;
                    switch(*(pixelbuf + (image_width * row) + column))
                    {
                        case '1':
                            outdata[i] = fgred;
                            outdata[i + 1] = fggrn;
                            outdata[i + 2] = fgblu;
                            break;
                        default:
                            outdata[i] = bgred;
                            outdata[i + 1] = bggrn;
                            outdata[i + 2] = bgblu;
                            break;

                    }
                }
                /* write row contents to file */
                image_data = outdata;
                png_write_row(png_ptr, image_data);
            }
            break;
        case 90: /* Plot 90 degrees clockwise */
            for(row = 0; row < image_width; row++) {
                for(column = 0; column < image_height; column++) {
                    i = column * 3;
                    switch(*(pixelbuf + (image_width * (image_height - column - 1)) + row))
                    {
                        case '1':
                            outdata[i] = fgred;
                            outdata[i + 1] = fggrn;
                            outdata[i + 2] = fgblu;
                            break;
                        default:
                            outdata[i] = bgred;
                            outdata[i + 1] = bggrn;
                            outdata[i + 2] = bgblu;
                            break;

                    }
                }

                /* write row contents to file */
                image_data = outdata;
                png_write_row(png_ptr, image_data);
            }
            break;
        case 180: /* Plot upside down */
            for(row = 0; row < image_height; row++) {
                for(column = 0; column < image_width; column++) {
                    i = column * 3;
                    switch(*(pixelbuf + (image_width * (image_height - row - 1)) + (image_width - column - 1)))
                    {
                        case '1':
                            outdata[i] = fgred;
                            outdata[i + 1] = fggrn;
                            outdata[i + 2] = fgblu;
                            break;
                        default:
                            outdata[i] = bgred;
                            outdata[i + 1] = bggrn;
                            outdata[i + 2] = bgblu;
                            break;

                    }
                }

                /* write row contents to file */
                image_data = outdata;
                png_write_row(png_ptr, image_data);
            }
            break;
        case 270: /* Plot 90 degrees anti-clockwise */
            for(row = 0; row < image_width; row++) {
                for(column = 0; column < image_height; column++) {
                    i = column * 3;
                    switch(*(pixelbuf + (image_width * column) + (image_width - row - 1)))
                    {
                        case '1':
                            outdata[i] = fgred;
                            outdata[i + 1] = fggrn;
                            outdata[i + 2] = fgblu;
                            break;
                        default:
                            outdata[i] = bgred;
                            outdata[i + 1] = bggrn;
                            outdata[i + 2] = bgblu;
                            break;

                    }
                }

                /* write row contents to file */
                image_data = outdata;
                png_write_row(png_ptr, image_data);
            }
            break;
    }

    /* End the file */
    png_write_end(png_ptr, NULL);

    printf("size %d", state.size);
    /* make sure we have disengaged */
    if (png_ptr && info_ptr) png_destroy_write_struct(&png_ptr, &info_ptr);
    if(symbol->output_options & BARCODE_STDOUT) {
        fflush(wpng_info.outfile);
    } else {
        fclose(wpng_info.outfile);
    }
    return 0;
}

int
save_png_to_file() {
    /* static */
    struct mem_encode state;

    /* initialise - put this before png_write_png() call */
    state.buffer = NULL;
    state.size = 0;
    png_structp png_ptr;

// extern PNG_EXPORT(void,png_set_write_fn) PNGARG((png_structp png_ptr,
//    png_voidp io_ptr, png_rw_ptr write_data_fn, png_flush_ptr output_flush_fn));
    /* if my_png_flush() is not needed, change the arg to NULL */
    png_set_write_fn(png_ptr, &state, my_png_write_data, NULL);
}

int validator(char test_string[], char source[])
{ /* Verifies that a string only uses valid characters */
    unsigned int i, j, latch;

    for(i = 0; i < strlen(source); i++) {
        latch = 0;
        for(j = 0; j < strlen(test_string); j++) {
            if (source[i] == test_string[j]) { latch = 1; } }
            if (!(latch)) {
                return ZERROR_INVALID_DATA; }
    }

    return 0;
}

int escape_char_process(struct zint_symbol *symbol, uint8_t input_string[], int length)
{
    int error_number;
    int i, j;
    uint8_t escaped_string[length + 1];

    i = 0;
    j = 0;

    do {
        if(input_string[i] == '\\') {
            switch(input_string[i + 1]) {
                case '0': escaped_string[j] = 0x00; i += 2; break; /* Null */
                case 'E': escaped_string[j] = 0x04; i += 2; break; /* End of Transmission */
                case 'a': escaped_string[j] = 0x07; i += 2; break; /* Bell */
                case 'b': escaped_string[j] = 0x08; i += 2; break; /* Backspace */
                case 't': escaped_string[j] = 0x09; i += 2; break; /* Horizontal tab */
                case 'n': escaped_string[j] = 0x0a; i += 2; break; /* Line feed */
                case 'v': escaped_string[j] = 0x0b; i += 2; break; /* Vertical tab */
                case 'f': escaped_string[j] = 0x0c; i += 2; break; /* Form feed */
                case 'r': escaped_string[j] = 0x0d; i += 2; break; /* Carriage return */
                case 'e': escaped_string[j] = 0x1b; i += 2; break; /* Escape */
                case 'G': escaped_string[j] = 0x1d; i += 2; break; /* Group Separator */
                case 'R': escaped_string[j] = 0x1e; i += 2; break; /* Record Separator */
                case '\\': escaped_string[j] = '\\'; i += 2; break;
                default: escaped_string[j] = input_string[i]; i++; break;
            }
        } else {
            escaped_string[j] = input_string[i];
            i++;
        }
        j++;
    } while (i < length);
    escaped_string[j] = '\0';

    error_number = ZBarcode_Encode(symbol, escaped_string, j);

    return error_number;
}

#define NESET "0123456789"

int main() {
    struct zint_symbol *symbol;
    int textdone, main_width, comp_offset, large_bar_count;
    char textpart[10], addon[6];
    float addon_text_posn, preset_height, large_bar_height;
    int i, r, textoffset, yoffset, xoffset, latch, image_width, image_height;
    char *pixelbuf;
    int addon_latch = 0, smalltext = 0;
    int this_row, block_width, plot_height, plot_yposn, textpos;
    float row_height, row_posn;
    int error_number;
    int default_text_posn;
    int next_yposn;
    symbol = ZBarcode_Create();
    symbol->input_mode = UNICODE_MODE;
    // symbol->height = 100;
    // symbol->border_width = 1000; // [1, 1000]
    // symbol->scale = 1;
    error_number = escape_char_process(symbol, "abcd", sizeof("abcd"));
    if(symbol->symbology == BARCODE_MAXICODE) {
        xoffset = symbol->border_width + symbol->whitespace_width;
        yoffset = symbol->border_width;
        image_width = 300 + (2 * xoffset * 2);
        image_height = 300 + (2 * yoffset * 2);
    } else {
        uint8_t local_text[ustrlen(symbol->text) + 1];

        if(symbol->show_hrt != 0) {
            to_latin1(symbol->text, local_text);
        } else {
            local_text[0] = '\0';
        }

        textdone = 0;
        main_width = symbol->width;
        strcpy(addon, "");
        comp_offset = 0;
        addon_text_posn = 0.0;
        row_height = 0;
        if(symbol->output_options & SMALL_TEXT) {
            smalltext = 1;
        }

        if (symbol->height == 0) {
            symbol->height = 50;
        }

        large_bar_count = 0;
        preset_height = 0.0;
        for(i = 0; i < symbol->rows; i++) {
            preset_height += symbol->row_height[i];
            if(symbol->row_height[i] == 0) {
                large_bar_count++;
            }
        }

        if (large_bar_count == 0) {
            symbol->height = preset_height;
            large_bar_height = 10;
        } else {
            large_bar_height = (symbol->height - preset_height) / large_bar_count;
        }

        while(!(module_is_set(symbol, symbol->rows - 1, comp_offset))) {
            comp_offset++;
        }

        /* Certain symbols need whitespace otherwise characters get chopped off the sides */
        if ((((symbol->symbology == BARCODE_EANX) && (symbol->rows == 1)) || (symbol->symbology == BARCODE_EANX_CC))
            || (symbol->symbology == BARCODE_ISBNX)) {
            switch(ustrlen(local_text)) {
                case 13: /* EAN 13 */
                case 16:
                case 19:
                    if(symbol->whitespace_width == 0) {
                        symbol->whitespace_width = 10;
                    }
                    main_width = 96 + comp_offset;
                    break;
                default:
                    main_width = 68 + comp_offset;
            }
        }

        if (((symbol->symbology == BARCODE_UPCA) && (symbol->rows == 1)) || (symbol->symbology == BARCODE_UPCA_CC)) {
            if(symbol->whitespace_width == 0) {
                symbol->whitespace_width = 10;
                main_width = 96 + comp_offset;
            }
        }

        if (((symbol->symbology == BARCODE_UPCE) && (symbol->rows == 1)) || (symbol->symbology == BARCODE_UPCE_CC)) {
            if(symbol->whitespace_width == 0) {
                symbol->whitespace_width = 10;
                main_width = 51 + comp_offset;
            }
        }

        latch = 0;
        r = 0;
        /* Isolate add-on text */
        if(is_extendable(symbol->symbology)) {
            for(i = 0; i < ustrlen(local_text); i++) {
                if (latch == 1) {
                    addon[r] = local_text[i];
                    r++;
                }
                if (symbol->text[i] == '+') {
                    latch = 1;
                }
            }
        }
        addon[r] = '\0';

        if(ustrlen(local_text) != 0) {
            textoffset = 9;
        } else {
            textoffset = 0;
        }
        xoffset = symbol->border_width + symbol->whitespace_width;
        yoffset = symbol->border_width;
        image_width = 2 * (symbol->width + xoffset + xoffset);
        image_height = 2 * (symbol->height + textoffset + yoffset + yoffset);
    }
    symbol->output_buffer = (char *)malloc(image_width * image_height);
    int rotate_angle = 0;
    // error_number = validator(NESET, optarg);
    // if (error_number == ZERROR_INVALID_DATA) {
    //     return 1;
    // }
    symbol->output_options += BARCODE_STDOUT;
    strncpy(symbol->outfile, "dummy.png", 10);

    // symbol->symbology = atoi(optarg);
    if(error_number == 0) {
        error_number = ZBarcode_Print(symbol, rotate_angle);
    }
    // png_to_file(symbol, image_height, image_width, symbol->output_buffer, 0, 100);
    error_number = png_pixel_plot(symbol, image_width, image_width, symbol->output_buffer, rotate_angle);
    FILE *write_ptr;

    write_ptr = fopen("test.png","wb");  // w for write, b for binary

    fwrite(state.buffer,sizeof(state.buffer), state.size, write_ptr); // write 10 bytes to our buffer
    ZBarcode_Delete(symbol);
    return 0;
}
