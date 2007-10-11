BEGIN { $^W = 1; }
use strict;

my $top = shift(@ARGV) or die;
my $root = "/";
if ($top =~ m,^(.:/),)
{
    $root = $1;
}

my $will_filter = 0;
my $filtering_env = 0;
my $printing_env = 0;
my $got_more = 0;

while (<>)
{
    s/$top/--top--/;
    s,$root,/,;
    if (m/env = both/)
    {
	$will_filter = 1;
    }
    if ($will_filter)
    {
	if (m/^env:/)
	{
	    $printing_env = 5;
	    $filtering_env = 1;
	}
    }
    if ($filtering_env)
    {
	if ($printing_env)
	{
	    --$printing_env;
	}
	elsif (m/^done/)
	{
	    $filtering_env = 0;
	}
    }
    if ($filtering_env && (! $printing_env))
    {
	if (! $got_more)
	{
	    $got_more = 1;
	    print "  <other vars>\n";
	}
	next;
    }
    print;
}
