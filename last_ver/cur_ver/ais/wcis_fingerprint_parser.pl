#!/usr/bin/perl

# Order of information:
#    http command
#    http protocol
#    request length
#    number of input vars
#    number of % chars
#    number of ' chars
#    number of + chars
#    number of .. chars
#    number of \ chars
#    number of ( chars
#    number of ) chars
#    number of // chars
#    number of < chars
#    number of > chars

die "Usage: $0 <input_file> <output_file>" if $#ARGV < 1;

my $infile = $ARGV[0];
my $outfile = $ARGV[1];

open(INP, $infile) or die "Cannot open file $normal: $!\n";

open(FOUT, ">>$outfile") or die "Cannot open $outfile for append: $!\n";

my $file, $len, $prot, $vars, $cmd, $perc, $apos, $plus, $cdot, $back, 
   $oparen, $cparen, $forward, $lt, $gt;

while($line = <INP>) {
  chomp($line);
  parse_line($line);
  print FOUT "$file $cmd $prot $len $vars $perc $apos $plus $cdot $back $oparen $cparen $forward $lt $gt\n";
}


sub parse_line {
  my $line = shift(@_);

  my @parts = split(/ /, $line);
  my $cnt = scalar(@parts);
  my $req = "-";
  $vars = 0;

  if($cnt > 2) {
    if($parts[2] =~ /HTTP\/(.+)/) {
      $prot = $1;
    }
    else {
      $prot = 0;
    }
  }
  else {
    $prot = 0;
  }
  if($cnt > 1) {
    $req = $parts[1];
    if($req =~ /(.+)/) {
      $req = $1;
    }
  }
  $cmd = $parts[0];
  $len = length($req);

  @parts = split(/\?/, $req);
  $file = $parts[0];

  if(scalar(@parts) > 1) {
    my $tmp = $parts[1] . " a";
    $vars = scalar(split(/\&/, $tmp));
  }

  $req = $req . " a";
  $perc = scalar(split(/%/, $req)) - 1;
  $apos = scalar(split(/'/, $req)) - 1;
  $plus = scalar(split(/\+/, $req)) - 1;
  $cdot = scalar(split(/\.\./, $req)) - 1;
  $back = scalar(split(/\\/, $req)) - 1;
  $oparen = scalar(split(/\(/, $req)) - 1;
  $cparen = scalar(split(/\)/, $req)) - 1;
  $forward = scalar(split(/\/\//, $req)) - 1;
  $lt = scalar(split(/</, $req)) - 1;
  $gt = scalar(split(/>/, $req)) - 1;
}
