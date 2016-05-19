use Test::Nginx::Socket 'no_plan';

repeat_each(30);
# plan tests => repeat_each() * (3 * blocks());
no_shuffle();
run_tests();

__DATA__
=== TEST 1: memory should not be leak
--- config
    location = /barcode {
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

--- request eval
["GET /barcode?txt=123", "GET /barcode?txt=456",
 "GET /barcode?txt=abcd", "GET /barcode?txt=defg",
 "GET /barcode?txt=123aa", "GET /barcode?txt=456ww",
 "GET /barcode?txt=abcdaa", "GET /barcode?txt=defgww",
 "GET /barcode?txt=123ff", "GET /barcode?txt=456ww",
 "GET /barcode?txt=abcdgg", "GET /barcode?txt=defgww"
]

--- error_code eval
[200, 200,
200, 200,
200, 200,
200, 200,
200, 200,
200, 200,
]
