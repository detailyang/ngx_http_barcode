# move to [x-v8/ngx_http_barcode](https://github.com/x-v8/ngx_http_barcode)
# ngx http barcode module
![Branch master](https://img.shields.io/badge/branch-master-brightgreen.svg?style=flat-square)[![Build](https://api.travis-ci.org/detailyang/ngx_http_barcode.svg)](https://travis-ci.org/detailyang/ngx_http_barcode)[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/detailyang/ngx_http_barcode/master/LICENSE)[![release](https://img.shields.io/github/release/detailyang/ngx_http_barcode.svg)](https://github.com/detailyang/ngx_http_barcode/releases)


ngx_http_barcode_module is a an addon for nginx to generate barcode

Table of Contents
-----------------
* [How-To-Use](#how-to-use)
* [Requirements](#requirements)
* [Direction](#direction)
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
 * [zint](https://github.com/detailyang/zint) must use this version patched by [detailyang](https://github.com/detailyang), which can generate barcode in memory

Direction
------------

* barcode: enable barcode       
Syntax:     barcode       
Default:    -      
Context:    location       

```
    location / {
        barcode;
    }
```

* barcode_txt: barcode content    
Syntax:     barcode_txt xxx       
Default:    -        
Context:    location    

```
    location / {
        barcode_txt $arg_txt;
        barcode;
    }
```

* barcode_bg: background color      
Syntax:     barcode_bg xxx     
Default:    - (000000-ffffff)     
Context:    location       
```
    location / {
        barcode_bg $arg_bg;
        barcode;
    }  
```

* barcode_fg: foreground color       
Syntax:     barcode_fg xxx      
Default:    - (000000-ffffff)     
Context:    location       

```
    location / {
        barcode_fg $arg_fg;
        barcode;
    }  
```

* barcode_height: barcode height    
Syntax:     barcode_height xxx      
Default:    -           
Context:    location          
  
```
    location / {
        barcode_height $arg_height;
        barcode;
    }  
```

* barcode_scale: barcode picture scale     
Syntax:     barcode_scale xxx         
Default:    - ([0.01-3])       
Context:    location       

```
    location / {
        barcode_scale $arg_scale;
        barcode;
    }  
```

* barcode_rotate: barcode picture rotate angle       
Syntax:     barcode_scale xxx       
Default:    - ([0，90，180，270] only)        
Context:    location         

```
    location / {
        barcode_rotate $arg_rotate;
        barcode;
    }  
```

* barcode_rotate: barcode picture show human readable text     
Syntax:     barcode_hrt xxx     
Default:    false           
Context:    location        

```
    location / {
        barcode_hrt $arg_hrt;
        barcode;
    }  
```

* barcode_barcode: barcode type        
Syntax:     barcode_hrt xxx       
Default:    20         
Context:    location          

```
    location / {
        barcode_barcode $arg_barcode;
        barcode;
    } 
    # type can be as following (default 20):
    1: Code 11 51: Pharma One-Track 90: KIX Code
2: Standard 2of5 52: PZN 92: Aztec Code
3: Interleaved 2of5 53: Pharma Two-Track 93: DAFT Code
4: IATA 2of5 55: PDF417 97: Micro QR Code
6: Data Logic 56: PDF417 Trunc 98: HIBC Code 128
7: Industrial 2of5 57: Maxicode 99: HIBC Code 39
8: Code 39 58: QR Code 102: HIBC Data Matrix
9: Extended Code 39 60: Code 128-B 104: HIBC QR Code
13: EAN 63: AP Standard Customer 106: HIBC PDF417
16: GS1-128 66: AP Reply Paid 108: HIBC MicroPDF417
18: Codabar 67: AP Routing 112: HIBC Aztec Code
20: Code 128 68: AP Redirection 128: Aztec Runes
21: Leitcode 69: ISBN 129: Code 23
22: Identcode 70: RM4SCC 130: Comp EAN
23: Code 16k 71: Data Matrix 131: Comp GS1-128
24: Code 49 72: EAN-14 132: Comp Databar-14
25: Code 93 75: NVE-18 133: Comp Databar Ltd
28: Flattermarken 76: Japanese Post 134: Comp Databar Ext
29: Databar-14 77: Korea Post 135: Comp UPC-A
30: Databar Limited 79: Databar-14 Stack 136: Comp UPC-E
31: Databar Extended 80: Databar-14 Stack Omni 137: Comp Databar-14 Stack
32: Telepen Alpha 81: Databar Extended Stack 138: Comp Databar Stack Omni
34: UPC-A 82: Planet 139: Comp Databar Ext Stack
37: UPC-E 84: MicroPDF 140: Channel Code
40: Postnet 85: USPS OneCode 141: Code One
47: MSI Plessey 86: UK Plessey 142: Grid Matrix
49: FIM 87: Telepen Numeric
50: Logmars 89: ITF-14
```

Contributing
------------

To contribute to ngx_http_barcode, clone this repo locally and commit your code on a separate branch.


Author
------

> GitHub [@detailyang](https://github.com/detailyang)


License
-------
ngx_http_barcode is licensed under the [MIT] license.

[MIT]: https://github.com/detailyang/ybw/blob/master/licenses/MIT
