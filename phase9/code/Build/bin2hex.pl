#!/usr/bin/perl
# bin2hex.pl, simple Perl script to convert ?.out to ?.out.hex
# W. Chang, copyfree 2012

$#ARGV == 0 || die "Usage: bin2hex.pl <input filename>\n";

$in_file = $ARGV[0];
$out_file = "$in_file.hex";
print "Input: $in_file, output: $out_file\n";

open IN, "< $in_file" || die "Can't read $in_file,"; # read from IN
open OUT, "> $out_file" || die "Can't write $out_file,"; # write to OUT

binmode IN; # will read as binary data

$size = 0;
$bytes = "";
while (($n = read IN, $byte, 1) != 0)
{
   $bytes .= $byte;
   $size++;
}

for $i (0 .. $size - 1) # from byte 0 to size-1 ($i can start from 128)
{
   next if($i < 128); # skip 1st 128 bytes (byte 0..127), the header info

   $byte = substr($bytes, $i, 1); # get a char in the string (byte stream)

   $str = sprintf(" 0x%x", ord($byte)); # convert output format to hex with sprintf

   print OUT $str;

   print OUT "," unless($i == $size - 1); # separate numbers with , unless last

   if((($i + 1) % 16 == 0) || ($i == $size - 1))
   {
      print OUT "\n"; # every 16 hex numbers or the last number, add a newline
   }
}

close OUT; # close output file handle

