#!/usr/bin/perl 

use 5.010;
use strict;
use warnings;
use CGI;

use File::Spec;

require File::Temp;
use File::Temp ();
use File::Temp qw/ :seekable /;
require Encode;  

my $q = CGI->new;                       

##################
# File variables #
##################

my $bfh;
my $ffh;
my $batchfilename;
my $factfilename;
my $escapefilename;

###################################
# Create the batch and fact files #
# using temporary files.          #
###################################

$bfh = File::Temp->new();
$batchfilename = $bfh->filename;
  
$ffh = File::Temp->new();
$factfilename = $ffh->filename;

#######################################
# Create the batch and fact files and #
# place them in the temp directory.   #
#######################################

# $batchfilename = 'temp/' . 'temp' . $$ . '.bat';
# $factfilename = 'temp/' . 'temp' . $$ . '.fct';
# 
# open($bfh, '>', $batchfilename) or die;
# open($ffh, '>', $factfilename) or die;

##################################
# Create the batch file contents #
##################################

say $bfh '(load* autocgi.clp)';
say $bfh '(reset)';
say $bfh '(load-facts auto_en.fct)';

$escapefilename = $factfilename;
$escapefilename =~ s/\\/\\\\/g;
say $bfh "(load-facts \"$escapefilename\")";

say $bfh '(run)';
say $bfh '(exit)';

close $bfh;

################################# 
# Create the fact file contents #
#################################

my $prevAnswer;

my $variableString;
if ($q->param('variableString'))
  { $variableString = $q->param('variableString'); }
else
  { $variableString = ""; }
    
my @variableArray = split('%1B',$variableString);

my $answerString;
if ($q->param('answerString'))
  { $answerString = $q->param('answerString'); }
else
  { $answerString = ""; }
    
my @answerArray = split('%1B',$answerString);
  
if ($q->param('Next'))
  {  
   my $variable = $q->param('variable');
   my $answer = $q->param('answer');
   
   if ($variable and $answer)
     { 
      my $newVar = "($variable $answer)";
      
      push @variableArray , $newVar;
      
      push @answerArray, $answer;
            
      print $ffh join ("\n", @variableArray);
     }
  }
elsif ($q->param('Prev'))
  {
   pop @variableArray;
   
   $prevAnswer = pop @answerArray;
   
   print $ffh join ("\n", @variableArray);
  }
elsif ($q->param('Restart'))
  {
   @variableArray = ();
   @answerArray = ();
  }
  
$variableString = join ('%1B' , @variableArray);
$answerString = join ('%1B' , @answerArray);

# Close the file

close $ffh;

######################################
# Run CLIPS and retrieve the results #
######################################

# Set the current working directory to
# the one containing our perl script

my $volume;
my $directories;
my $file;

($volume,$directories,$file) = File::Spec->splitpath($0);
                       
chdir $directories;

# Execute CLIPS

my @output = `../clips -f2 $batchfilename`;

# Retrieve the results

my %info;

foreach (@output)
  {
   my ($attribute , $value) = split /=/ , $_ ;
   chomp $value;

   $info{$attribute} = Encode::decode_utf8($value);
  }

###################
# Create the HTML #
###################

print $q->header;
                    
print $q->start_html('auto'); 
      
#<noscript>Your browser does not support JavaScript!</noscript>
print $q->noscript('Your browser does not support JavaScript!');

############
# The Form #
############

print $q->start_form;

print $q->hidden(-name => 'variableString',
                 -value => $variableString,
                 -override => 1);

print $q->hidden(-name => 'answerString',
                 -value => $answerString,
                 -override => 1);

if ($info{variable})
  {
   print $q->hidden(-name => 'variable',
                    -value => $info{variable},
                    -override => 1);
  }

print $q->start_table({-border=>0,-cellpadding=>'10',-width=>'400'});

print $q->Tr({-align=>'CENTER',-valign=>'MIDDLE',-height => '50'},
             $q->td($q->h2($info{autoDemoLabel})));

print $q->Tr({-align=>'CENTER',-valign=>'MIDDLE',-height => '50'},
             $q->td($info{display}));

if ($info{displayAnswers})
  {
   my @returnAnswers = split(/:/, $info{validAnswers});
   my @displayLabels = split(/:/, $info{displayAnswers});
   my %labelHash;
   
   for my $i (0 .. $#returnAnswers)
     {  $labelHash{$returnAnswers[$i]} = ucfirst($displayLabels[$i]); }

   if (@returnAnswers > 1)
     {
      my $defaultValue;
      
      if ($prevAnswer)
        { $defaultValue = $prevAnswer; }
      else
        { $defaultValue = $returnAnswers[0]; }
        
      print $q->Tr({-align=>'CENTER',-valign=>'MIDDLE'},
                   $q->td({-height => '50' },
                          $q->radio_group({-name => 'answer' ,
                                           -values => \@returnAnswers ,
                                           -labels => \%labelHash ,
                                           -override => 1 ,
                                           -default => $defaultValue ,
                                           -linebreak => 'true'})));
     }
   elsif (@returnAnswers == 1)
     {
      print $q->Tr({-height => '50' },
                   $q->hidden(-name => 'answer',
                              -value => $returnAnswers[0],
                              -override => 1));
     }
   else
     { print $q->Tr($q->td({-height => '50' })); }
  }
else
  {
   print $q->Tr($q->td({-height => '50' }));
  }

if ($info{state} eq 'greeting')
  {
   print $q->Tr({-align => 'CENTER',-valign => 'MIDDLE',-height => '50'},
                $q->td($q->submit(-name => 'Next',
                                  -value => $info{nextLabel})));
  }
elsif ($info{state} eq 'interview')
  {
   print $q->Tr({-align=>'CENTER',-valign =>'MIDDLE',-height => '50'},
                $q->td($q->submit(-name => 'Prev',
                                  -value => $info{prevLabel}),
                       $q->submit(-name => 'Next',
                                  -value => $info{nextLabel})));
  }
elsif ($info{state} eq 'conclusion')
  {
   print $q->Tr({-align=>'CENTER',-valign => 'MIDDLE',-height => '50'},
                $q->td($q->submit(-name => 'Prev',
                                  -value => $info{prevLabel}),
                       $q->submit(-name => 'Restart',
                                  -value => $info{restartLabel})));
  }

print $q->end_table;

print $q->end_form;

#####################################
# Debugging: Print the CLIPS Output #
#####################################

# print $q->h2('CLIPS Output');
#
# if ($info{state})
#   { print $q->p('state = ' . $info{state}); }
# 
# if ($info{display})
#    { print $q->p('display = ' . $info{display}); }
# 
# if ($info{variable})
#   { print $q->p('variable = ' . $info{variable}); }
# 
# if ($info{validAnswers})
#   { print $q->p('validAnswers = ' . $info{validAnswers}); }
# 
# if ($info{displayAnswers})
#   { print $q->p('displayAnswers = ' . $info{displayAnswers}); }
#   
# print $q->p("constructed variable list =|" . $variableString . "|");
# 
# print $q->p("constructed answer list =|" . $answerString . "|");

#########################################
# Debugging: Print the parameter values #
#########################################

# my @names = $q->param;
# 
# print $q->h2('Params');
# 
# if ($q->param('Next'))
#   { print $q->p('Process Next'); }
# 
# if ($q->param('Prev'))
#   { print $q->p('Process Prev'); }
# 
# if ($q->param('Restart'))
#   { print $q->p('Process Restart'); }
#   
# foreach my $param (@names)
#   {
#    my $value = $q->param($param);
#   
#    print $q->p("$param = $value");
#   }
 
###############
# End of HTML #
###############

print $q->end_html;                 

