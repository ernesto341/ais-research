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

  # The protocol should be the third piece of information but sometimes there
  # is no protocol or there are spaces in the URL which moves its position
  my $found = 0;
  my $i;
  $prot = 0;
  if($cnt > 2) {
    if($parts[2] =~ /HTTP\/(.+)/) {
      $prot = $1;
    }
    # Some URLs contain spaces, first scan ahead and see if any of the 
    # remaining portions of the line have a protocol in them
    else {
      for($i = 3; $i < $cnt; $i++) {
        if($parts[$i] =~ /HTTP\/(.+)/) {
          $found = $i;
          $prot = $1;
          break;
        }
      }
    }
  }
  if($cnt > 1) {
    $req = $parts[1];
    if($found > 0) { # URL contains spaces, rejoin them
      for($i = 2; $i < $found; $i++) {
        $req .= $parts[$i];
      }
    }
    if($req =~ /(.+)/) {
      $req = $1;
    }
  }
  if($cnt == 1) {  # Something is wrong, treat whole URL as request
    $req = $parts[0];
    $cmd = "-";
  }
  else {  # Normal URL, has a HTTP method followed by the request
    $cmd = $parts[0];
  }
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
