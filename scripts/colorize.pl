#! /usr/bin/perl -w

#
# ripped from colorgcc Version: 1.3.2
# http://schlueters.de/colorgcc.1.3.2.txt
#

use Term::ANSIColor;

sub initDefaults
{
   $nocolor{"dumb"} = "true";

   $colors{"srcColor"} = color("cyan");
   $colors{"introColor"} = color("blue");

   $colors{"warningFileNameColor"} = color("yellow");
   $colors{"warningNumberColor"}   = color("yellow");
   $colors{"warningMessageColor"}  = color("yellow");

   $colors{"errorFileNameColor"} = color("bold red");
   $colors{"errorNumberColor"}   = color("bold red");
   $colors{"errorMessageColor"}  = color("bold red");
}

sub loadPreferences
{
# Usage: loadPreferences("filename");

   my($filename) = @_;

   open(PREFS, "<$filename") || return;

   while(<PREFS>)
   {
      next if (m/^\#.*/);          # It's a comment.
      next if (!m/(.*):\s*(.*)/);  # It's not of the form "foo: bar".

      $option = $1;
      $value = $2;

      if ($option eq "nocolor")
      {
	 # The nocolor option lists terminal types, separated by
	 # spaces, not to do color on.
	 foreach $termtype (split(/\s+/, $value))
	 {
	    $nocolor{$termtype} = "true";
	 }
      }
      else
      {
	 $colors{$option} = color($value);
      }
   }
   close(PREFS);
}

sub srcscan
{
# Usage: srcscan($text, $normalColor)
#    $text -- the text to colorize
#    $normalColor -- The escape sequence to use for non-source text.

# Looks for text between ` and ', and colors it srcColor.

   my($line, $normalColor) = @_;

   my($srcon) = color("reset") . $colors{"srcColor"};
   my($srcoff) = color("reset") . $normalColor;

   $line = $normalColor . $line;

   # This substitute replaces `foo' with `AfooB' where A is the escape
   # sequence that turns on the the desired source color, and B is the
   # escape sequence that returns to $normalColor.
   $line =~ s/\`(.*?)\'/\`$srcon$1$srcoff\'/g;

   print($line, color("reset"));
}

#
# Main program
#

# Set up default values for colors and compilers.
initDefaults();

# Read the configuration file, if there is one.
$configFile = $ENV{"HOME"} . "/.colorgccrc";
if (-f $configFile)
{
   loadPreferences($configFile);
}

# Colorize the input
while(<>)
{
   if (m/^(.*?):([0-9]+):(.*)$/) # filename:lineno:message
   {
      $field1 = $1 || "";
      $field2 = $2 || "";
      $field3 = $3 || "";

      if ($field3 =~ m/\s+warning:.*/)
      {
	 # Warning
	 print($colors{"warningFileNameColor"}, "$field1:", color("reset"));
	 print($colors{"warningNumberColor"}, "$field2:", color("reset"));
	 srcscan($field3, $colors{"warningMessageColor"});
      }
      else 
      {
	 # Error
	 print($colors{"errorFileNameColor"}, "$field1:", color("reset"));
	 print($colors{"errorNumberColor"}, "$field2:", color("reset"));
	 srcscan($field3, $colors{"errorMessageColor"});
      }
      print("\n");
   }
   elsif (m/^(.*?):(.+):$/) # filename:message:
   {
      # No line number, treat as an "introductory" line of text.
      srcscan($_, $colors{"introColor"});
   } elsif (m/No such file or directory/) {
     print($colors{"errorFileNameColor"}, $_);
   } else # Anything else.
   {
      # Doesn't seem to be a warning or an error. Print normally.
      print(color("reset"), $_);
   }
}
