# This "module" is intended to be required by all test suites that
# test abuild.  It contains initialization code that used to be part
# of abuild.test before that was split into smaller test suites.
#
# Sequence:
#
# require TestDriver;
# require abuild_test_utils;
# chdir("dir") or die;
# my $td = new TestDriver("description");
# test_setup();
#
# Note that test_setup() returns a list of messages that should be
# printed from exactly one test suite with $td->emphasize.
#

use Cwd;
use File::Copy;
use File::Basename;
use File::Temp 'tempdir';
use File::Path;

my ($top, $abuild_top, $filters, $atimes_updated, $have_xmllint);
my ($test_java, $test_junit, $jar, $java);
my ($in_windows, $platform_native, $native_out);
my @setup_messages = ();

sub test_setup
{
    _sanitize_environment();

    $top = getcwd();
    $abuild_top = dirname(dirname(dirname($top)));
    if (! -f "$abuild_top/make/abuild.mk")
    {
	die "test_setup() was called from the wrong directory";
    }
    $filters = dirname($top) . "/filters";
    $atimes_updated = _atimes_updated();
    $have_xmllint = _check_xmllint();
    $in_windows = _check_windows();
    ($test_java, $test_junit, $jar, $java) = @{_check_java()};
    $platform_native = _determine_native_platform();
    $native_out = "abuild-$platform_native";

    _do_filter_setup();

    @setup_messages;
}

# Information functions.  XXX Which are needed?

sub get_top
{
    $top;
}

sub get_abuild_top
{
    $abuild_top = dirname(dirname(dirname($top)));
}

sub get_filters
{
    $filters;
}

sub have_xmllint
{
    $have_xmllint;
}

sub in_windows
{
    $in_windows;
}

sub get_java_information
{
    [$test_java, $test_junit, $jar, $java];
}

sub get_native_platform
{
    $platform_native;
}

sub get_native_out
{
    $native_out;
}

# "Public" functions

sub cleanup
{
    cd();
    system("rm -rf work");
}

sub cd
{
    my $dir = shift;
    chdir($top) or die;
    if (defined $dir)
    {
	chdir($dir) or die "can't chdir $dir: $!";
    }
}

sub setup
{
    my ($td, $topdir) = (@_);
    cleanup();
    mkdir 'work', 0777 or die;
    my @dirs = ('.');
    while (@dirs)
    {
	my $d = shift(@dirs);
	my $srcdir = "$topdir/$d";
	my $destdir = "work/$d";
	opendir(D, $srcdir) or die "opendir $srcdir: $!";
	my @entries = grep { ! m/^\.\.?$/ } (sort readdir(D));
	closedir(D);
	if ($d ne '.')
	{
	    mkdir $destdir, 0777 or die "mkdir $destdir: $!";
	}
	foreach my $entry (@entries)
	{
	    # This filtering code should match the corresponding code
	    # in gen-example-list.pl in the doc/manual directory.
	    # Since we verify that all files that pass this filter are
	    # accessed ruing the test suite run, this will ensure that
	    # we are inserting the right files in the documentation.
	    next if $entry =~ m/^\.svn|CVS$/;
	    next if $entry =~ m/\~$/;
	    next if $entry eq '.empty';
	    my $srcpath = "$srcdir/$entry";
	    my $destpath = "$destdir/$entry";
	    if (-d $srcpath)
	    {
		push(@dirs, "$d/$entry");
	    }
	    else
	    {
		copy($srcpath, $destpath) or die "copy $srcpath $destpath: $!";
		if (-x $srcpath)
		{
		    chmod 0755, $destpath;
		}
	    }
	}
    }
    open(F, ">work/.now") or die;
    close(F);
    if ($atimes_updated)
    {
	sleep 1;
    }
    cd("work");
}

sub check_work_accessed
{
    my ($td, $not_accessed) = @_;
    cd();
    my $output = {$td->EXIT_STATUS => 0};
    if (defined $not_accessed)
    {
	$output->{$td->FILE} = $not_accessed;
    }
    else
    {
	$output->{$td->STRING} = "work/.now\n";
    }
    if (-f "work/.now")
    {
	if ($atimes_updated)
	{
	    $td->runtest("make sure all files were read",
			 {$td->COMMAND =>
			      "find work -type f ! -anewer work/.now -print",
			      $td->FILTER => "LANG=C sort"},
			 $output,
			 $td->NORMALIZE_NEWLINES);
	}
	else
	{
	    $td->runtest("skipping access time test",
			 {$td->STRING => "skip"},
			 {$td->STRING => "skip"});
	}
    }
    cleanup();
}

sub windir			# XXX public?
{
    my $dir = shift;
    my $windir = $dir;
    if ($^O eq 'cygwin')
    {
	chop($windir = `cygpath -w $dir`);
	$windir =~ s,\\,/,g;
    }
    $windir;
}

sub validate_dump_data		# XXX public?
{
    my ($td, $extract) = @_;
    $extract = 0 unless defined $extract;
    if ($have_xmllint)
    {
	my $extract_cmd = "";
	if ($extract)
	{
	    $extract_cmd = " | perl $filters/extract-xml.pl";
	}
	$td->runtest("dump-data xml validation",
		     {$td->COMMAND =>
			  "abuild --dump-data" .
			  ($extract ? " 2>/dev/null" : "") .
			  " | perl $filters/extract-xml.pl" .
			  $extract_cmd .
			  " | xmllint --noout --dtdvalid" .
			  " $top/../../abuild_data.dtd -"},
		     {$td->STRING => "",
		      $td->EXIT_STATUS => 0});
    }
    else
    {
	$td->runtest("skipping dump-data xml validation",
		     {$td->STRING => "1"},
		     {$td->STRING => "1"});
    }
}

sub prepend_runtime_pathvar	# XXX public?
{
    # Used by shared library tests
    my ($prepend) = @_;

    # Set variables to be used when adding to our runtime library path for
    # executing things built with shared libraries.
    my $runtime_var = ($in_windows ? 'PATH' : 'LD_LIBRARY_PATH');
    my $old_value = $ENV{$runtime_var} || "";

    my $result =
	join(':', map { "$top/work/$_/$native_out" } @$prepend);
    if ($old_value ne '')
    {
	$result .= ":" . $old_value;
    }
    "$runtime_var=\"$result\" ";
}

sub write_static		# XXX public?
{
    # Used by test10
    sleep 1;
    open(S, ">$top/work/static/string-value") or die;
    print S $_[0], "\n";
    close S;
}

sub create_file			# XXX public?
{
    my $file = shift;
    open(F, ">$file") or die;
    close(F);
}

# Setup functions

sub _sanitize_environment
{
    # Clear environment variables that make exports.  The test suite
    # itself is running from make, and we want make to behave here as if
    # is non-recursive.  Note: deleting from $ENV doesn't remove
    # environment variables from exec'ed programs in cygwin as of perl
    # 5.8.7.
    $ENV{'MAKELEVEL'} = '';
    $ENV{'MAKEFLAGS'} = '';
    $ENV{'MFLAGS'} = '';

    # Make sure any platform selectors are turned off.  We'll delete the
    # environment variable in addition to resetting it so that on those
    # platforms where ENV deletion does work, we'll be testing without the
    # environment variable existing.
    if (exists $ENV{'ABUILD_PLATFORM_SELECTORS'})
    {
	$ENV{'ABUILD_PLATFORM_SELECTORS'} = '';
	delete $ENV{'ABUILD_PLATFORM_SELECTORS'};
    }
    $ENV{'ABUILD_BOOTSTRAP_RELEASE'} = '';
}

sub _atimes_updated
{
    # On some systems, access times are not recorded on files,
    # particularly over network file systems, file systems on flash
    # disks, etc.  Determine whether access times are honored on this
    # system.

    my $dir = tempdir("./" . ('X' x 6), CLEANUP => 1);

    open(F, ">$dir/file1") || die;
    print F "test\n";
    close(F);
    open(F, ">$dir/file2") || die;
    print F "test\n";
    close(F);
    sleep 1;
    open(F, ">$dir/.now") || die;
    close(F);
    sleep 1;
    open(F, "<$dir/file1") or die;
    scalar(<F>);
    close(F);

    # Use find just as we do with the test cases -- this can fail
    # because find doesn't have anewer as well as because atimes
    # aren't preserved.

    my $newer =	`(cd $dir; find . -type f ! -anewer .now -print 2>/dev/null)`;
    my $r = $?;
    $newer = join(' ', sort (split(/\r?\n/, $newer)));
    my $result = 0;
    if ($? == 0)
    {
	if ($newer eq './.now ./file2')
	{
	    $result = 1;
	}
	else
	{
	    push(@setup_messages,
		 "file access times are not updated here;" .
		 " access time tests will be skipped");
	}
    }
    else
    {
	push(@setup_messages,
	     "cannot run find -anewer;" .
	     " access time tests will be skipped");
    }

    rmtree($dir);

    $result;
}

sub _check_xmllint
{
    my $result = 1;
    if (system("xmllint --version >/dev/null 2>&1") != 0)
    {
	$result = 0;
	push(@setup_messages,
	     "xmllint is not present; xml validation tests will be skipped");
    }
    $result;
}

sub _check_windows
{
    # See if we're in Windows.  For now, assume Cygwin means Windows.
    my $result = 0;
    if ($^O =~ m/^(cygwin|MSWin32)$/)
    {
	$result = 1;
    }
    $result;
}

sub _check_java
{
    # Figure out what platforms will be used for native and java builds.

    my $test_java = (! ($ENV{'ABUILD_NO_JAVA'} || 0));
    my $jar = undef;
    my $java = undef;
    my $test_junit = 0;
    if ($test_java)
    {
	if (exists $ENV{'JAVA_HOME'})
	{
	    my $java_home = $ENV{'JAVA_HOME'};
	    if ($^O eq 'cygwin')
	    {
		chop($java_home = `cygpath '$java_home'`);
	    }
	    $jar = "$java_home/bin/jar";
	    $java = "$java_home/bin/java";
	}
	else
	{
	    $jar = "jar";
	    $java = "java";
	}
	if ((exists $ENV{'ABUILD_JUNIT_JAR'}) && (-f $ENV{'ABUILD_JUNIT_JAR'}))
	{
	    $test_junit = 1;
	}
	else
	{
	    push(@setup_messages,
		 "ABUILD_JUNIT_JAR is not set or doesn't point to a file;" .
		 " junit tests will be skipped");
	}
    }

    [$test_java, $test_junit, $jar, $java];
}

sub _determine_native_platform
{
    chop(my $p = `../../../private/bin/bootstrap_native_platform`);
    die "can't determine native platform" unless $? == 0;
    $p =~ m/^[^\.]+\.[^\.]+\.[^\.]+\.[^\.]+$/
	or die "can't parse bootstrap native platform";
    $p;
}

sub _do_filter_setup
{
    # Set variables used by various filter programs.
    $ENV{'ABUILD_PLATFORM_NATIVE'} = $platform_native;
    $ENV{'ABUILD_TEST_TOP'} = "$top/work";
    $ENV{'ABUILD_TEST_DIR'} = $abuild_top;
    $ENV{'ABUILD_WTEST_TOP'} = windir("$top/work");
    $ENV{'ABUILD_WTEST_DIR'} = windir($abuild_top);
}

1;
