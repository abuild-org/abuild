#!/usr/bin/env perl
require 5.008;
use warnings;
use strict;

while (<>)
{
    if (m/abuild: \(verbose\)/)
    {
	if (m/looking for top|top-search|travers|backing/)
	{
	    print;
	}
	elsif (m/(creating|importing) interface/)
	{
	    print;
	}
    }
    else
    {
	print;
    }
}
