require 5.008;
BEGIN { $^W = 1; }
use strict;

my $whoami = ($0 =~ m,([^/\\]*)$,) ? $1 : $0;
#my $dirname = ($0 =~ m,(.*)[/\\][^/\\]+$,) ? $1 : ".";

my $calculate = $ENV{'CALCULATE'} or die "$whoami: CALCULATE is not defined\n";

my $file = shift(@ARGV);
open(F, "<$file") or die "$whoami: can't open $file: $!\n";
my @numbers = ();
while (<F>)
{
    s/\r?\n//;
    if (! m/^\d+$/)
    {
	die "$whoami: each line of $file must be a number\n";
    }
    push(@numbers, $_);
}

print <<EOF
\#include <iostream>
void generate()
{
EOF
    ;

open(P, "$calculate " . join(' ', @numbers) . "|") or
    die "$whoami: can't run calculate\n";
while (<P>)
{
    if (m/^(\d+)\t(\d+)/)
    {
	print "    std::cout << $1 << \" squared is \" << $2 << std::endl;\n";
    }
}

print <<EOF
}
EOF
    ;
