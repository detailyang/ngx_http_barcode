#ifndef __NGX_HTTP_BARCODE_MODULE_H__
#define __NGX_HTTP_BARCODE_MODULE_H__
#include <ngx_core.h>
#include <ngx_http.h>
#include <nginx.h>
#include <zint.h>
#include <png.h>
#include <string.h>

#define SSET    "0123456789ABCDEF"

typedef struct {
  char *buffer;
  size_t size;
  ngx_log_t *log;
  ngx_pool_t *pool;
} png_buf_t;

struct mainprog_info_type {
    long width;
    long height;
    FILE *outfile;
    jmp_buf jmpbuf;
};

typedef enum {
    barcode_cfg_txt = 0,
    barcode_cfg_fg,
    barcode_cfg_bg,
    barcode_cfg_height,
    barcode_cfg_scale,
    barcode_cfg_rotate,
    barcode_cfg_hrt
} ngx_http_barcode_cfg_t;

typedef struct {
    ngx_http_barcode_cfg_t   cfg_code;
    ngx_array_t             *args;
} ngx_http_barcode_cmd_t;

typedef struct {
    ngx_str_t       raw_value;
    ngx_array_t     *lengths;
    ngx_array_t     *values;
} ngx_http_barcode_cmd_template_t;

typedef struct {
    ngx_str_t fg;
    ngx_str_t bg;
    ngx_str_t txt;
    ngx_str_t height;
    ngx_str_t scale;
    ngx_str_t rotate;
    ngx_str_t hrt;
    ngx_array_t *cmds;
} ngx_http_barcode_loc_conf_t;

static char *ngx_http_barcode_txt(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char * ngx_http_barcode_set(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_barcode_handler(ngx_http_request_t *req);
static void *ngx_http_barcode_create_loc_conf(ngx_conf_t *cf);
static ngx_int_t ngx_http_barcode_handler(ngx_http_request_t *req);
static void writepng_error_handler(png_structp png_ptr, png_const_charp msg);
static int escape_char_process(struct zint_symbol *symbol, uint8_t input_string[], int length);
static void ngx_http_barcode_png_write_data(png_structp png_ptr, png_bytep data, png_size_t length);
static int get_barcode_size(struct zint_symbol *symbol, ngx_int_t *i_height, ngx_int_t *i_width);
static int png_pixel_plot(struct zint_symbol *symbol, int image_height, int image_width,
    char *pixelbuf, int rotate_angle, png_buf_t *png_buf);
static int png_pixel_scale(ngx_http_request_t *req, struct zint_symbol *symbol,
    ngx_int_t *image_height, ngx_int_t *image_width);
static void *ngx_http_barcode_ngx_prealloc(ngx_pool_t *pool, void *p, size_t old_size, size_t new_size);
static ngx_int_t ngx_http_barcode_run_variables(ngx_http_request_t *r, ngx_http_barcode_loc_conf_t *blcf);
static char *ngx_http_barcode_compile_variables(ngx_http_barcode_cfg_t cfg_code,
        ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_http_barcode_fg(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_http_barcode_bg(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_http_barcode_height(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_http_barcode_scale(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_http_barcode_rotate(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_http_barcode_hrt(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

extern int ustrlen(const uint8_t source[]);
extern void to_latin1(uint8_t source[], uint8_t preprocessed[]);
extern int module_is_set(struct zint_symbol *symbol, int y_coord, int x_coord);
extern int is_extendable(int symbology);
extern int ctoi(char source);
extern char itoc(int source);
extern void to_upper(uint8_t source[]);
extern int is_sane(char test_string[], uint8_t source[], int length);

#endif
