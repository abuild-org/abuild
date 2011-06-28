#!/usr/bin/env perl

# This script creates an abuild area with lots of build items and
# dependencies.  It can be used for performance testing.  The basic
# scheme is that we create build items numbered from 2 to 1000 with
# each build item's path related to its prime factors.  Build items
# depend upon some of their factors.  This is just a mechanism to
# enable us to easily construct a lot of build items and dependencies
# without having to worry about cycles.  Once this is set up, you can
# go to a build item's directory and run various abuild commands to
# measure how much time abuild spends doing its validations, etc.

require 5.008;
use warnings;
use strict;
use File::Basename;

my $whoami = basename($0);

my @primes = (2);
primes();

my $top = 'abuild-performance-test';
system("rm -rf $top");

foreach (my $i = 2; $i < 1000; ++$i)
{
    my @factors = factor($i);
    my $path = "$top/backing";
    while (@factors)
    {
	$path .= "/" . shift(@factors);
	mkdir_p($path, 0777) or die;
    }
    $path .= "/$i";
    mkdir_p($path, 0777);
    open(F, ">$path/Abuild.conf") or die;
    print F "this: $i\n";
    my @deps = ();
    my $toggle = 0;
    for (my $j = 2; $j < $i; ++$j)
    {
	if ($i == $j * int($i / $j))
	{
	    push(@deps, $j) if $toggle;
	    $toggle = 1 - $toggle;
	}
    }
    if (@deps)
    {
	print F "deps: ", join(' ', @deps), "\n";
    }
    close(F);
}
my @dirs = ("$top/backing");
while (@dirs)
{
    my $dir = shift(@dirs);
    opendir(D, $dir) or die;
    my @entries = readdir(D) or die;
    closedir(D);
    my @children = ();
    foreach my $e (@entries)
    {
	next if $e eq '.';
	next if $e eq '..';
	my $full = "$dir/$e";
	if (-d $full)
	{
	    push(@dirs, $full);
	    push(@children, $e);
	}
    }
    open(F, ">>$dir/Abuild.conf") or die;
    if ($dir ne "$top/backing")
    {
	print F "parent-dir: ..\n";
    }
    if (@children)
    {
	print F "child-dirs: ", join(' ', @children), "\n";
    }
    close(F);
}
system("cp -a $top/backing $top/main");
system("rm -rf $top/main/[579]??");
open(F, ">$top/main/Abuild.backing") or die;
print F "rw: ../backing\n";
close(F);

print "Now cd $top/main/2/2 and run various abuild commands\n";

sub primes
{
  loop:
    for (my $i = 3; $i < 1000; ++$i)
    {
	foreach my $p (@primes)
	{
	    next loop if $i == ($p * int($i / $p));
	}
	push(@primes, $i);
    }
}

sub factor
{
    my $n = shift;
    my @result = ();
    foreach my $p (@primes)
    {
	while ($n == ($p * int($n / $p)))
	{
	    push(@result, $p);
	    $n /= $p;
	}
    }
    @result;
}

sub mkdir_p
{
    # Make all directories down to specified point.  It is not an
    # error if the directory already exists.
    my $dir = shift;
    my $mode = shift;
    my $cur_path = (($dir =~ s,^/,,) ? "/" : "");
    my @builditems = split('/', $dir);
    my $r = 1;
    while (@builditems)
    {
	$cur_path .= shift(@builditems);
	($r = mkdir $cur_path, $mode) unless -d $cur_path;
	last unless $r;
	$cur_path .= "/";
    }
    $r;
}
