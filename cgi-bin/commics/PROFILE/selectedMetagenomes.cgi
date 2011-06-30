#!/usr/bin/perl -w

use CGI;

$query = new CGI;

use constant WORK_DIR => "/usr/lib/cgi-bin/commics/work-dir";

my $file2open = $query->param('file');

open (IN, WORK_DIR."/info_cda_$file2open.txt");

print $query->header;
if(tell(IN) != -1)
{
  @file = <IN>;

  print "<select id=\"metagenomes\">";
  foreach my $line(@file)
  {
    my($id,$name)=split("\t",$line);
    print "<option value=\"$id\">$name</option>\n";
  }

  print "</select>\n";

  close(IN);
}
else
{
print "<select id=\"metagenomes\">";
print "</select>\n";
}


