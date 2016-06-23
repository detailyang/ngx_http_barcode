
use warnings;
use strict;
use Test::Nginx::Socket 'no_plan';
require Image::Magick;
require Barcode::ZBar;

no_shuffle();
run_tests();

sub bardecode($) {
    my $content = shift;
    my $scanner = Barcode::ZBar::ImageScanner->new();

    # configure the reader
    $scanner->parse_config("enable");

    my $magick = Image::Magick->new(magick => 'png');
    $magick->BlobToImage($content);

    my $raw = $magick->ImageToBlob(magick => 'GRAY', depth => 8);
    my $image = Barcode::ZBar::Image->new();
    $image->set_format('Y800');
    $image->set_size($magick->Get(qw(columns rows)));
    $image->set_data($raw);

    # scan the image for barcodes
    my $n = $scanner->scan_image($image);

    # extract results
    foreach my $symbol ($image->get_symbols()) {
        # clean up
        undef($image);
        return $symbol->get_type() . ":" . $symbol->get_data();
    }

    #nope
    return "badbeef";
}

__DATA__
=== TEST 1: barcode should be right
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
["GET /barcode?txt=1",
 "GET /barcode?txt=2",
 "GET /barcode?txt=3",
 "GET /barcode?txt=4",
]

--- response_body_filters eval
[\&main::bardecode]

--- response_body_like eval
["CODE-128:1", "CODE-128:2",
 "CODE-128:3", "CODE-128:4",
]

