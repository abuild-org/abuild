BEGIN { $^W = 1; }
use strict;

my $found = 0;
while (<>)
{
    if (m/^abuild: ERROR:.*JavaBuilder/)
    {
	$found = 1;
    }
    else
    {
	print;
    }
}

if ($found)
{
    print "--found JavaBuilder errors--\n";
}
else
{
    print "NO JavaBuilder errors found!\n";
}
