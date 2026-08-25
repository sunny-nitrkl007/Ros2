#!/usr/bin/perl
# See README for more context.
#
# inputs:
#   -h shows usage
#   condition name
#   condition message
#   activation debounce duration in ms (defaults to zero - no debounce)
#   deactivation debounce duration in ms (defaults to zero - no debounce)
#   time to live duration in ms (defaults to 10000 - 10 seconds)
#
# checks:
#   run script from anywhere, but insist that CAT_DIR be defined
#     and that $CAT_DIR/prod/common/autonomyConditions/conditions exists
#   bail if  prod/common/autonomyCondition/condition/$conditionName exists
#
# if preconditions satisfied:
# create prod/common/interfaces/autonomyCondition/condition/$conditionName.h
# that contains the autonomy condition class with the specified settings

use Getopt::Long;
use strict;

##
## Get command line options
##

my $conditionText="/* insert condition message here*/";
my $activationDebounce=0;
my $deactivationDebounce=0;
my $timeToLive=86400000;  # one day

my $opts = GetOptions(
    'conditionName|n=s' => \ my $conditionName,
    'conditionText|m=s' => \ $conditionText,
    'activationDebounce|a=i' => \ $activationDebounce,
    'deactivationDebounce|d=i' => \ $deactivationDebounce,
    'timeToLive|t=i' => \ $timeToLive,
    'help|h' => \ my $help,
);


##
## Define Constants
##
my $env_cat_dir = $ENV{'CAT_DIR'};
my $conditionDir = $env_cat_dir . '/prod/common/autonomyConditions/conditions/';
my $conditionFile = $conditionDir . $conditionName . ".h";

##
## Check pre-conditions
##
if (defined $help) { help(); exit(0); }
if ( not defined $conditionName ) { help(); exit(1); }
die "Error: CAT_DIR environment variable not set" unless (-d $env_cat_dir);
die "Error: $conditionDir does not exist" unless ( -d $conditionDir );
die "Error: $conditionFile already exists" if ( -d $conditionFile );


##
## Make the interfaces
##
print( "Creating condition file: " . $conditionFile . "...\n" );

(my $second, my $minute, my $hour, my $dayOfMonth, my $monthOffset, my $yearOffset, my $dayOfWeek, my $dayOfYear, my $daylightSavings) = localtime();

my $month = $monthOffset + 1;
my $year = $yearOffset + 1900;

## Create the convenience InterfaceTypes.h file
open(CONDITIONFILE, ">>$conditionFile");


print CONDITIONFILE "///////////////////////////////////////////////////////////////////////////////\n";
print CONDITIONFILE "/// \@file      " . $conditionName . "\n";
print CONDITIONFILE "/// \@author    " . $ENV{'USER'} . "\n";
print CONDITIONFILE "/// \@date      " . $month . "/" . $dayOfMonth . "/" . $year . "\n";
print CONDITIONFILE "/// \@brief     Autonomy Condition for " . $conditionName . "\n";
print CONDITIONFILE "///                                                                            \n";
print CONDITIONFILE "/// \@attention Copyright (C) 2012                                              \n";
print CONDITIONFILE "/// \@attention National Robotics Engineering Center                            \n";
print CONDITIONFILE "/// \@attention Carnegie Mellon University                                      \n";
print CONDITIONFILE "/// \@attention All rights reserved                                             \n";
print CONDITIONFILE "///////////////////////////////////////////////////////////////////////////////\n";

print CONDITIONFILE "#ifndef " . $conditionName . "_h\n";
print CONDITIONFILE "#define " . $conditionName . "_h\n\n";

print CONDITIONFILE "#include <string>\n";
print CONDITIONFILE "#include \"autonomyConditions/AutonomyCondition.h\"\n\n";

print CONDITIONFILE "class " . $conditionName . " : public AutonomyCondition\n";
print CONDITIONFILE "{\n";
print CONDITIONFILE "public:\n\n";
print CONDITIONFILE "  static std::string getConditionTypeString() {return \"" . $conditionName . "\";}\n";
print CONDITIONFILE "  static const unsigned int activationDebounce_ms = " . $activationDebounce . ";\n";
print CONDITIONFILE "  static const unsigned int deactivationDebounce_ms = " . $deactivationDebounce . ";\n";
print CONDITIONFILE "  static const unsigned int timeToLive_ms = " . $timeToLive . ";\n\n";
print CONDITIONFILE "  " . $conditionName . "(/*insert any needed arguments for message string*/):\n";
print CONDITIONFILE "    AutonomyCondition(getConditionTypeString(),\n";
print CONDITIONFILE "                      \"" . $conditionText . "\",\n";
print CONDITIONFILE "                      activationDebounce_ms,\n";
print CONDITIONFILE "                      deactivationDebounce_ms,\n";
print CONDITIONFILE "                      timeToLive_ms)\n";
print CONDITIONFILE "  {}\n";
print CONDITIONFILE "};\n\n";

print CONDITIONFILE "#endif\n";
close CONDITIONFILE;


##
## Define sub-routines
##
sub help
{
    print ( "Usage: \n" .
            "  --conditionName|-n        (required) name of the autonomy condtion\n" .
            "                            the new autonomy condtion will be placed in a header file with this name\n" .
            "  --conditionText|-m        optional Text to display for this condition\n" .
            "  --activationDebounce|-a   optional activation debounce time in ms - defaults to zero\n" .
            "  --deactivationDebounce|-d optional deactivation debounce time in ms - defaults to zero\n" .
            "  --timeToLive|-t           optional how long a set condition should stay active in ms - defaults to 86400000 (one day)\n" .
            "  --help|-h                 print this help message\n" .
            "\n" .
            "Example: \n" .
            "  ./generateCondtion.pl -n SomeNewCondtion -m \"Info about this new condtion\" -a 1000 -d 5000 -t 86400000\n" .
            "\n" 
    );
}

