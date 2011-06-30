#!/usr/bin/perl -w

package DataParser;

use DBI;

use constant WORK_DIR => "/usr/lib/cgi-bin/commics/work-dir";
use constant DB_HOST => "localhost";
use constant DB_NAME => "METAGENOMESIMG";
use constant DB_USER => "commics";
use constant DB_PASS => "";

use strict;


sub new 
{
  my ($class_name) = @_;
 
  my $self = {
      _dsn => "",
      _dbh  => undef,
  };

  bless $self, $class_name;
  return $self;
}

##########################
#
# Subroutine to initialize the database connection
#
##########################
sub init
{
  my( $self ) = @_;
  $self->{_dsn} = "DBI:mysql:".DB_NAME.":".DB_HOST;
  $self->{_dbh} = DBI->connect($self->{_dsn}, DB_USER, DB_PASS)
                or die "Couldn't connect to database: " . DBI->errstr;
}

##########################
#
# Subroutine to parse user metagenomes. 
# It stores the abundance of the functional categories which the annotated COGs belong to in %userFunctionalSets
# Funtional categories are used to conduct a CDA rather than individual COGs
#
# It also stores in %userGenesCount the total amount of genes annotated in that metagenome
#
##########################
sub parseCdaUserMetagenomes
{
  my( $self, $userMetagenomes, $userMetaNames, $userGenesCount, $userCountCogs, $userFunctionalSets ) = @_;

  my $funct_cath = $self->{_dbh}->prepare("SELECT COG_CATEGORY FROM cog_categories WHERE COG= ?")
                or die "Couldn't prepare statement: " . $self->{_dbh}->errstr;

  my (@separatedData) = split( '\n', $userMetagenomes); 

  my $name = "";
  my $countMetagenomes = -1;
  for( my $i = 0; $i < scalar(@separatedData); $i++ )
  {
    if(  substr($separatedData[$i], 0, 1) eq '>'  )
    {
      $name = substr( $separatedData[$i],1, length( $separatedData[$i] ) - 1 );
      push(@$userMetaNames,$name);
      $countMetagenomes++;
    }
    elsif( $separatedData[$i] =~ m/^COG\d\d\d\d/ )
    {
      my($cog,$count) = split( ' ', $separatedData[$i] );
      $$userCountCogs{$countMetagenomes}{$cog} = $count;
      $funct_cath->execute($cog);
      while( my ($cat) = $funct_cath->fetchrow_array() )
      {
        if( defined( $$userFunctionalSets{$countMetagenomes}{$cat} ) )
        {
          $$userFunctionalSets{$countMetagenomes}{$cat} += $count;
        }
        else
        {
          $$userFunctionalSets{$countMetagenomes}{$cat} = $count;
        }
      }
      $funct_cath->finish;
    } 
    elsif( $separatedData[$i] =~ m/^TOTAL_GENES/ )
    {
      my(undef, $count_genes) = split(' ',$separatedData[$i]);
      $$userGenesCount{$countMetagenomes} = $count_genes;
    }
  }#for
}

##########################
#
# Subroutine to parse user metagenomes. 
# It stores the abundance of the annotated COGs in %userGenesCount
#
# It also stores in %userGenesCount the total amount of genes annotated in that metagenome
#
##########################
sub parseUserMetagenomes
{
  my( $self, $userMetagenomes, $userMetaNames, $userGenesCount, $userCountCogs ) = @_;
  my (@separatedData) = split( '\n', $userMetagenomes); 

  my $name = "";
  my $countMetagenomes = -1;
  for( my $i = 0; $i < scalar(@separatedData); $i++ )
  {
    if(  substr($separatedData[$i], 0, 1) eq '>'  )
    {
      $name = substr( $separatedData[$i],1, length( $separatedData[$i] ) - 1 );
      push(@$userMetaNames,$name);
      $countMetagenomes++;
    }
    elsif( $separatedData[$i] =~ m/^COG\d\d\d\d/ )
    {
      my($cog,$count) = split( ' ', $separatedData[$i] );
      $$userCountCogs{$countMetagenomes}{$cog} = $count;

    } 
    elsif( $separatedData[$i] =~ m/^TOTAL_GENES/ )
    {
      my(undef, $count_genes) = split(' ',$separatedData[$i]);
      $$userGenesCount{$countMetagenomes} = $count_genes;
    }
  }

}

##########################
#
# Subroutine to generate a text file which contains a logs-odd ratio matrix wich depicts 
# the over or under representation of the selected COGs in all the selected metagenomes.
# This file is used later on to perform the clustering.
#
# $params -> Metagenomes Ids
# $fileName -> Unique identifier
# $categoriesParams -> Functional categories
# $userCogs -> Individual COGs the user introduced
# $userMetagenomes -> String containing the annotation of the user metagenomes
#
##########################
sub generateClusterData
{
  my( $self, $params, $fileName, $categoriesParams, $userCogs, $userMetagenomes ) = @_;

  my @values = split( '-', $params );
  @values = sort {$a <=> $b} @values;
  
  my @categories = split( '-',$categoriesParams );
  @categories = sort @categories;
  
  my @userMetaNames;
  my %userGenesCount;
  my %userCountCogs;
  
  my @userMetagenomes = $self->parseUserMetagenomes( $userMetagenomes, \@userMetaNames, \%userGenesCount, \%userCountCogs );
  
  my $placeHolders = " "; 
  for( my $j = 0; $j < @categories;$j++){ $placeHolders .= "?,"; } 
  $placeHolders = substr($placeHolders,0,-1); 
  
  if( scalar(@categories) > 0 )
  {
    my $tmph = $self->{_dbh}->prepare("CREATE TABLE tmp_cogs_$fileName AS SELECT distinct(COG) FROM cog_categories WHERE COG_CATEGORY IN($placeHolders) AND COG IS NOT NULL AND COG!=\'\'")
                  or die "Couldn't prepare statement: " . $self->{_dbh}->errstr;
  
    $tmph->execute(@categories);
    $tmph->finish;
  }
  else
  {
    my $tmph = $self->{_dbh}->prepare("CREATE TABLE tmp_cogs_$fileName AS SELECT distinct(COG) FROM cog_categories WHERE 1=2")
                  or die "Couldn't prepare statement: " . $self->{_dbh}->errstr;
  
    $tmph->execute();
    $tmph->finish;
  }
  my $insertUserh = $self->{_dbh}->prepare("INSERT INTO tmp_cogs_$fileName VALUES(?)")
                  or die "Couldn't prepare statement: " . $self->{_dbh}->errstr;
  
  my @userCogsArray = split( "\n", $userCogs );
  foreach my $userCog(@userCogsArray)
  {
    if( $userCog =~ m/^COG\d\d\d\d$/ )
    {
      $insertUserh->execute( $userCog );
    }
  }
  $insertUserh->finish;
  
  my $droptmph = $self->{_dbh}->prepare("DROP TABLE tmp_cogs_$fileName");

  my $counth = $self->{_dbh}->prepare("SELECT count_cogs.COG, count_cogs.COUNT_COG FROM count_cogs INNER JOIN tmp_cogs_$fileName tmp ON count_cogs.COG=tmp.COG AND count_cogs.METAID=?")
                  or die "Couldn't prepare statement: " . $self->{_dbh}->errstr;
  
  
  my $cogsh = $self->{_dbh}->prepare("SELECT distinct(COG) FROM tmp_cogs_$fileName")
                  or die "Couldn't prepare statement: " . $self->{_dbh}->errstr;
  
  my $totalgenesh = $self->{_dbh}->prepare('select count(distinct(geneid)) from genes where metaid= ? group by metaid')
                  or die "Couldn't prepare statement: " . $self->{_dbh}->errstr;
  
  my $cogmetagenomes = $self->{_dbh}->prepare('select count(distinct(metaid)) from genes where cog= ?')
                  or die "Couldn't prepare statement: " . $self->{_dbh}->errstr; 
  
  my $namemetagenomeh = $self->{_dbh}->prepare('select MICROBIOME_NAME from metadata where META_ID= ?')
                  or die "Couldn't prepare statement: " . $self->{_dbh}->errstr; 
  
  my $categoryh = $self->{_dbh}->prepare('select COG_CATEGORY from cog_categories where COG = ?')
                  or die "Couldn't prepare statement: " . $self->{_dbh}->errstr; 
  
  open (OUTPUTFILE, ">".WORK_DIR."/data_$fileName.txt");
  
  
  my (@array_cogs, @array_metaids);
  my (%hash_cogs, %hash_metaids);
  
  $cogsh->execute();
  my $cogs_rows = $cogsh->rows();
  
  for (my $i = 0; $i < $cogs_rows; $i++) 
  {
    my ($cog) = $cogsh->fetchrow_array();
    $categoryh->execute($cog);
    my $cog_category = $cog.'[';
    while( my $category = $categoryh->fetchrow_array() )
    {
      $cog_category = $cog_category.$category.',';
    }
    $categoryh->finish;
    $cog_category = substr($cog_category,0,-1); 
    $cog_category = $cog_category.']';
    push(@array_cogs, $cog_category);
    $hash_cogs{$cog} = $i;
  }
  $cogsh->finish;
 
  print OUTPUTFILE "\t";
  
  my $i = 0;
  my $metaids_rows = scalar(@values) + scalar(@userMetaNames);
  #user metagenomes
  for( my $k=0; $k < scalar(@userMetaNames); $k++)
  {
    $hash_metaids{$k} = $i;
    push(@array_metaids,$k);
    if( 0 == scalar( @values) && $k == scalar(@userMetaNames) -1 )
    {
      print OUTPUTFILE "$userMetaNames[$k]\($k\)";
    }
    else
    {
      print OUTPUTFILE "$userMetaNames[$k]\($k\)\t";
    }
  
    $i++;
  }

  #db metagenomes
  for( my $j = 0; $j < scalar(@values); $j++ )
  {
      my $metaid = $values[$j];
  
      $hash_metaids{$metaid} = $i;
      push(@array_metaids,$metaid);
      $namemetagenomeh->execute($metaid);
      my $metagenome_name = $namemetagenomeh->fetchrow_array();		
      $namemetagenomeh->finish;
      if( $j < scalar(@values) - 1 )
      {
        print OUTPUTFILE "$metagenome_name\($metaid\)\t";
      }
      else
      {
        print OUTPUTFILE "$metagenome_name\($metaid\)";
      }
      $i++;
  }
  
  print OUTPUTFILE "\n";
  
  my @data_matrix = ([],[]);
  
  foreach my $metaid( keys %hash_metaids )
  {
    $counth->execute( $metaid );
    my $metaid_rows = $counth->rows();
  
    $totalgenesh->execute( $metaid );
    my $totalgenes = $totalgenesh->fetchrow_array();		
    $totalgenesh->finish;
  
    for (my $i = 0; $i < $metaid_rows; $i++) 
    {
      my($cog, $count) = $counth->fetchrow_array();
      $data_matrix[ $hash_cogs{$cog} ][ $hash_metaids{$metaid} ] = $count/$totalgenes;
    }
    $counth->finish;
  }

  foreach my $userMetaId( keys %userCountCogs )
  {
    foreach my $userCog( keys %{ $userCountCogs{$userMetaId} })
    {
      if(defined( $hash_cogs{$userCog} ))
      {
        $data_matrix[ $hash_cogs{$userCog} ][ $hash_metaids{$userMetaId} ] = $userCountCogs{$userMetaId}{$userCog}/$userGenesCount{$userMetaId};
      }
    }
  }     
    
  #Begin of the calculation of the logs-odd ratio matrix
  my(@rows,@columns);
  
  for (my $i = 0; $i <$cogs_rows ; $i++) 
  { 
    my $count_cog = 0;
    for (my $j = 0; $j < $metaids_rows; $j++) 
    {
      if( defined($data_matrix[ $i ][ $j ]))
      {
        $count_cog += $data_matrix[ $i ][ $j ];
      }
    }
    push(@columns, $count_cog);
  }
  
  for (my $i = 0; $i <$metaids_rows ; $i++) 
  { 
    my $count_metaid = 0;
    for (my $j = 0; $j < $cogs_rows; $j++) 
    {
      if( defined($data_matrix[ $j ][ $i ]))
      {
        $count_metaid += $data_matrix[ $j ][ $i ];
      }
    }
    push(@rows, $count_metaid);
  }

  my $T = 0;
  for (my $i = 0; $i < scalar(@rows); $i++)
  {
    $T += $rows[$i];
  }
  
  for (my $i = 0; $i <$cogs_rows ; $i++) 
  { 
    for (my $j = 0; $j < $metaids_rows; $j++) 
    {
      my $expected = 0;
      if( defined($data_matrix[ $i ][ $j ]))
      {
        $expected = ($rows[$j]*$columns[$i])/$T;
        my $log_odd = log( ($data_matrix[ $i ][ $j ]) / $expected );
        $data_matrix[ $i ][ $j ] = $log_odd;
      }
      else
      {
        $expected = ($rows[$j]*$columns[$i])/$T;
        my $log_odd = log( 1 / ( 1 + $expected ) );
        $data_matrix[ $i ][ $j ] = $log_odd;
      }
    }
  }
  
  # Writting to file
  for (my $i = 0; $i <$cogs_rows ; $i++) 
  { 
    print OUTPUTFILE "$array_cogs[$i]\t";
    for (my $j = 0; $j < $metaids_rows; $j++) 
    {
      if( defined($data_matrix[ $i ][ $j ]))
      {
        if( $j < $metaids_rows - 1 )
        {
          print OUTPUTFILE "$data_matrix[ $i ][ $j ]\t";
        }
        else
        {
          print OUTPUTFILE "$data_matrix[ $i ][ $j ]";
        }
      }#if
    }#for my $j
    print OUTPUTFILE "\n";
  }#for my $i

  close (OUTPUTFILE); 
  
  $droptmph->execute;
  $droptmph->finish;
}

###########################
#
# Subroutine to generate a text file (cda_<id>.txt) which contains the necessary data to perform a CDA in R
#
# $params -> Metagenome ID
# $fileName -> Unique file name that contains the data
# $categoriesParams -> Functional categories
# $userCogs -> List of COGs that the user added
# $userMetagenomes -> String containing the abundance of COGs of user metagenomes
#
###########################
sub generateCdaData
{
  my( $self, $params, $fileName, $categoriesParams, $userCogs, $userMetagenomes ) = @_;
  my @values = split( '-', $params );

  my @categories = split( '-',$categoriesParams );
  @values = sort {$a <=> $b} @values; # sort numerically ascending
  @categories = sort @categories; #sort alphabetically

  my @userMetaNames;
  my %userGenesCount;
  my %userCountCogs;
  my %userFunctionalSets;
  my @userMetagenomes = $self->parseCdaUserMetagenomes( $userMetagenomes, \@userMetaNames, \%userGenesCount, \%userCountCogs, \%userFunctionalSets );

  my $placeHolders = " "; 
  for( my $j = 0; $j < @categories;$j++){ $placeHolders .= "?,"; } 
  $placeHolders = substr($placeHolders,0,-1);  #Remove the last comma

  my $counth;
  if( scalar(@categories) > 0 )
  {
    $counth = $self->{_dbh}->prepare("SELECT COG_CATEGORY, SUM(COUNT_COG) FROM count_cogs WHERE COG_CATEGORY IN($placeHolders) AND METAID=? GROUP BY(COG_CATEGORY)")
                or die "Couldn't prepare statement: " . $self->{_dbh}->errstr;
  }

  my $countSingleCogsh = $self->{_dbh}->prepare("SELECT COG, COUNT_COG FROM count_cogs WHERE COG= ? AND METAID=?")
                or die "Couldn't prepare statement: " . $self->{_dbh}->errstr;


  my $totalgenesh = $self->{_dbh}->prepare('select count(distinct(geneid)) from genes where metaid= ? group by metaid')
                or die "Couldn't prepare statement: " . $self->{_dbh}->errstr;

  my $namemetagenomeh = $self->{_dbh}->prepare('select MICROBIOME_NAME from metadata where META_ID= ?')
                or die "Couldn't prepare statement: " . $self->{_dbh}->errstr; 

  my $categoryh = $self->{_dbh}->prepare('select COG_CATEGORY from cog_categories where COG = ?')
                or die "Couldn't prepare statement: " . $self->{_dbh}->errstr; 

  open (OUTPUTFILE, ">".WORK_DIR."/cda_$fileName.txt");
  open (INFOFILE, ">".WORK_DIR."/info_cda_$fileName.txt");

  my @userCogsArray = split( "\n", $userCogs );
  my (@array_cogs, @array_metaids);
  my (%hash_cogs, %hash_metaids);

  for (my $i = 0; $i < scalar(@userCogsArray); $i++) 
  {
     my ($cog) = $userCogsArray[$i];
     $categoryh->execute($cog);
     my $cog_category = $cog.'.';
     while( my $category = $categoryh->fetchrow_array() )
     {
       $cog_category = $cog_category.$category.',';
     }
     $categoryh->finish;
     $cog_category = substr($cog_category,0,-1); 
    
     push(@array_cogs, $cog_category);
     $hash_cogs{$cog} = $i;
  }

  #funcional categories, array_cogs contains COGs ids and functional categories
  for (my $i = 0; $i < scalar(@categories); $i++) 
  {
    my($category) = $categories[$i];
    push(@array_cogs, $category);
    $hash_cogs{$category} = $i + scalar(@userCogsArray);# don't forget the individual cogs
  }

 for( my $i = 0; $i < scalar(@array_cogs);$i++)
 {
   if( $i == scalar(@array_cogs) -1 )
   {
     print OUTPUTFILE "$array_cogs[$i]";
   }
   else
   {
     print OUTPUTFILE "$array_cogs[$i]\t";
   }
  }#for

  print OUTPUTFILE "\n";

  my $i = 0;
  my $metaids_rows = scalar(@values) + scalar(@userMetaNames);
  #user metagenomes
  for( my $k=0; $k < scalar(@userMetaNames); $k++)
  {
    $hash_metaids{$k} = $i;
    push(@array_metaids,$k);
    print INFOFILE "$k\t$userMetaNames[$k]\n";
    $i++;
  }

  #db metagenomes
  for( my $j = 0; $j < scalar(@values); $j++ )
  {
    my $metaid = $values[$j];

    $hash_metaids{$metaid} = $i;
    push(@array_metaids,$metaid);
    $namemetagenomeh->execute($metaid);
    my $metagenome_name = $namemetagenomeh->fetchrow_array();		
    $namemetagenomeh->finish;
  
    print INFOFILE "$metaid\t$metagenome_name\n";
    $i++;
  } 

  my @data_matrix = ([],[]);
  #initialization of the data matrix, all the values are set to 0
  for (my $j = 0; $j < scalar(@values) + scalar(@userMetaNames); $j++)  
  { 
    for (my $i = 0; $i < scalar( @array_cogs ) ; $i++)
    {
      $data_matrix[ $i ][ $j ] = 0;
    }
  }

  foreach my $metaid( keys %hash_metaids )
  {
    $totalgenesh->execute( $metaid );
    my $totalgenes = $totalgenesh->fetchrow_array();		
    $totalgenesh->finish;
    if( scalar(@categories) > 0)
    {
      $counth->execute( @categories, $metaid );
      my $metaid_rows = $counth->rows();
      for (my $i = 0; $i < $metaid_rows; $i++) 
      {
        my($cog, $count) = $counth->fetchrow_array(); 
        if(defined($cog) && defined($count) ) #in case we are using a user metagenome the sql query will return null values
        { 
          $data_matrix[ $hash_cogs{$cog} ][ $hash_metaids{$metaid} ] = $count/$totalgenes; 
        }
      }
      $counth->finish;
    }#if

    foreach my $userCog( @userCogsArray )
    {
      $countSingleCogsh->execute( $userCog, $metaid ); 
      my($cog, $count) = $countSingleCogsh->fetchrow_array(); 
      if(defined($cog) && defined($count) )# if they are both defined,it means that they are in the db, otherwise,the next foreach will fill datamatrix
      {
        $data_matrix[ $hash_cogs{$userCog} ][ $hash_metaids{$metaid} ] = $count/$totalgenes;
      }
      else
      {
        $data_matrix[ $hash_cogs{$userCog} ][ $hash_metaids{$metaid} ] = 0;
      }
      $countSingleCogsh->finish;
    }
  }# foreach my $metaId

  foreach my $userMetaId( keys %userCountCogs )
  {
    foreach my $userCog( keys %{ $userCountCogs{$userMetaId} })
    {
      if( defined( $hash_cogs{$userCog} ) )
      {
        $data_matrix[ $hash_cogs{$userCog} ][ $hash_metaids{$userMetaId} ] = $userCountCogs{$userMetaId}{$userCog}/$userGenesCount{$userMetaId};
      }
    }
    foreach my $category(@categories)
    {
      if( defined($userFunctionalSets{$userMetaId}{$category}) )
      {
        $data_matrix[ $hash_cogs{$category} ][ $hash_metaids{$userMetaId} ] = $userFunctionalSets{$userMetaId}{$category}/$userGenesCount{$userMetaId};
      }
      else
      {
        $data_matrix[ $hash_cogs{$category} ][ $hash_metaids{$userMetaId} ] = 0;
      }
    }
  }#foreach my $userMetaId

  for (my $j = 0; $j < $metaids_rows; $j++)  
  { 
    print OUTPUTFILE "$array_metaids[$j]\t";
    for (my $i = 0; $i < scalar( @array_cogs ) ; $i++)
    {
      if( defined($data_matrix[ $i ][ $j ]))
      {
        my $percent = $data_matrix[ $i ][ $j ]*100;
        if( $i < scalar( @array_cogs ) - 1 )
        {
          print OUTPUTFILE "$percent\t";
        }
        else
        {
          print OUTPUTFILE "$percent";
        }
      }               
    }# for my $i
    print OUTPUTFILE "\n";
  }# for my $j

  close (OUTPUTFILE); 
  close(INFOFILE);
}

###################
#
#Destructor
#
###################
sub DESTROY 
{ 
  my($self) = @_;
  $self->{_dbh}->disconnect;
}

1;
