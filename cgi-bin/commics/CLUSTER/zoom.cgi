#!/usr/bin/perl -w

use CGI;
use compclusteringengine;

use constant WORK_DIR => "/usr/lib/cgi-bin/commics/work-dir";
use constant IMAGES_DIR => "/var/www/commics/CLUSTER/images";

my $query = new CGI;

my $zoomParams = $query->param('z');
my $fileName = $query->param('file');

my @values = split('-', $zoomParams);
my $coord_y = $values[0];
my $height = $values[1];

my $handler = compclusteringengine::CompClusteringEngine->new(IMAGES_DIR);  
$handler->readFile(WORK_DIR."/data_$fileName.cdt");

my $zoomPath = $handler->paintZoomImage($coord_y,$height);
my $cogsPath = $handler->paintCogs( $coord_y,$height );
my $metagenomesPath = $handler->paintMetagenomes();

print $query->header;

if( $zoomPath eq "" || $cogsPath eq "" || $metagenomesPath eq "" ){ print $handler->getErrorString(); }
else
{
  print "<table width=\"0\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">
  <tr>
  <td></td>
  <td style=\"border-style:none\">
    <div id=\"divMeta\" style=\"border:0; width:450px; height:100px; overflow:hidden; overflow-y:scroll;\">
      <div id=\"metagenome\" style=\"border:0\"> <img src=\"images/$metagenomesPath\" width=\"\" height=\"\" /></div>
    </div>
  </td>
  </tr>
  <tr>
    <td style=\"border-style:none;\">
      <div id=\"divCog\" style=\"border:0; width:60px; height:450px; overflow:scroll; overflow-y:hidden; \" >
        <div id=\"cog\"  style=\"border:0\"> <img src=\"images/$cogsPath\" width=\"\" height=\"\" /></div>
      </div>
    </td>
    <td style=\"border-style:none;\">
      <div id=\"divZoom\" style=\"border:0; width:450px; height:450px; overflow:scroll; overflow-y:scroll;\" onscroll=\"divScroll();\" >
        <div id=\"cluster\"  style=\"border:0\"> <img src=\"images/$zoomPath\" alt=\"clustering data\" width=\"\" height=\"\" id=\"\"/></div>
      </div>
    </td>
  </tr>

</table>"
}


