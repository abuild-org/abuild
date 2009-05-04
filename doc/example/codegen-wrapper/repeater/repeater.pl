BEGIN { $^W = 1; }
use strict;
use File::Basename;
my $whoami = basename($0);

my $in = undef;
my $out = undef;
while (@ARGV)
{
    my $arg = shift(@ARGV);
    if ($arg eq '-i')
    {
	die "-i requires an argument" unless @ARGV;
	$in = shift(@ARGV);
    }
    elsif ($arg eq '-o')
    {
	die "-o requires an argument" unless @ARGV;
	$out = shift(@ARGV);
    }
    else
    {
	usage();
    }
}
usage() unless ((defined $in) && (defined $out));

open(I, "<$in") or die "$whoami: can't open $in: $!\n";
open(O, ">$out") or die "$whoami: can't open $out: $!\n";
while (<I>)
{
    if (s/^\[repeat (\d+)\]\s*//)
    {
	for (my $i = 0; $i < $1; ++$i)
	{
	    print O $_;
	}
    }
    else
    {
	print O $_;
    }
}
close(O);
close(I);

sub usage
{
    die "Usage: $whoami -i in -o out\n";
}
