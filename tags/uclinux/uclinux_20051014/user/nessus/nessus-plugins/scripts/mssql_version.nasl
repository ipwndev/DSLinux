#
# modified by Michael Scheidell SECNAP Network security
# to poll the smb registry (udp ping returned wrong info)
#
# Supercedes MS02-034 MS02-020 MS02-007 MS01-060 MS01-032 MS00-092 MS00-048
#            MS00-041 MS00-014 MS01-041
#
#
# CAN-2002-0056, CAN-2002-0154, CAN-2002-0624, 
# CAN-2002-0641, CAN-2002-0642  CVE-2001-0879
# CVE-2000-0603  CAN-2000-1082  CAN-2000-1083
# CAN-2000-1084  CAN-2000-1085  CAN-2001-0509
# CAN-2000-1086
	       
if(description)
{
 script_id(11217);
 script_version ("$Revision$");
 name["english"] = "Microsoft's SQL Version Query";
 script_name(english:name["english"]);
 
 script_cve_id("CAN-2000-1081", "CVE-2000-0202", "CVE-2000-0485",
 	       "CAN-2000-1087", "CAN-2000-1088", "CAN-2002-0982",
 	       "CAN-2001-0542", "CVE-2001-0344" );
	       
 script_bugtraq_id(4135, 4847, 5014, 5205);
  
 desc["english"] = "
The plugin attempts a smb connection to read version from
the registry key 
SOFTWARE\Microsoft\MSSQLServer\MSSQLServer\CurrentVersion
to determine the Version of SQL and Service Pack the host
is running.

Some versions may allow remote access, denial of service
attacks, and the ability of a hacker to run code of their
choice.

Risk factor : Low to High
Solution : Apply current service packs and hotfixes";


 script_description(english:desc["english"]);

 summary["english"] = "Microsoft's SQL Version Query";
 script_summary(english:summary["english"]);

 script_category(ACT_GATHER_INFO);
 script_copyright(english:"This script is Copyright (C) 2003 John Lampe");
 family["english"] = "Windows";
 script_family(english:family["english"]);
 script_dependencies("netbios_name_get.nasl",
                     "smb_login.nasl", "smb_registry_full_access.nasl",
		     "mssqlserver_detect.nasl");
 script_require_keys("SMB/transport", "SMB/name", "SMB/login",
                     "SMB/password", "SMB/registry_full_access");
 script_require_ports(139, 445, 1433, "Services/mssql");

 exit(0);
}


mssql_port = get_kb_item("Services/mssql");
if(!mssql_port)port = 1433;

# versions culled from http://www.sqlsecurity.com

version[0] = "8.00.760";  desc[0] = "2000 SP3   ";
version[1] = "8.00.679";  desc[1] = "2000 SP2+Q316333   ";
version[2] = "8.00.667";  desc[2] = "2000 SP2+8/14 fix  ";
version[3] = "8.00.665";  desc[3] = "2000 SP2+8/8 fix  ";
version[4] = "8.00.655";  desc[4] = "2000 SP2+7/24 fix  ";
version[5] = "8.00.650";  desc[5] = "2000 SP2+Q322853   ";
version[6] = "8.00.608";  desc[6] = "2000 SP2+Q319507   ";
version[7] = "8.00.604";  desc[7] = "2000 SP2+3/29 fix  ";
version[8] = "8.00.578";  desc[8] = "2000 SP2+Q317979   ";
version[9] = "8.00.561";  desc[9] = "2000 SP2+1/29 fix  ";
version[10] = "8.00.534";  desc[10] = "2000 SP2.01   ";
version[11] = "8.00.532";  desc[11] = "2000 SP2   ";
version[12] = "8.00.475";  desc[12] = "2000 SP1+1/29 fix  ";
version[13] = "8.00.452";  desc[13] = "2000 SP1+Q308547   ";
version[14] = "8.00.444";  desc[14] = "2000 SP1+Q307540/307655   ";
version[15] = "8.00.443";  desc[15] = "2000 SP1+Q307538   ";
version[16] = "8.00.428";  desc[16] = "2000 SP1+Q304850   ";
version[17] = "8.00.384";  desc[17] = "2000 SP1   ";
version[18] = "8.00.287";  desc[18] = "2000 No SP+Q297209  ";
version[19] = "8.00.250";  desc[19] = "2000 No SP+Q291683  ";
version[20] = "8.00.249";  desc[20] = "2000 No SP+Q288122  ";
version[21] = "8.00.239";  desc[21] = "2000 No SP+Q285290  ";
version[22] = "8.00.233";  desc[22] = "2000 No SP+Q282416  ";
version[23] = "8.00.231";  desc[23] = "2000 No SP+Q282279  ";
version[24] = "8.00.226";  desc[24] = "2000 No SP+Q278239  ";
version[25] = "8.00.225";  desc[25] = "2000 No SP+Q281663  ";
version[26] = "8.00.223";  desc[26] = "2000 No SP+Q280380  ";
version[27] = "8.00.222";  desc[27] = "2000 No SP+Q281769  ";
version[28] = "8.00.218";  desc[28] = "2000 No SP+Q279183  ";
version[29] = "8.00.217";  desc[29] = "2000 No SP+Q279293/279296  ";
version[30] = "8.00.211";  desc[30] = "2000 No SP+Q276329  ";
version[31] = "8.00.210";  desc[31] = "2000 No SP+Q275900  ";
version[32] = "8.00.205";  desc[32] = "2000 No SP+Q274330  ";
version[33] = "8.00.204";  desc[33] = "2000 No SP+Q274329  ";
version[34] = "8.00.194";  desc[34] = "2000 No SP  ";
version[35] = "8.00.190";  desc[35] = "2000 Gold, no SP ";
version[36] = "8.00.100";  desc[36] = "2000 Beta 2  ";
version[37] = "8.00.078";  desc[37] = "2000 EAP5   ";
version[38] = "8.00.047";  desc[38] = "2000 EAP4   ";
version[39] = "7.00.1077";  desc[39] = "7.0 SP4+Q316333   ";
version[40] = "7.00.1063";  desc[40] = "7.0 SP4   ";
version[41] = "7.00.1004";  desc[41] = "7.0 SP3+Q304851   ";
version[42] = "7.00.996";  desc[42] = "7.0 SP3 + hotfix ";
version[43] = "7.00.978";  desc[43] = "7.0 SP3+Q285870   ";
version[44] = "7.00.977";  desc[44] = "7.0 SP3+Q284351   ";
version[45] = "7.00.970";  desc[45] = "7.0 SP3+Q283837/282243   ";
version[46] = "7.00.961";  desc[46] = "7.0 SP3   ";
version[47] = "7.00.921";  desc[47] = "7.0 SP2+Q283837   ";
version[48] = "7.00.919";  desc[48] = "7.0 SP2+Q282243   ";
version[49] = "7.00.918";  desc[49] = "7.0 SP2+Q280380   ";
version[50] = "7.00.917";  desc[50] = "7.0 SP2+Q279180   ";
version[51] = "7.00.910";  desc[51] = "7.0 SP2+Q275901   ";
version[52] = "7.00.905";  desc[52] = "7.0 SP2+Q274266   ";
version[53] = "7.00.889";  desc[53] = "7.0 SP2+Q243741   ";
version[54] = "7.00.879";  desc[54] = "7.0 SP2+Q281185   ";
version[55] = "7.00.857";  desc[55] = "7.0 SP2+Q260346   ";
version[56] = "7.00.842";  desc[56] = "7.0 SP2   ";
version[57] = "7.00.835";  desc[57] = "7.0 SP2 Beta  ";
version[58] = "7.00.776";  desc[58] = "7.0 SP1+Q258087   ";
version[59] = "7.00.770";  desc[59] = "7.0 SP1+Q252905   ";
version[60] = "7.00.745";  desc[60] = "7.0 SP1+Q253738   ";
version[61] = "7.00.722";  desc[61] = "7.0 SP1+Q239458   ";
version[62] = "7.00.699";  desc[62] = "7.0 SP1   ";
version[63] = "7.00.689";  desc[63] = "7.0 SP1 Beta  ";
version[64] = "7.00.677";  desc[64] = "7.0 MSDE O2K Dev ";
version[65] = "7.00.662";  desc[65] = "7.0 Gold+Q232707   ";
version[66] = "7.00.658";  desc[66] = "7.0 Gold+Q244763   ";
version[67] = "7.00.657";  desc[67] = "7.0 Gold+Q229875   ";
version[68] = "7.00.643";  desc[68] = "7.0 Gold+Q220156   ";
version[69] = "7.00.623";  desc[69] = "7.0 Gold, no SP ";
version[70] = "7.00.583";  desc[70] = "7.0 RC1   ";
version[71] = "7.00.517";  desc[71] = "7.0 Beta 3  ";
version[72] = "7.00.416";  desc[72] = "7.0 SP5a   ";
version[73] = "7.00.415";  desc[73] = "7.0 SP5 ** BAD **";
version[74] = "7.00.339";  desc[74] = "7.0 SP4 + y2k ";
version[75] = "7.00.297";  desc[75] = "7.0 SP4 + SBS ";
version[76] = "7.00.281";  desc[76] = "7.0 SP4   ";
version[77] = "7.00.259";  desc[77] = "7.0 SP3 + SBS ";
version[78] = "7.00.258";  desc[78] = "7.0 SP3   ";
version[79] = "7.00.252";  desc[79] = "7.0 SP3 ** BAD **";
version[80] = "7.00.240";  desc[80] = "7.0 SP2   ";
version[81] = "7.00.213";  desc[81] = "7.0 SP1   ";
version[82] = "7.00.201";  desc[82] = "7.0 No SP  ";
version[83] = "7.00.198";  desc[83] = "7.0 Beta 1  ";
version[84] = "7.00.151";  desc[84] = "7.0 SP3   ";
version[85] = "7.00.139";  desc[85] = "7.0 SP2   ";
version[86] = "7.00.124";  desc[86] = "7.0 SP1   ";
version[87] = "7.00.121";  desc[87] = "7.0 No SP  ";
version[88] = "6.50.479";  desc[88] = "6.5 Post SP5a  ";
version[89] = "6.50.464";  desc[89] = "6.5 SP5a+Q275483   ";
version[90] = "6.50.416";  desc[90] = "6.5 SP5a   ";
version[91] = "6.50.415";  desc[91] = "6.5 Bad SP5  ";
version[92] = "6.50.339";  desc[92] = "6.5 Y2K Hotfix  ";
version[93] = "6.50.297";  desc[93] = "6.5 Site Server 3 ";
version[94] = "6.50.281";  desc[94] = "6.5 SP4   ";
version[95] = "6.50.259";  desc[95] = "6.5 SBS only  ";
version[96] = "6.50.258";  desc[96] = "6.5 SP3   ";
version[97] = "6.50.252";  desc[97] = "6.5 Bad SP3  ";
version[98] = "6.50.240";  desc[98] = "6.5 SP2   ";
version[99] = "6.50.213";  desc[99] = "6.5 SP1   ";
version[100] = "6.50.201";  desc[100] = "6.5 Gold   ";
version[101] = "6.00.151";  desc[101] = "6.0 SP3   ";
version[102] = "6.00.139";  desc[102] = "6.0 SP2   ";
version[103] = "6.00.124";  desc[103] = "6.0 SP1   ";
version[104] = "6.00.121";  desc[104] = "6.0 No SP  ";

#
# The script code starts here
#

include("smb_nt.inc");

    port = get_kb_item("SMB/transport");
    if(!port)port = 139;
  
    access = get_kb_item("SMB/registry_full_access");
    if(!access)exit(0);
    key = "SOFTWARE\Microsoft\MSSQLServer\MSSQLServer\CurrentVersion";
    item = "CSDVersion";
    value = registry_get_sz(key:key, item:item);
    if(!value)value = registry_get_sz(key:key, item:"CurrentVersion");
    if(!value)exit(0);
    set_kb_item(name:"mssql/SQLVersion",value:value);

    for (i=0; version[i] ; i = i + 1) {
        if ( version[i] >< value ) {
		        myret = string("The server is running MS SQL ", desc[i], value,"\n");
			if( (i == 0) || (i == 39) ) {
		           security_note(port:mssql_port, data:myret);
			   exit(0);
			   }
			if(i < 39) {
				myret = string(myret,"but needs ", desc[0],"due to security flaws\n");
			   }
			else {
				myret = string(myret,"but needs ", desc[39],"due to security flaws\n");
			   }
			security_hole(port:mssql_port, data:myret);
			exit(0);
        }
    }
    #
exit(0);
