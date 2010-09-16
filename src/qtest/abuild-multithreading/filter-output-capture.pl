use warnings;
use strict;

my $all_interleaved = 0;
my $full_a_out = undef;
my $full_a_err = undef;
my $full_b_out = undef;
my $full_b_err = undef;
my $full_a_out_line = 0;
my $full_a_err_line = 0;
my $full_b_out_line = 0;
my $full_b_err_line = 0;
my $prefix_count = 0;

while (<>)
{
    if (m/a out1/ && m/a err1/ && m/b out1/ && m/b err1/ && m/err2/)
    {
	$all_interleaved = 1;
    }
    if (m/^(.*?)(\[1\] )?a out1 a out2\r?$/)
    {
	$full_a_out = $1;
	$full_a_out_line = $.;
	++$prefix_count if defined $2;
    }
    if (m/^(.*?)(\[1\] )?a err1 a err2\r?$/)
    {
	$full_a_err = $1;
	$full_a_err_line = $.;
	++$prefix_count if defined $2;
    }
    if (m/^(.*?)(\[2\] )?b out1 b out2\r?$/)
    {
	$full_b_out = $1;
	$full_b_out_line = $.;
	++$prefix_count if defined $2;
    }
    if (m/^(.*?)(\[2\] )?b err1 b err2\r?$/)
    {
	$full_b_err = $1;
	$full_b_err_line = $.;
	++$prefix_count if defined $2;
    }
}

print "saw all interleaved: $all_interleaved\n";
if (! $all_interleaved)
{
    print "prefix count: $prefix_count\n";
    print "full a out: -$full_a_out-\n" if defined $full_a_out;
    print "full a err: -$full_a_err-\n" if defined $full_a_err;
    print "full b out: -$full_b_out-\n" if defined $full_b_out;
    print "full b err: -$full_b_err-\n" if defined $full_b_err;
    if (defined $full_a_out)
    {
	my $both_err_before_out = 0;
	if (($full_a_err_line < $full_b_out_line) &&
	    ($full_b_err_line < $full_a_out_line))
	{
	    $both_err_before_out = 1;
	}
	print "both err before both out: $both_err_before_out\n";
    }
}
