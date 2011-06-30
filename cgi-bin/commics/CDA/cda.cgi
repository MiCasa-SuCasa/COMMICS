#!/usr/bin/perl -w
use CGI;
use DBI;
use Statistics::R ;

use constant WORK_DIR => "/usr/lib/cgi-bin/commics/work-dir";
use constant IMAGES_DIR => "/var/www/commics/CDA/images";
use constant DB_HOST => "localhost";
use constant DB_NAME => "METAGENOMESIMG";
use constant DB_USER => "commics";
use constant DB_PASS => "";
use strict;

my $query = new CGI;

my $dsn = "DBI:mysql:".DB_NAME.":".DB_HOST;
my $dbh = DBI->connect($dsn, DB_USER, DB_PASS)
                or die "Couldn't connect to database: " . DBI->errstr;

my $typeEnvh = $dbh->prepare('select HABITAT_SUPERTYPE, HABITAT_TYPE, HABITAT_SUBTYPE from metadata where META_ID = ?')
                or die "Couldn't prepare statement: " . $dbh->errstr; 

my $file2open = $query->param('file');
my $envType = $query->param('type');

open (OUTPUTFILE, ">".WORK_DIR."/cda_$file2open");
open(FILEH, WORK_DIR."/$file2open") || die("Could not open the specified file!");
my @file_content = <FILEH>;
close(FILEH);

my @cog_categories;
my $count = 0;
my %hashTypes;
my $typesCount = 1;
my @rPchs;
my $categories;
foreach my $line( @file_content )
{
  chomp($line);
  if( 0 == $count)
  {
    $categories = $line;
    print OUTPUTFILE "$line\tEnvironment\n";
  }	
  else
  {
    my (@lineArr) = split("\t", $line); 
    my $metaid = $lineArr[0];
    $typeEnvh->execute($metaid);
    my($superType, $type,$subType) = $typeEnvh->fetchrow_array(); 
    my $rows = $typeEnvh->rows;
    if( $rows > 0 )
    {
      $superType =~ s/\s+/\./g;
      $superType =~ s/\/+//g;
      $type =~ s/\s+/\./g;
      $type =~ s/\/+//g;
      $subType =~ s/\s+/\./g;
      $subType =~ s/\/+//g;
    }
    $typeEnvh->finish;
    print OUTPUTFILE "$line\t";

    my $type2print;
    if($rows > 0)
    {
      if( $envType eq "Supertype")
      {
        $type2print = "$superType";
      }
      elsif( $envType eq "Type")
      {
        $type2print = "$superType\_$type";
      }
      elsif( $envType eq "Subtype")
      {
        $type2print = "$superType\_$type\_$subType";
      }
    }
    else
    {
      $type2print = "User_Data";
    }

    print OUTPUTFILE "$type2print\n";

    if( defined( $hashTypes{$type2print}) )
    {
      push( @rPchs, $hashTypes{$type2print});
    }
    else
    {
      $hashTypes{$type2print} = $typesCount;
      push( @rPchs, $hashTypes{$type2print});
      $typesCount++;
    }

  }
  $count++;

}
close (OUTPUTFILE); 
$dbh->disconnect;

$categories =~ s/\t/,/g;

my $statement1 = "library(candisc)\n";
$statement1 .= "input.data<-data.frame(read.table(\"".WORK_DIR."/cda_$file2open\"))\n";
$statement1 .= "input.mod <- lm(cbind($categories) ~ Environment, data=input.data)\n";
$statement1 .= "input.can <- candisc(input.mod, data=input.data)\n";

my $R = Statistics::R->new() ; #It might be needed to redirect stderr
$R->startR ;
$R->send($statement1) ;
my $ret = $R->read ;
$ret =~ s/</[/g;
$ret =~ s/>/]/g;

my $statement2 .= "d.colors<-colors()\n";
my $vector = join(",", @rPchs);
$statement2 .= "pch <-c($vector)\n";

my $colorVector;
foreach my $pch( @rPchs )
{
  my $number = $pch*11;
  $colorVector .= "d.colors[$number],";
}
$colorVector = substr($colorVector,0,-1); 

$statement2 .= "col <-c($colorVector)\n";
my $image = substr($file2open,0,-4); #remove extension .txt
$image = substr($envType,0,3).$image;#prefix of the environment type
$statement2 .= "png(file=\"".IMAGES_DIR."/$image.png\"); plot(input.can, col=col, pch=pch); dev.off()\n"; # These three commands must be in the same line
                                                                                                         # otherwise, it won't work (???)
                                                                                                         # http://tolstoy.newcastle.edu.au/R/e2/help/07/03/11903.html

$R->send($statement2) ;
$R->stopR() ;

print $query->header;

if( -e IMAGES_DIR."/$image.png" )
{
  print "<div id=\"cdaimg\" style=\"border:0\"> <img src=\"images/$image.png\" width=\"\" height=\"\" /></div>";
}
else
{
  print "<div id=\"cdaimg\" style=\"border:0\">$ret</div>";
}
