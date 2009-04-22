BEGIN { $^W = 1; }
use strict;

my %attempted = ();
my %failed = ();

my $show_failures = 1;
my $logfile = shift(@ARGV);
if (@ARGV && ($ARGV[0] = '-no-failures'))
{
    $show_failures = 0;
}

open(L, ">$logfile");
while (<STDIN>)
{
    print L;
    # These regexps are not anchored since they are sometimes
    # interleaved with make output.
    if (m/abuild: (\S+) \(abuild-.*?\): all/)
    {
	$attempted{$1} = 1;
    }
    elsif (m/abuild: (\S+) \(abuild-.*?\): build failed/)
    {
	$failed{$1} = 1;
    }
}
close(L);

print "attempted: ", join(' ', sort keys %attempted), "\n";
print "failed: ", join(' ', sort keys %failed), "\n" if $show_failures;
