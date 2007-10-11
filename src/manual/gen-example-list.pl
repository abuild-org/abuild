BEGIN { $^W = 1; }
use strict;
use File::Basename;
use File::stat;

my $dirname = dirname($0);

my @files = (get_files("$dirname/../../doc/example"),
	     (grep { $_->[0] =~ m,/example\..*\.out$, }
	      get_files("$dirname/../qtest/abuild")));
open(E, ">example-list.new") or die "open example-list.new: $!";
for (@files)
{
    print E join(' ', @$_), "\n";
}
close(E);
if (system("cmp -s example-list.new example-list") != 0)
{
    rename "example-list.new", "example-list";
}

sub get_files
{
    my $topdir = shift;
    my @dirs = ($topdir);
    while (@dirs)
    {
	my $d = shift(@dirs);
	opendir(D, $d) or die "opendir $d: $!";
	my @entries = grep { ! m/^\.\.?$/ } (sort readdir(D));
	closedir(D);
	foreach my $entry (@entries)
	{
	    # This filtering code should match the corresponding code
	    # in the setup method of abuild.test.  That way, if the
	    # test suite passes, we know we're getting the right
	    # files.  (The test suite ensures that all files that
	    # match this filter have are accessed when it is run.)
	    next if $entry =~ m/^\.svn|CVS$/;
	    next if $entry =~ m/\~$/;
	    next if $entry eq '.empty';
	    my $fullpath = "$d/$entry";
	    if (-d $fullpath)
	    {
		push(@dirs, $fullpath);
	    }
	    else
	    {
		my $st = stat($fullpath) or die "stat $fullpath: $!";
		push(@files, [$fullpath, $st->mtime]);
	    }
	}
    }
    sort { $a->[0] cmp $b->[0] } @files;
}
