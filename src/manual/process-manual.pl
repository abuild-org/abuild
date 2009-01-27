#!/usr/bin/env perl

require 5.008;
BEGIN { $^W = 1; }
use strict;
use File::Basename;
use IO::File;

my $whoami = ($0 =~ m,([^/\\]*)$,) ? $1 : $0;
my $dirname = ($0 =~ m,(.*)[/\\][^/\\]+$,) ? $1 : ".";

my $tabwidth = 8;

die "Usage: $whoami in out\n" unless @ARGV == 2;
my ($in, $out) = @ARGV;
open(IN, "<$in") or die "$whoami: can't open $in: $!\n";
open(OUT, ">$out") or die "$whoami: can't create $out: $!\n";

my %dirs =
    ('example' => "../../doc/example",
     'qtest' => "../qtest/abuild-examples",
    );

my $last_id = undef;
my @examples = ();

while (<IN>)
{
    if (m/<\?(example|qtest)/)
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
    elsif (m/<\?dump-data\?>/)
    {
	generate_dump_data();
    }
    elsif (m/<\?interface-dump\?>/)
    {
	generate_interface_dump();
    }
    elsif (m/<\?ccxx-mk\?>/)
    {
	generate_ccxx_mk();
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

sub generate_dump_data
{
    print OUT "<programlisting><![CDATA[";
    open(F, "<../../abuild_data.dtd") or
	die "$whoami: can't open abuild_data.dtd: $!\n";
    while (<F>)
    {
	print OUT process_line($_);
    }
    close(F);
    print OUT "]]></programlisting>\n";
}

sub generate_interface_dump
{
    print OUT "<programlisting><![CDATA[";
    open(F, "<../../interface_dump.dtd") or
	die "$whoami: can't open interface_dump.dtd: $!\n";
    while (<F>)
    {
	print OUT process_line($_);
    }
    close(F);
    print OUT "]]></programlisting>\n";
}

sub generate_ccxx_mk
{
    print OUT "<programlisting><![CDATA[";
    open(F, "<../../../make/rules/object-code/ccxx.mk") or
	die "$whoami: can't open ccxx.mk: $!\n";
    while (<F>)
    {
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
