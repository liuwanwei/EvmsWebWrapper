#!/usr/bin/perl -w

# called by ../shell/ResourceMapping.sh

use strict;

# The intermidiate file for parsing, has the original messages printed by 'osmtest' tool
my $inter_file = "/tmp/search_inter_file";

# The result file for upper level C codes to use, has all ib ports name stored in it
my $result_file= "/tmp/all_linked_ports";

# Delete result file firstly
system("rm $result_file -rf");

($#ARGV == -1) && die "GetConnectedPorts.pl local_port1 local_port2";
my $port1_id=$ARGV[0];
my $port2_id=$ARGV[1];

# For testing
# my $port1_id="0x0002c90200210d05";
# my $port2_id="0x0002c90200210d06";

my $find_port=0;
my $dgid="";

open(RESULT_FILE_HANDLE, ">>$result_file") or die "cannot open $result_file\n";
open (INTERL_FILE_HANDLE, "<$inter_file") or die "cannot open $inter_file\n";

while(<INTERL_FILE_HANDLE>)
{
	# jump the blank line
	next if((/^\s*$/) || (/^\s*#/));

	if(/^dgid\s*(\w+)\s*(\w+)/)
	{
		#print "dgid: $2\n";

		if((($port1_id ne "") && ($2 eq $port1_id)) || (($port2_id ne "") && ($2 eq $port2_id)))
		{
			$find_port = 1;
			$dgid =  $2;
		}
	}
	elsif(/^sgid\s*(\w+)\s*(\w+)/)
	{
		#print "sgid: $2\n";

		if($find_port != 0)
		{
			if( $2 ne $dgid)
			{
				print "find one port.\n";
				print RESULT_FILE_HANDLE "$2\n";
			}
		}

		$find_port = 0;
		$dgid = "";
	}
}

close INTERL_FILE_HANDLE;
close RESULT_FILE_HANDLE;
