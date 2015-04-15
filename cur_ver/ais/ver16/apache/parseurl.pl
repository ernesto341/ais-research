#!/usr/bin/perl

die "Usage: $0 <input_file> <output_file>" if $#ARGV < 1;

my $infile = $ARGV[0];
my $outfile = $ARGV[1];

open(INP, $infile) or die "Cannot open file $infile: $!\n";

open(FOUT, ">>$outfile") or die "Cannot open $outfile for append: $!\n";

while($line = <INP>) {
  chomp($line);
  if($line =~ /^.*\"([^"]*)\"\s+\d+.*/) {
    if(int(rand(10)) < 2) {
      print FOUT "$1\n";
    }
  }
  else {
    print "Could not extract URL from $line\n";
  }
}

