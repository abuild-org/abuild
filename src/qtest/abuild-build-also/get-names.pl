#!/usr/bin/env perl
require 5.008;
use warnings;
use strict;
use File::Basename;

my $whoami = basename($0);

my @names = ();
while (<>)
{
    if (m/^  -- (\S+) --/)
    {
	push(@names, $1);
    }
    elsif (s/^E //)
    {
	print;
    }
}
print join(' ', @names), "\n";
