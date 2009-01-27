BEGIN { $^W = 1; }
use strict;

my $in_xml = 0;
while (<>)
{
    if (m/<?xml /)
    {
	$in_xml = 1;
    }
    if ($in_xml)
    {
	print;
	if (m,</abuild-data>,)
	{
	    $in_xml = 0;
	}
    }
}
