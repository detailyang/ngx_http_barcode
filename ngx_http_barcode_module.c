/*
* @Author: detailyang
* @Date:   2016-05-18 09:43:45
* @Last Modified by:   detailyang
* @Last Modified time: 2016-05-19 13:42:07
*/
#include "ngx_http_barcode_module.h"

static ngx_command_t ngx_http_barcode_commands[] = {
    {
        ngx_string("barcode"),
        NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
        ngx_http_barcode_set,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL,
    },
    {
        ngx_string("barcode_txt"),
        NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        ngx_http_barcode_txt,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_barcode_loc_conf_t, txt),
        NULL
    },
    {
        ngx_string("barcode_fg"),
        NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        ngx_http_barcode_fg,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_barcode_loc_conf_t, fg),
        NULL
    },
    {
        ngx_string("barcode_bg"),
        NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        ngx_http_barcode_bg,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_barcode_loc_conf_t, bg),
        NULL
    },
    {
        ngx_string("barcode_height"),
        NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        ngx_http_barcode_height,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_barcode_loc_conf_t, height),
        NULL
    },
    {
        ngx_string("barcode_scale"),
        NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        ngx_http_barcode_scale,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_barcode_loc_conf_t, scale),
        NULL
    },
    {
        ngx_string("barcode_rotate"),
        NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        ngx_http_barcode_rotate,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_barcode_loc_conf_t, rotate),
        NULL
    },
    {
        ngx_string("barcode_hrt"),
        NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        ngx_http_barcode_hrt,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_barcode_loc_conf_t, hrt),
        NULL
    },
    ngx_null_command
};

static ngx_http_module_t ngx_http_barcode_module_ctx = {
    NULL,                             /* preconfiguration */
    NULL,                             /* postconfiguration */
    NULL,                             /* create main configuration */
    NULL,                             /* init main configuration */
    NULL,                             /* create server configuration */
    NULL,                             /* merge server configuration */
    ngx_http_barcode_create_loc_conf, /* create location configuration */
    NULL                              /* merge location configuration */
};

ngx_module_t ngx_http_barcode_module = {
    NGX_MODULE_V1,
    &ngx_http_barcode_module_ctx,  /* module context */
    ngx_http_barcode_commands,     /* module directives */
    NGX_HTTP_MODULE,               /* module type */
    NULL,                          /* init master */
    NULL,                          /* init module */
    NULL,                          /* init process */
    NULL,                          /* init thread */
    NULL,                          /* exit thread */
    NULL,                          /* exit process */
    NULL,                          /* exit master */
    NGX_MODULE_V1_PADDING
};

static void *
ngx_http_barcode_create_loc_conf(ngx_conf_t *cf) {
    ngx_http_barcode_loc_conf_t *blcf;

    blcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_barcode_loc_conf_t));
    if (blcf == NULL) {
        return NGX_CONF_ERROR;
    }

    return blcf;
}

static char *
ngx_http_barcode_set(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    ngx_http_core_loc_conf_t        *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_barcode_handler;

    return NGX_CONF_OK;
};

static ngx_int_t
ngx_http_barcode_handler(ngx_http_request_t *req) {
    struct zint_symbol *symbol;
    png_buf_t png_buf = {
        .buffer = NULL,
        .size = 0,
        .log = req->connection->log,
        .pool = req->pool,
    };
    ngx_int_t rc, image_width, image_height;
    int error_number;
    ssize_t size;
    ngx_http_barcode_loc_conf_t *blcf;
    u_char *txt;
    int rotate = 0;

    blcf = ngx_http_get_module_loc_conf(req, ngx_http_barcode_module);
    rc = ngx_http_barcode_run_variables(req, blcf);
    if (rc != NGX_OK)  {
        ngx_log_error(NGX_LOG_ERR, req->connection->log, 0, "barcode: run variables error");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    if (blcf->txt.len == 0) {
        return NGX_HTTP_BAD_REQUEST;
    }
    txt = ngx_pcalloc(req->pool, blcf->txt.len + 1);
    if (txt == NULL) {
        ngx_log_error(NGX_LOG_ERR, req->connection->log, 0, "barcode: pcalloc txt error");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    ngx_sprintf(txt, "%V", &blcf->txt);

    symbol = ZBarcode_Create();
    if (blcf->fg.len) {
        strncpy(symbol->fgcolour, (const char *)blcf->fg.data, blcf->fg.len > 6 ? 6 : blcf->fg.len);
    }
    if (blcf->bg.len) {
        strncpy(symbol->bgcolour, (const char *)blcf->bg.data, blcf->bg.len > 6 ? 6 : blcf->bg.len);
    }
    if (blcf->height.len) {
        symbol->height = ngx_atoi(blcf->height.data, blcf->height.len);
        if (symbol->height < 1 || symbol->height > 1000) {
            return NGX_HTTP_BAD_REQUEST;
        }
    }
    if (blcf->scale.len) {
        symbol->scale = ngx_atofp(blcf->scale.data, blcf->scale.len, 2) / 100.00;
        ngx_log_error(NGX_LOG_ERR, req->connection->log, 0, "scale %.2f", symbol->scale);
        if (symbol->scale < 0.01 || symbol->scale > 3) {
            return NGX_HTTP_BAD_REQUEST;
        }
    }
    if (blcf->hrt.len) {
        symbol->show_hrt = 0;
    }
    if (blcf->rotate.len) {
        rotate = ngx_atoi(blcf->rotate.data, blcf->rotate.len);
        if (rotate != 0 && rotate != 90 && rotate != 180 && rotate != 270) {
            return NGX_HTTP_BAD_REQUEST;
        }
    }
    error_number = escape_char_process(symbol, (uint8_t *)txt, ngx_strlen(txt));
    if (error_number != 0) {
        ZBarcode_Delete(symbol);
        ngx_log_error(NGX_LOG_ERR, req->connection->log, 0, "barcode: escape error");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    size = get_barcode_size(symbol, &image_height, &image_width);
    if (size == 0) {
        ngx_log_error(NGX_LOG_ERR, req->connection->log, 0, "barcode: barcode size is zero");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    char *barcode_buf;
    barcode_buf = ngx_pcalloc(req->pool, size);
    if (barcode_buf == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    symbol->output_buffer = barcode_buf;

    error_number = ZBarcode_Print(symbol, rotate);
    if(error_number != 0) {
        ngx_log_error(NGX_LOG_ERR, req->connection->log, 0, "barcode: print error %d", error_number);
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    if (blcf->scale.len) {
        if (png_pixel_scale(req, symbol, &image_height, &image_width) != NGX_OK) {
            ngx_log_error(NGX_LOG_ERR, req->connection->log, 0, "barcode: scale error");
            return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }
    }
    error_number = png_pixel_plot(symbol, image_height, image_width,
        symbol->output_buffer, rotate, &png_buf);
    if (error_number != 0) {
        ngx_log_error(NGX_LOG_ERR, req->connection->log, 0, "barcode: errno %d", error_number);
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    ngx_log_error(NGX_LOG_ERR, req->connection->log, 0, "size: %d", png_buf.size);

    req->headers_out.status = 200;
    req->headers_out.content_length_n = png_buf.size;
    ngx_str_set(&req->headers_out.content_type, "image/png");
    rc = ngx_http_send_header(req);
    if (rc == NGX_ERROR || rc > NGX_OK || req->header_only) {
        return rc;
    }

    ngx_buf_t *b; b = ngx_pcalloc(req->pool, sizeof(ngx_buf_t));
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    ngx_chain_t out;
    out.buf = b;
    out.next = NULL;
    b->pos = (u_char *)png_buf.buffer;
    b->last = (u_char *)(png_buf.buffer + png_buf.size);
    b->memory = 1;
    b->last_buf = 1;

    return ngx_http_output_filter(req, &out);
}

static char *
ngx_http_barcode_txt(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    return ngx_http_barcode_compile_variables(barcode_cfg_txt, cf, cmd, conf);
}

static char *
ngx_http_barcode_fg(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    return ngx_http_barcode_compile_variables(barcode_cfg_fg, cf, cmd, conf);
}

static char *
ngx_http_barcode_bg(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    return ngx_http_barcode_compile_variables(barcode_cfg_bg, cf, cmd, conf);
}

static char *
ngx_http_barcode_height(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    return ngx_http_barcode_compile_variables(barcode_cfg_height, cf, cmd, conf);
}

static char *
ngx_http_barcode_scale(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    return ngx_http_barcode_compile_variables(barcode_cfg_scale, cf, cmd, conf);
}

static char *
ngx_http_barcode_rotate(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    return ngx_http_barcode_compile_variables(barcode_cfg_rotate, cf, cmd, conf);
}

static char *
ngx_http_barcode_hrt(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    return ngx_http_barcode_compile_variables(barcode_cfg_hrt, cf, cmd, conf);
}

static char *
ngx_http_barcode_compile_variables(ngx_http_barcode_cfg_t cfg_code,
        ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    ngx_http_barcode_loc_conf_t  *blcf;
    ngx_http_barcode_cmd_t       *barcode_cmd;
    ngx_str_t   *raw_args;
    ngx_array_t **cmds_ptr;
    ngx_array_t **args_ptr;
    ngx_http_barcode_cmd_template_t  *arg;
    ngx_http_script_compile_t       sc;
    ngx_uint_t i, n;

    blcf = (ngx_http_barcode_loc_conf_t *)conf;
    cmds_ptr = &blcf->cmds;

    if (*cmds_ptr == NULL)
    {
        *cmds_ptr = ngx_array_create(cf->pool, 1, sizeof(ngx_http_barcode_cmd_t));
        if (*cmds_ptr == NULL) {
            return NGX_CONF_ERROR;
        }
    }

    barcode_cmd = ngx_array_push(*cmds_ptr);
    if (barcode_cmd == NULL) {
        return NGX_CONF_ERROR;
    }

    barcode_cmd->cfg_code = cfg_code;

    args_ptr = &barcode_cmd->args;
    *args_ptr = ngx_array_create(cf->pool, 1, sizeof(ngx_http_barcode_cmd_template_t));

    if (*args_ptr == NULL) {
        return NGX_CONF_ERROR;
    }

    raw_args = cf->args->elts;
    for (i = 1 ; i < cf->args->nelts; i++)
    {
        arg = ngx_array_push(*args_ptr);

        if (arg == NULL)
            return NGX_CONF_ERROR;

        arg->raw_value  = raw_args[i];
        arg->lengths    = NULL;
        arg->values     = NULL;

        n = ngx_http_script_variables_count(&arg->raw_value);
        if (n > 0) {
            ngx_memzero(&sc, sizeof(ngx_http_script_compile_t));

            sc.cf = cf;
            sc.source  = &arg->raw_value;
            sc.lengths = &arg->lengths;
            sc.values  = &arg->values;
            sc.variables = n;
            sc.complete_lengths = 1;
            sc.complete_values  = 1;

            if (ngx_http_script_compile(&sc) != NGX_OK) {
                return NGX_CONF_ERROR;
            }
        }
    } /* end for */

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_barcode_eval_cmd_args(ngx_http_request_t *r,
        ngx_http_barcode_cmd_t *cmd, ngx_array_t *compiled_args)
{
    ngx_http_barcode_cmd_template_t *value;
    ngx_str_t *arg;
    ngx_uint_t i;

    value = cmd->args->elts;
    for (i = 0; i < cmd->args->nelts; i++)
    {
        arg = ngx_array_push(compiled_args);

        if (arg == NULL) {
            return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }

        if (value[i].lengths == NULL) {
            *arg = value[i].raw_value;
        } else {
            if (ngx_http_script_run(r, arg, value[i].lengths->elts,
                        0, value[i].values->elts) == NULL) {
                return NGX_HTTP_INTERNAL_SERVER_ERROR;
            }
        }
    }

    return NGX_OK;
}

static ngx_int_t
ngx_http_barcode_run_variables(ngx_http_request_t *r, ngx_http_barcode_loc_conf_t *blcf) {
    ngx_array_t             *cmds;
    ngx_http_barcode_cmd_t  *cmd;
    ngx_array_t             *compiled_args = NULL;
    ngx_str_t               *value;
    ngx_int_t               rc;
    ngx_uint_t              i;

    cmds = blcf->cmds;
    if (cmds == NULL) {
        return NGX_DECLINED;
    }

    cmd = cmds->elts;
    for (i = 0; i < cmds->nelts; i++)
    {
        if (cmd[i].args)
        {
            compiled_args = ngx_array_create(r->pool,
                    cmd[i].args->nelts, sizeof(ngx_str_t));

            if (compiled_args == NULL) {
                ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                        "barcode: alloc compiled_args error");
                return NGX_HTTP_INTERNAL_SERVER_ERROR;
            }

            rc = ngx_http_barcode_eval_cmd_args(r, &cmd[i], compiled_args);
            if (rc != NGX_OK) {
                ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                        "barcode: eval args errro");
                return rc;
            }
        }

        value = compiled_args->elts;
        switch (cmd[i].cfg_code) {
            case barcode_cfg_txt:
                blcf->txt.data = value[0].data;
                blcf->txt.len = value[0].len;
                break;
            case barcode_cfg_fg:
                blcf->fg.data = value[0].data;
                blcf->fg.len = value[0].len;
                break;
            case barcode_cfg_bg:
                blcf->bg.data = value[0].data;
                blcf->bg.len = value[0].len;
                break;
            case barcode_cfg_height:
                blcf->height.data = value[0].data;
                blcf->height.len = value[0].len;
                break;
            case barcode_cfg_scale:
                blcf->scale.data = value[0].data;
                blcf->scale.len = value[0].len;
                break;
            case barcode_cfg_rotate:
                blcf->rotate.data = value[0].data;
                blcf->rotate.len = value[0].len;
                break;
            case barcode_cfg_hrt:
                blcf->hrt.data = value[0].data;
                blcf->hrt.len = value[0].len;
                break;
            default:
                ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                        "barcode: unknow directive %d", cmd[i].cfg_code);
                return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }
    }

    return NGX_OK;
}

static void *
ngx_http_barcode_ngx_prealloc(ngx_pool_t *pool, void *p, size_t old_size, size_t new_size)
{
    void *new;
    ngx_pool_t *node;

    if (p == NULL) {
        return ngx_palloc(pool, new_size);
    }

    if (new_size == 0) {
        if ((u_char *) p + old_size == pool->d.last) {
           pool->d.last = p;
        } else {
           ngx_pfree(pool, p);
        }

        return NULL;
    }

    if (old_size <= pool->max) {
        for (node = pool; node; node = node->d.next) {
            if ((u_char *)p + old_size == node->d.last
                && (u_char *)p + new_size <= node->d.end) {
                node->d.last = (u_char *)p + new_size;
                return p;
            }
        }
    }

    if (new_size <= old_size) {
       return p;
    }

    new = ngx_palloc(pool, new_size);
    if (new == NULL) {
        return NULL;
    }

    ngx_memcpy(new, p, old_size);

    ngx_pfree(pool, p);

    return new;
}


static int
get_barcode_size(struct zint_symbol *symbol, ngx_int_t *i_height, ngx_int_t *i_width) {
    int textdone, main_width, comp_offset, large_bar_count;
    char addon[6];
    float addon_text_posn, preset_height, large_bar_height;
    int i, r, textoffset, yoffset, xoffset, latch, image_width, image_height;
    int smalltext = 0;
    float row_height;

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
    *i_width = image_width;
    *i_height = image_height;

    return image_width * image_height;
}

static int
escape_char_process(struct zint_symbol *symbol, uint8_t input_string[], int length)
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

static int
png_pixel_scale(ngx_http_request_t *req, struct zint_symbol *symbol, ngx_int_t *image_height, ngx_int_t *image_width) {
    float scaler = symbol->scale;
    u_char *scaled_pixelbuf;
    int horiz, vert, i;
    ngx_int_t scale_width, scale_height;

    if(scaler == 0) {
        scaler = 0.5;
    }
    scale_width = *image_width * scaler;
    scale_height = *image_height * scaler;

    scaled_pixelbuf = ngx_pcalloc(req->pool, scale_width * scale_height);
    if (scaled_pixelbuf == NULL) {
        ngx_log_error(NGX_LOG_ERR, req->connection->log, 0, "barcode: scale buffer realloc error");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    for(i = 0; i < (scale_width * scale_height); i++) {
        *(scaled_pixelbuf + i) = '0';
    }

    for(vert = 0; vert < scale_height; vert++) {
        for(horiz = 0; horiz < scale_width; horiz++) {
            *(scaled_pixelbuf + (vert * scale_width) + horiz) =
                *(symbol->output_buffer + ((int)(vert / scaler) * *image_width) + (int)(horiz / scaler));
        }
    }
    if (ngx_pfree(req->pool, symbol->output_buffer) != NGX_OK) {
        return NGX_ERROR;
    }
    symbol->output_buffer = (char *)scaled_pixelbuf;
    *image_height = scale_height;
    *image_width = scale_width;

    return NGX_OK;
}

static int
png_pixel_plot(struct zint_symbol *symbol,
               int image_height, int image_width,
               char *pixelbuf, int rotate_angle,
               png_buf_t *png_buf) {
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

    png_set_write_fn(png_ptr, png_buf, ngx_http_barcode_png_write_data, NULL);

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

    /* make sure we have disengaged */
    if (png_ptr && info_ptr) png_destroy_write_struct(&png_ptr, &info_ptr);
    return 0;
}

static void
writepng_error_handler(png_structp png_ptr, png_const_charp msg) {
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

static void
ngx_http_barcode_png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    /* with libpng15 next line causes pointer deference error; use libpng12 */
    png_buf_t * p = (png_buf_t *)png_get_io_ptr(png_ptr); /* was png_ptr->io_ptr */
    size_t nsize = p->size + length;

    p->buffer = ngx_http_barcode_ngx_prealloc(p->pool, p->buffer, p->size, nsize);

    if(!p->buffer) {
        ngx_log_error(NGX_LOG_ERR, p->log, 0, "barcode: realloc error");
        png_error(png_ptr, "Write Error");
    }

    /* copy new bytes to end of buffer */
    memcpy(p->buffer + p->size, data, length);
    p->size += length;
}
