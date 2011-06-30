#!/usr/bin/perl -w
  use DBI;
  use CGI;

$query = new CGI;


print $query->header;
  my $dbhost ='localhost';
  my $dbname ='METAGENOMESIMG';	
				
  my $dbuser ='commics';	
  my $dbpass ='';


  my $dsn = "DBI:mysql:$dbname:$dbhost";
  my $dbh = DBI->connect($dsn, $dbuser, $dbpass)
                or die "Couldn't connect to database: " . DBI->errstr;


  my $selecth = $dbh->prepare('SELECT META_ID, MICROBIOME_NAME, HABITAT_SUPERTYPE, HABITAT_TYPE, HABITAT_SUBTYPE,HABITAT_EXTRA FROM metadata')
                or die "Couldn't prepare statement: " . $dbh->errstr;

  $selecth->execute()
    or die "Couldn't execute statement: " . $selecth->errstr; 

  # BIND TABLE COLUMNS TO VARIABLES
  my ($metaId, $microbiomeName, $habitat_super,$habitat_type, $habitat_sub,$habitat_extra);
  $selecth->bind_columns(\$metaId, \$microbiomeName, \$habitat_super,\$habitat_type, \$habitat_sub, \$habitat_extra);

  # LOOP THROUGH RESULTS







print <<ENDHTML;
<table id "sorted_table" class="sorted regroup" cellspacing="0" cellpadding="0"> 
				<thead> 
					<tr> 
						<th id="metaid" class="sortedplus"><span>Id</span></th> 
						<th id="name"><span>Name</span></th> 
						<th id="habitat_super"><span>Habitat Supertype</span></th> 
						<th id="habitat_type"><span>Habitat Type</span></th> 
						<th id="habitat_sub"><span>Habitat Subtype</span></th> 
						<th id="habitat_extra"><span>Additional info</span></th> 
					</tr> 
				</thead> 

				<tbody> 
ENDHTML
if($selecth->fetch())
{
print "<tr id=\"row1\"><td axis=\"number\" headers=\"metaid\"><a href=\'http://img.jgi.doe.gov/cgi-bin/m/main.cgi?section=TaxonDetail&page=taxonDetail&taxon_oid=$metaId\' target=\'_blank\'>$metaId</a></td><td axis=\"string\" headers=\"name\">$microbiomeName</td><td axis=\"string\" headers=\"habitat_super\">$habitat_super</td><td axis=\"string\" headers=\"habitat_type\">$habitat_type</td><td axis=\"string\" headers=\"habitat_sub\">$habitat_sub</td><td axis=\"string\" headers=\"habitat_extra\">$habitat_extra</td></tr>\n ";
}

while($selecth->fetch()) 
  {
    print "<tr><td axis=\"number\" headers=\"metaid\"><a href=\'http://img.jgi.doe.gov/cgi-bin/m/main.cgi?section=TaxonDetail&page=taxonDetail&taxon_oid=$metaId\' target=\'_blank\'>$metaId</a></td><td axis=\"string\" headers=\"name\">$microbiomeName</td><td axis=\"string\" headers=\"habitat_super\">$habitat_super</td><td axis=\"string\" headers=\"habitat_type\">$habitat_type</td><td axis=\"string\" headers=\"habitat_sub\">$habitat_sub</td><td axis=\"string\" headers=\"habitat_extra\">$habitat_extra</td></tr>\n ";
  }
					
					
print "</tbody>"; 
print "</table>"; 


  $selecth->finish;
  $dbh->disconnect;
