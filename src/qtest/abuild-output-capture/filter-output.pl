use warnings;
use strict;

my $section = 'unknown';
my $unexp_section = undef;

while (<>)
{
    if (m/^Unallocated output/)
    {
	$section = 'unallocated output';
    }
    elsif (m/Output for (.+)/)
    {
	$section = $1;
    }

    if (m/JavaBuilder.*will attempt/)
    {
	$unexp_section = $section;
    }
    else
    {
	print;
    }
}
if (defined $unexp_section)
{
    print "JavaBuilder unexpected exit message found in $unexp_section\n";
}
