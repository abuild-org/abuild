use warnings;
use strict;

$| = 1;

my $run_child = 1;
if (@ARGV && ($ARGV[0] eq '-no-child'))
{
    $run_child = 0;
}

my $indent = $run_child ? "" : "  ";

my $in = scalar(<STDIN>);
if (defined $in)
{
    warn "stdin was not empty\n";
}

print "${indent}line 1\n";
pause();
# Write to stdout with stderr interleaved.  The output handler will
# de-interleave them.
print "${indent}out";
warn "${indent}error\n";
pause();
print "put\n";
if ($run_child)
{
    print "${indent}running child\n";
    system("perl $0 -no-child");
}
else
{
    print "${indent}not running child\n";
}
print "${indent}bye\n";

sub pause
{
    select(undef, undef, undef, 0.25);
}
