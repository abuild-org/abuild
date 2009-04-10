BEGIN { $^W = 1; }
use strict;

my @found = ();
my $debug = 0;
while (<>)
{
    if (m/^\s*(\S+?)\.dll/i)
    {
	my $dll = $1;
	if ($dll =~ m/MSVC(.)/i)
	{
	    push(@found, lc($1));
	}
	if ($dll =~ m/d$/i)
	{
	    $debug = 1;
	}
    }
}
my $found = join('', sort @found);
my $dll_type = ($found ? "$found:" . ($debug ? "debug" : "release") : "none");
print "MSVC DLLs found: $dll_type\n";
