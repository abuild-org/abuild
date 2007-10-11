BEGIN { $^W = 1; }
use strict;

my @lines = ();

while (<>)
{
    s,\\,/,g;
    push(@lines, $_);
}
map { print } sort @lines;
