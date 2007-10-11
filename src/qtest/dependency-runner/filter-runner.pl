BEGIN { $^W = 1; }
use strict;

my @cache = ();
my $line = "";
while (<>)
{
    if (m/^Status:/)
    {
	$line = $_;
	last;
    }
    push(@cache, $_);
}
for (sort @cache)
{
    print;
}
print $line;
while (<>)
{
    print;
}
