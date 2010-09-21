use warnings;
use strict;

while (<>)
{
    s/(list_platforms --native-data)(.*?)(failed)(.*)(\r?\n)$/$1...$3...$5/;
    print;
}
