#!/usr/bin/perl 

use 5.010;
use strict;
use warnings;
use CGI;

use File::Spec;

require File::Temp;
use File::Temp ();
use File::Temp qw/ :seekable /;
  
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

say $bfh '(load* winecgi.clp)';
say $bfh '(reset)';

$escapefilename = $factfilename;
$escapefilename =~ s/\\/\\\\/g;
say $bfh "(load-facts \"$escapefilename\")";

say $bfh '(run)';
say $bfh '(exit)';

close $bfh;

################################# 
# Create the fact file contents #
#################################

my $slotvalue;

# Preferred Color

$slotvalue = $q->param('preferred-color');
if ($slotvalue)
  { say $ffh "(attribute (name preferred-color) (value $slotvalue))"; }
else
  { say $ffh "(attribute (name preferred-color) (value unknown))"; }

# Preferred Body

$slotvalue = $q->param('preferred-body');
if ($slotvalue)
  { say $ffh "(attribute (name preferred-body) (value $slotvalue))"; }
else
  { say $ffh "(attribute (name preferred-body) (value unknown))"; }

# Preferred Sweetness

$slotvalue = $q->param('preferred-sweetness');
if ($slotvalue)
  { say $ffh "(attribute (name preferred-sweetness) (value $slotvalue))"; }
else
  { say $ffh "(attribute (name preferred-sweetness) (value unknown))"; }

# Main course

$slotvalue = $q->param('main-course');

if ((not $slotvalue) or ($slotvalue eq 'unknown'))
  {
   say $ffh "(attribute (name main-component) (value unknown))";
   say $ffh "(attribute (name has-turkey) (value unknown))";
  }
elsif (($slotvalue eq 'beef') or
       ($slotvalue eq 'pork') or
       ($slotvalue eq 'lamb'))
  {
   say $ffh "(attribute (name main-component) (value meat))";
   say $ffh "(attribute (name has-turkey) (value no))";
  }
elsif ($slotvalue eq 'turkey')
  {
   say $ffh "(attribute (name main-component) (value poultry))";
   say $ffh "(attribute (name has-turkey) (value yes))";
  }
elsif (($slotvalue eq 'chicken') or
       ($slotvalue eq 'duck'))
  {
   say $ffh "(attribute (name main-component) (value poultry))";
   say $ffh "(attribute (name has-turkey) (value no))";
  }
elsif ($slotvalue eq 'fish')
  {
   say $ffh "(attribute (name main-component) (value fish))";
   say $ffh "(attribute (name has-turkey) (value no))";
  }
elsif ($slotvalue eq 'other')
  {
   say $ffh "(attribute (name main-component) (value unknown))";
   say $ffh "(attribute (name has-turkey) (value no))";
  }
  
# Sauce

$slotvalue = $q->param('sauce');

if ((not $slotvalue) or ($slotvalue eq 'unknown'))
  {
   say $ffh "(attribute (name has-sauce) (value unknown))";
   say $ffh "(attribute (name sauce) (value unknown))";
  }
elsif ($slotvalue eq 'none')
  { say $ffh "(attribute (name has-sauce) (value no))"; }
elsif ($slotvalue eq 'other')
  {
   say $ffh "(attribute (name has-sauce) (value yes))";
   say $ffh "(attribute (name sauce) (value unknown))";
  }
else
  {
   say $ffh "(attribute (name has-sauce) (value yes))";
   say $ffh "(attribute (name sauce) (value $slotvalue))";
  }

# Flavor

$slotvalue = $q->param('flavor');
if ($slotvalue)
  { say $ffh "(attribute (name tastiness) (value $slotvalue))"; }
else
  { say $ffh "(attribute (name tastiness) (value unknown))"; }

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

###################
# Create the HTML #
###################

print $q->header;
                    
print $q->start_html('wine'); 
      
#<noscript>Your browser does not support JavaScript!</noscript>
print $q->noscript('Your browser does not support JavaScript!');

print $q->h1('Wine Demo');

print $q->start_table({-border=>0});

print $q->start_Tr({-valign=>'TOP'});;

print $q->start_td;

print $q->start_form;

print $q->start_table({-border=>undef,-cellpadding=>'10'});

print $q->Tr({-align=>'CENTER',-valign=>'TOP'},
             $q->th(['Preferences', 'Meal']));

#print $q->start_Tr({-align=>'CENTER',-valign=>'TOP'});

# Preferences Cell: Color, Body, and Sweetness

print $q->start_Tr ({ -valign=>'TOP' });

print $q->start_td;
				
print $q->p(attribute_select_html(
              { 'text_label' => 'Color:',
                'popup_name' => 'preferred-color',
                'popup_labels' => { 'unknown' => 'Don\'t Care',
					                'red' => 'Red',
					                'white'=>'White' },
                'popup_default' => 'unknown',
				'popup_values' => ['unknown', 'red', 'white'] }));

print $q->p(attribute_select_html(
              { 'text_label' => 'Body:',
                'popup_name' => 'preferred-body',
                'popup_labels' => { 'unknown'=>'Don\'t Care',
					                'light'=>'Light',
					                'medium'=>'Medium',
					                'full'=>'Full' },
                'popup_default' => 'unknown',
				'popup_values' => ['unknown', 'light', 'medium', 'full'] }));


print $q->p(attribute_select_html(
              { 'text_label' => 'Sweetness:',
                'popup_name' => 'preferred-sweetness',
                'popup_labels' => { 'unknown'=>'Don\'t Care',
					                'dry'=>'Dry',
					                'medium'=>'Medium',
					                'sweet'=>'Sweet' },
                'popup_default' => 'unknown',
				'popup_values' => ['unknown', 'dry', 'medium', 'sweet'] }));

print $q->end_td;

# Meal Cell: Main Course, Sauce, and Flavor

print $q->start_td;

print $q->p(attribute_select_html(
              { 'text_label' => 'Main Course:',
                'popup_name' => 'main-course',
                'popup_labels' => { 'unknown'=>'Don\'t Know',
					                'beef'=>'Beef',
					                'pork'=>'Pork',
					                'lamb'=>'Lamb',
					                'turkey'=>'Turkey',
					                'chicken'=>'Chicken',
					                'duck'=>'Duck',
					                'fish'=>'Fish',
					                'other'=>'Other'},
                'popup_default' => 'unknown',
				'popup_values' => ['unknown', 'beef', 'pork', 'lamb', 'turkey', 
				                   'chicken', 'duck', 'fish', 'other'] }));

print $q->p(attribute_select_html(
              { 'text_label' => 'Sauce:',
                'popup_name' => 'sauce',
                'popup_labels' => { 'unknown'=>'Don\'t Know',
					                'none'=>'None',
					                'spicy'=>'Spicy',
					                'sweet'=>'Sweet',
					                'cream'=>'Cream',
					                'other'=>'Other' },
                'popup_default' => 'unknown',
				'popup_values' => ['unknown', 'none', 'spicy', 
				                   'sweet', 'cream', 'other'] }));

print $q->p(attribute_select_html(
              { 'text_label' => 'Flavor:',
                'popup_name' => 'flavor',
                'popup_labels' => { 'unknown'=>'Don\'t Know',
					                'delicate'=>'Delicate',
					                'average'=>'Average',
					                'strong'=>'Strong' },
                'popup_default' => 'unknown',
				'popup_values' => ['unknown', 'delicate', 'average', 'strong'] }));

print $q->end_td;

print $q->end_Tr;

print $q->end_table;

print $q->end_form;

print $q->end_td;

###################
# Suggested Wines #
###################

print $q->start_td;

print $q->start_table({-border=>undef,-cellpadding=>'10'});

print $q->Tr({-align=>'CENTER',-valign=>'TOP'},
             $q->th(['Wine', 'Recommendation Weight']));

foreach (@output) 
  { 
   my ($wine , $cv) = split /=/ , $_ ;
   chomp $cv;
   
   print $q->start_Tr();
   print $q->td($wine);  

   print $q->start_td;
   print $q->start_div({-style=>'background-color: lightgray;width: 200px;'});  
   print $q->start_div({-style=>"background-color: blue;width: $cv%; height: 20px;"});  
   print $q->end_div;
   print $q->end_div;
   print $q->end_td;
   
   print $q->start_Tr;  
  }

print $q->end_table;

print $q->end_td;

print $q->end_Tr;

print $q->end_table;

#########################################
# Debugging: Print the parameter values #
#########################################

# my @names = $q->param;
# 
# print $q->h2('Params');
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

###############
# Subroutines #
###############

sub attribute_select_html 
  {
   my ($args) = @_;
   
   return $q->label($args->{'text_label'}) .
          $q->br .
          $q->popup_menu(-name => $args->{'popup_name'},
                         -values => $args->{'popup_values'},
                         -default => $args->{'popup_default'},
                         -labels => $args->{'popup_labels'},
                         -onChange => 'this.form.submit()'
                         );
  }


