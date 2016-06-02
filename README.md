# ngx http barcode module



ngx_http_barcode_module is a an addon for nginx to generate barcode

Table of Contents
-----------------
* [How-To-Use](#how-to-work)
* [Requirements](#requirements)
* [Direction](#direction)
* [Production](#production)
* [Contributing](#contributing)
* [Author](#author)
* [License](#license)


How-To-Use
----------------

ngx_http_barcode_module let the nginx location become a barcode interface.
For example:

```bash
location / {
        barcode_txt $arg_txt;
        barcode_bg $arg_bg;
        barcode_fg $arg_fg;
        barcode_height $arg_height;
        barcode_scale $arg_scale;
        barcode_rotate $arg_rotate;
        barcode_hrt $arg_hrt;
        barcode_barcode $arg_barcode;
        barcode;
}
```

Requirements
------------

ngx_http_barcode requires the following to run:

 * [nginx](http://nginx.org/) or other forked version like [openresty](http://openresty.org/)、[tengine](http://tengine.taobao.org/)
 * [zint](https://github.com/detailyang/zint) must use this version patched by (detailyang)[https://github.com/detailyang], which can generate barcode in memory



