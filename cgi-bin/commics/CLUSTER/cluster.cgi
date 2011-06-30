#!/usr/bin/perl -w
use CGI;
use compclusteringengine;

use constant WORK_DIR => "/usr/lib/cgi-bin/commics/work-dir";
use constant IMAGES_DIR => "/var/www/commics/CLUSTER/images";

$query = new CGI;

$clusterParams = $query->param('w');
$fileName = $query->param('file');

print $query->header;

my $process_result = system("cluster -v 1>/dev/null");

if( -1 == $process_result)
{
  print "cluster is not correctly installed in the system\n";
}
else
{
  my $cluster_query = "cluster $clusterParams -f ".WORK_DIR."/data_$fileName.txt";

  my $cluster_message = `$cluster_query`;

  if(  "" ne $cluster_message )
  {
    print "Message from cluster: $cluster_message\n";
  }

  my $handler = compclusteringengine::CompClusteringEngine->new(IMAGES_DIR);  
  my $cdtFile = "data_$fileName.cdt";

  while( $clusterParams =~ /(k\s+\d+)/g )
  {
    my $query = $1;
    if( $query =~ /(\d+)/g)
    {
      $cdtFile = "data_$fileName\_K\_G$1\_A$1.cdt";
    }
  }#while

  $handler->readFile(WORK_DIR."/$cdtFile");

  my $clusterPic = $handler->paintClusteringData();
  my $hTreePic = $handler->paintHorizontalTree();
  my $vTreePic= $handler->paintVerticalTree();
  my $scalePic = $handler->paintScale();
  my $clusterPicWidth = $handler->getDataImgWidth;
  my $clusterPicHeight = $handler->getDataImgHeight;

  if($clusterPic eq ""){print $handler->getErrorString();}
  else
  {
    print "$clusterPicWidth&$clusterPicHeight|<table width=\"0\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">
    <tr>
    <td></td>
    <td style=\"border-style:none|\">
    <div id=\"divHTree\" style=\"border:0; width:310px; height:100px; overflow:hidden; overflow-y:scroll;\">";
    if( $hTreePic ne "")
    {
      print "<div id=\"htree\" style=\"border:0\"> <img src=\"images/$hTreePic\" width=\"\" height=\"\" /></div>";
    }
    print "</div>
      </td>
      </tr>
      <tr>
      <td style=\"border-style:none;\">
      <div id=\"divVTree\" style=\"border:0; width:120px; height:450px; overflow:scroll; overflow-y:hidden; \" >";
    if( $vTreePic ne "" )
    {
      print "<div id=\"vtree\"  style=\"border:0\"> <img src=\"images/$vTreePic\" width=\"\" height=\"\" /></div>";
    }
    print "</div>
      </td>
      <td style=\"border-style:none;\">
        <div id=\"divCluster\" style=\"border:0; width:310px; height:450px; overflow:scroll; overflow-y:scroll;\" onscroll=\"clusterScroll();\" >
        <div id=\"cluster\"  style=\"border:0\"> <img src=\"images/$clusterPic\" alt=\"clustering data\" width=\"$clusterPicWidth\" height=\"$clusterPicHeight\" id=\"cluster_image\"/></div>
        </div>
      </td>
    </tr>
    <tr>
      <td></td>
      <td><center><div><img src=\"images/$scalePic\"  /></div></center></td>
    </tr>
  </table>"
  }
}


