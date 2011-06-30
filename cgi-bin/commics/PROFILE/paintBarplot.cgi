#!/usr/bin/perl -w

use CGI;
use Statistics::R ;

use constant WORK_DIR => "/usr/lib/cgi-bin/commics/work-dir";
use constant IMAGES_DIR => "/var/www/commics/PROFILE/images";


$query = new CGI;

my $file2open = $query->param('file');
my $metagenome = $query->param('metagenome');

open (IN, WORK_DIR."/cda_$file2open.txt");

@file = <IN>;

my @categories = split("\t",$file[0]);
my @data;
my $metaId;
foreach my $line(@file)
{
  my($id,undef)=split("\t",$line);
  if($id eq $metagenome )
  {
    $metaId = $id;
    @data = split("\t",$line);
    last;
  }
}
close(IN);

my $statement="labels<- c(";
foreach my $label(@categories)
{
  chomp($label);
  $statement .= "\"$label\",";
}
$statement = substr($statement,0,-1); # Remove the last comma
$statement .= ")\n";


$statement .= "bar.data<- c(";
for( my $i = 1; $i < scalar(@data); $i++ )
{
  my $cleanData = $data[$i];
  chomp($cleanData);
  $statement .= "$cleanData,";
}
$statement = substr($statement,0,-1); # Remove the last comma
$statement .= ")\n";

my $catSize = scalar(@categories);
$statement .= "png(file=\"".IMAGES_DIR."/bplot_$file2open$metaId.png\");";
$statement .= "barplot(bar.data,main=\"Relative abundance(%)\", col=rainbow($catSize));"; 
$statement .= "legend(\"topleft\",labels,fill=rainbow($catSize), cex=0.8,bty=\"n\");";
$statement .= "dev.off()";

my $R = Statistics::R->new() ; #It might be needed to redirect stderr
$R->startR ;
$R->send($statement) ;
$R->stopR() ;

print $query->header;
print "<div id=\"barplotimg\" style=\"border:0\"> <img src=\"images/bplot_$file2open$metaId.png\" width=\"\" height=\"\" /></div>";
