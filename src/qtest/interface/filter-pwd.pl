BEGIN { $^W = 1; }
use strict;

my $cwd = shift(@ARGV);
while (<>)
{
    s/$cwd/--CWD--/g;
    print;
}
