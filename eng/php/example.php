<?php

dl( "phpmorpheng.dll" );

function CheckWord( $word, $ignore_capitals = 0 )
{
  if ( $ignore_capitals )
    return mlmaenCheckWord( $word, sfIgnoreCapitals );
  else
    return mlmaenCheckWord( $word, 0 );
}

function ReportCheckWord( $word )
{
  printf( "Checking the word '%s'...", $word );

  if ( CheckWord( $word ) )
    printf( "\tOK\n" );
  else
    printf( "\tfalse\n" );
}

ReportCheckWord( "word" );
ReportCheckWord( "wodr" );

$lemmas = mlmaenLemmatize( "windows", 0 );
  print_r( $lemmas );

$szform = mlmaenBuildForm( 10670, 1, "window" );
  for ( $nindex = 0; $nindex < count( $szform ); ++$nindex )
    printf( "    %s\n", $szform[$nindex] );

/*
$szform = mlmaenBuildForm( "�����", 0 );
  for ( $nindex = 0; $nindex < count( $szform ); ++$nindex )
    printf( "    %s\n", $szform[$nindex] );
*/
/*
$szform = mlmaenBuildForm( "Сегодня", 255 );
  for ( $nindex = 0; $nindex < count( $szform ); ++$nindex )
    printf( "    %s\n", $szform[$nindex] );
*/
/*
$szform = mlmaenBuildForm( "Сделать", 2 );
  for ( $nindex = 0; $nindex < count( $szform ); ++$nindex )
    printf( "    %s\n", $szform[$nindex] );
*/

?>
