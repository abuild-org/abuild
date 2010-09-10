use warnings;
use strict;

$| = 1;

my $run_child = 1;
if (@ARGV && ($ARGV[0] eq '-no-child'))
{
    $run_child = 0;
}

my $indent = $run_child ? "" : "  ";

print "${indent}line 1\n";
pause();
print "${indent}out";
warn "${indent}error\n";
pause();
print "put\n";
if ($run_child)
{
    print "${indent}running child\n";
    system("perl $0 -no-child");
}
else
{
    print "${indent}not running child\n";
}
print "${indent}bye\n";

sub pause
{
    select(undef, undef, undef, 0.25);
}
