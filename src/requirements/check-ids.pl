#!/usr/bin/env perl

require 5.008;
BEGIN { $^W = 1; }
use strict;
use File::Basename;

my $whoami = basename($0);
my $dirname = dirname($0);

die "Usage: $whoami xml-file\n" unless @ARGV == 1;
my $file = shift(@ARGV);

my $errors = 0;

my $last_id = '0';
open(P, "xsltproc $dirname/extract-id.xsl $file |")
    or die "$whoami: can't run xsltproc\n";
while (<P>)
{
    chomp;
    check_follower($last_id, $_);
    $last_id = $_;
}
close(P);
exit 2 if $errors;

sub check_follower
{
    my ($old, $new) = @_;
    my @old = split('\.', $old);
    my @new = split('\.', $new);
    if (scalar(@old) < scalar(@new))
    {
	if (pop(@new) ne '1')
	{
	    error("$new is longer than $old but last component is not 1");
	}
	if (! lists_equal(\@old, \@new))
	{
	    error("$new is longer than $old but their heads don't match");
	}
    }
    else
    {
	while (scalar(@old) > scalar(@new))
	{
	    pop(@old);
	}
	if (pop(@new) != (pop(@old) + 1))
	{
	    error("$new is not a successor to old's head");
	}
	if (! lists_equal(\@old, \@new))
	{
	    error("$new is shorter than or equal to old but their" .
		  " heads don't match");
	}
    }
}

sub lists_equal
{
    my ($lh, $rh) = @_;
    my $result = 1;
    if (scalar(@$lh) == scalar(@$rh))
    {
	for (my $i = 0; $i < @$lh; ++$i)
	{
	    if ($lh->[$i] ne $rh->[$i])
	    {
		$result = 0;
	    }
	}
    }
    else
    {
	$result = 0;
    }
    $result;
}

sub error
{
    my $msg = shift;
    warn "$whoami: $msg\n";
    $errors = 1;
}
