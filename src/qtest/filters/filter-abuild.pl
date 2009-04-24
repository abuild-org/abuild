BEGIN { $^W = 1; }
use strict;

my $topdir = $ENV{'ABUILD_TEST_TOP'} or die "ABUILD_TEST_TOP not defined";
my $abuild_dir = $ENV{'ABUILD_TEST_DIR'} or die "ABUILD_TEST_DIR not defined";
my $wtopdir = $ENV{'ABUILD_WTEST_TOP'} or die "ABUILD_TEST_TOP not defined";
my $wabuild_dir = $ENV{'ABUILD_WTEST_DIR'} or
    die "ABUILD_TEST_DIR not defined";
my $platform_native = $ENV{'ABUILD_PLATFORM_NATIVE'} or
    die "ABUILD_PLATFORM_NATIVE is not defined";

$platform_native =~ m/^([^\.]+\.[^\.]+\.[^\.]+)\.([^\.]+)$/
    or die "platform_native is malformed";
my ($os_data, $compiler) = ($1, $2);

my $in_autoconf = 0;
my $in_stacktrace = 0;
my $saw_junitreport = 0;
my $saw_javadoc = 0;

while (<>)
{
    if ($in_autoconf)
    {
	if (m/--> TEST_MESSAGE: /)
	{
	    # Let it go
	}
	elsif (m,Leaving directory.*/autoconf/,)
	{
	    print "[autoconf output suppressed]\n";
	    $in_autoconf = 0;
	}
	else
	{
	    next;
	}
    }
    if ($in_stacktrace)
    {
	if (m/--end stack trace--/)
	{
	    $in_stacktrace = 0;
	    print "--JAVA STACK TRACE--\n";
	}
	next;
    }

    if (m/^abuild:/)
    {
	$saw_junitreport = 0;
	$saw_javadoc = 0;
    }

    s,\\,/,g;
    # Normalize exit code of make
    s,(make:.*Error) (\d+)$,$1 1,;
    s/($topdir|(?i:$wtopdir))/--topdir--/g;
    s/($abuild_dir|(?i:$wabuild_dir))/--abuild-dir--/g;
    s,(--abuild-dir--/(?:make|rules)/.*:)\d+,${1}nn,;
    s/$platform_native/<native>/g;
    s/$os_data/<native-os-data>/;
    next if (m/^[^\s\/]+\.(c|cc|cpp)\r?$/); # Filter out VC++'s output
    # Skip VC++'s DLL creation output
    next if m/Creating library .*\.lib and object .*\.exp/i;
    # Filter junitreport
    if (m/\[junitreport\]\s/)
    {
	if (! $saw_junitreport)
	{
	    print "[junitreport]...\n";
	    $saw_junitreport = 1;
	}
	next;
    }
    # Filter javadoc
    if (m/\[javadoc\]\s/)
    {
	if (! $saw_javadoc)
	{
	    print "[javadoc]...\n";
	    $saw_javadoc = 1;
	}
	next;
    }
    s,--abuild-dir--.*abuild.xml,--abuild.xml--,;
    s,(--abuild.xml--):(\d+),$1:nn,;
    s/^(Total time: ).*/$1<time>/;
    s/(\[junit\].*Time elapsed: ).*/$1<time>/;
    s/(tree.\d+.-)\d+(-)/${1}RND${2}/g;
    next if m/^abuild: total build time: /;
    if (m,--begin stack trace--,)
    {
	$in_stacktrace = 1;
	next;
    }

    print;

    if (m,Entering directory.*/autoconf/,)
    {
	$in_autoconf = 1;
    }
}
