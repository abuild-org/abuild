use warnings;
use strict;
use File::Basename;

my @warnings = ();
my %warnings = ();
while (<>)
{
    if (m/^(\S.*\.h):.*warning:/)
    {
	my $file = $1;
	if (! exists $warnings{$file})
	{
	    $warnings{$file} = 1;
	    push(@warnings, $file);
	}
    }
}
print "warnings: " . join(' ', @warnings) . "\n";
