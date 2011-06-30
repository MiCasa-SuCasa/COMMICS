use compclusteringengine;

my $process_result = system("cluster -v 1>/dev/null");

if( -1 == $process_result)
{
	print "cluster is not correctly installed in your system\n";
}
else
{
#my $cluster_message = `cluster -g 7 -e 7 -f /home/master/Escritorio/cluster_bueno/cogs_data.txt`;
#if(  "" != $cluster_message )
#{
#	print "Message from cluster: $cluster_message\n";
#}
my $handler = compclusteringengine::CompClusteringEngine->new("/home/master/Escritorio/comp_clustering_engine");  
#print $handler->readFile("/home/master/Escritorio/cluster_bueno/cogs_data.cdt");
print $handler->readFile("/home/master/Escritorio/furbi/data_62_1294159688703.cdt");
print "\n";
print $handler->paintClusteringData;
print $handler->paintHorizontalTree;
print $handler->paintZoomImage( 5, 100 );
print $handler->paintCogs( 5, 100 );
print $handler->paintMetagenomes;
print $handler->getErrorString();
print "\n";
}
