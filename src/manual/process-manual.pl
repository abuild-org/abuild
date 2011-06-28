#!/usr/bin/env perl

require 5.008;
use warnings;
use strict;
use File::Basename;
use IO::File;

my $whoami = basename($0);
my $dirname = dirname($0);

my $tabwidth = 8;

die "Usage: $whoami in out\n" unless @ARGV == 2;
my ($in, $out) = @ARGV;
open(IN, "<$in") or die "$whoami: can't open $in: $!\n";
open(OUT, ">$out") or die "$whoami: can't create $out: $!\n";

my $top = '../../..';

my %dirs =
    ('example' => "../../doc/example",
     'qtest' => "../qtest/abuild-examples",
    );

my $last_id = undef;
my @examples = ();

my $stripping = 0;

while (<IN>)
{
    if ($stripping)
    {
	if (m/<\?end-strip\?>/)
	{
	    $stripping = 0;
	}
	next;
    }
    elsif (m/<\?strip\?>/)
    {
	$stripping = 1;
    }
    elsif (m/<\?(example|qtest)/)
    {
	if (! m/^\s*<\?(example|qtest) (\S+)\?>$/)
	{
	    die "$in:$.: malformed processing instruction\n";
	}
	process($1, $2);
    }
    elsif (m/<\?list-of-examples\?>/)
    {
	generate_example_list();
    }
    elsif (m/<\?include-file (\S+)\?>/)
    {
	include_file($1);
    }
    elsif (m/<\?help-files\?>/)
    {
	generate_help_files();
    }
    else
    {
	if (m/<(?:chapter|sect\d)\s*id=\"([^\"]+)\"/)
	{
	    $last_id = $1;
	    if ($last_id =~ m/^ref\.example\./)
	    {
		push(@examples, $last_id);
	    }
	}
	print OUT;
    }
}
close(IN);
close(OUT);

sub process
{
    my ($what, $file) = @_;

    if ($last_id !~ m/^ref\.example\./)
    {
	die "$in:$.: examples must appear in a section whose ID starts" .
	    " with ref.example\n";
    }

    my @xargs = ();
    if ($what eq 'qtest')
    {
	push(@xargs, 77, 72);
    }

    my $dir = "$dirname/$dirs{$what}";
    my $path = "$dir/$file";
    my $fh = new IO::File("<$path") or
	die "$in:$.: can't open $what $file: $!\n";
    print OUT "
<literallayout><emphasis><emphasis role=\"strong\">$file</emphasis></emphasis>
</literallayout>
<programlisting>\<![CDATA[";
    while (<$fh>)
    {
	print OUT process_line($_, @xargs);
    }
    print OUT "]]></programlisting>
";
    $fh->close();
}

sub generate_example_list
{
    print OUT "<simplelist>\n";
    foreach my $e (@examples)
    {
	print OUT "<member><xref linkend=\"" . $e . "\"/></member>\n";
    }
    print OUT "</simplelist>\n";
}

sub generate_help_files
{
    my @help_topics = (<$top/help/*.txt>);
    my @rule_help = (<$top/rules/*/*-help.txt>);
    my @toolchain_help = (<$top/make/toolchains/*-help.txt>);

    for (@help_topics)
    {
	my $base = help_base($_);
	print OUT "<sect1 id=\"ref.help.topic.$base\">\n";
	print OUT "<title><command>abuild --help $base</command></title>\n";
	print OUT "<para>\n";
	include_file($_, 1, 1);
	print OUT "</para>\n";
	print OUT "</sect1>\n";
    }
    for (@rule_help)
    {
	my $base = help_base($_);
	print OUT "<sect1 id=\"ref.help.rule.$base\">\n";
	print OUT "<title><command>abuild --help rules rule:$base</command></title>\n";
	print OUT "<para>\n";
	include_file($_, 1, 1);
	print OUT "</para>\n";
	print OUT "</sect1>\n";
    }
    for (@toolchain_help)
    {
	my $base = help_base($_);
	print OUT "<sect1 id=\"ref.help.toolchain.$base\">\n";
	print OUT "<title><command>abuild --help rules toolchain:$base</command></title>\n";
	print OUT "<para>\n";
	include_file($_, 1, 1);
	print OUT "</para>\n";
	print OUT "</sect1>\n";
    }
}

sub help_base
{
    my $file = shift;
    my $base = basename($file);
    $base =~ s/-help.txt$//;
    $base =~ s/\.txt$//;
    $base;
}

sub include_file
{
    my ($file, $with_top, $strip_comments) = @_;
    $with_top = 0 unless defined $with_top;
    $strip_comments = 0 unless defined $strip_comments;
    if (! $with_top)
    {
	$file = "$top/$file";
    }
    print OUT "<programlisting><![CDATA[";
    open(F, "<$file") or
	die "$whoami: can't open $file: $!\n";
    while (<F>)
    {
	next if $strip_comments && m/^\#/;
	print OUT process_line($_, 80, 72);
    }
    close(F);
    print OUT "]]></programlisting>\n";
}

sub process_line
{
    my ($line, $maxlength, $chunklength) = @_;

    # expand tabs
    my $new = "";
    while ($line =~ m/\t/g)
    {
	$new .= $`;
	$new .= (' ') x ($tabwidth - (length($new) % $tabwidth));
	$line = $'; #'
    }
    $line = $new . $line;

    if (defined $maxlength)
    {
	$new = "";
	while (length($line) > $maxlength)
	{
	    $new = substr($line, 0, $chunklength) . "\\\n\\";
	    $line = substr($line, $chunklength);
	}
	$line = $new . $line;
    }

    $line;
}
