#!/usr/bin/perl -w
use CGI;
use DBI;

use DataParser;

my $query = new CGI;


my $params = $query->param('data');
my $fileName =$query->param('sessionId');
my $categoriesParams = $query->param('categories');
my $userCogs = $query->param('customCogs');
my $userMetagenomes = $query->param('customData');

my $p = new DataParser;
$p->init();
$p->generateClusterData( $params, $fileName, $categoriesParams,$userCogs,$userMetagenomes );

$p->generateCdaData( $params, $fileName, $categoriesParams, $userCogs, $userMetagenomes );

print $query->header;



