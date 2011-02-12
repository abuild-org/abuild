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
my $got_more = 0;

while (<>)
{
    s/$top/--top--/;
    s,$root,/,;
    if (m/env = both/)
    {
	$will_filter = 1;
    }
    # Unconditionally filter protected variables
    next if m/LD_LIBRARY_PATH=/;
    next if m/SYSTEMROOT=/;
    my $suppress = 0;
    if ($filtering_env)
    {
	if (m/^done/)
	{
	    print "  <other vars>\n" if $got_more;
	    $filtering_env = 0;
	    $got_more = 0;
	}
	elsif (! m/:qww:/)
	{
	    if ((! $got_more) && (m/=/))
	    {
		$got_more = 1;
	    }
	    $suppress = 1;
	}
    }
    elsif ($will_filter && (m/^env:/))
    {
	$filtering_env = 1;
    }
    print unless $suppress;
}
