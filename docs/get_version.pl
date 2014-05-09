eval '(exit $?0)' && eval 'exec perl -w -S $0 ${1+"$@"}'
    & eval 'exec perl -w -S $0 $argv:q'
    if 0;
    
use File::Basename;
my $path = dirname(__FILE__);
my $parent = $path . "/..";
my $source = "$parent/VERSION.txt";
my $target = "$path/VERSION.txt";


###################################################
# @author   James Edmondson <jedmondson@gmail.com>
# 
# This perl script is useful for generating a dynamic
# project number to be used by Doxygen in its help file
# generation process.
###################################################
    
open VERSION_FILE, "< $source" or
    die "Unable to open $source for reading.";
  $version = <VERSION_FILE>;
close VERSION_FILE;

open OUTPUT_FILE, ">$target" or
    die "Unable to open $target for writing.";
  print OUTPUT_FILE  "PROJECT_NUMBER = " . $version . "\n";
close OUTPUT_FILE;

