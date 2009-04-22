BEGIN { $^W = 1; }
use strict;

my %attempted = ();
my %failed = ();

my $show_failures = 1;
if (@ARGV && ($ARGV[0] = '-no-failures'))
{
    $show_failures = 0;
}

while (<STDIN>)
{
    if (m/^abuild: (\S+) \(abuild-.*?\): all/)
    {
	$attempted{$1} = 1;
    }
    elsif (m/^abuild: (\S+) \(abuild-.*?\): build failed/)
    {
	$failed{$1} = 1;
    }
}

print "attempted: ", join(' ', sort keys %attempted), "\n";
print "failed: ", join(' ', sort keys %failed), "\n" if $show_failures;
