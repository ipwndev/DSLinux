#!/usr/bin/perl

print "static PERL_SIGNAL_ARGS_REC perl_signal_args[] =\n{\n";

while (<STDIN>) {
	chomp;

	next if (!/^ "([^"]*)"(<.*>)?,\s*(.*)/);
	next if (/\.\.\./);
	next if (/\(/);

	$signal = $1;
	$_ = $3;

	s/GList \* of ([^,]*)/glistptr_\1/g;
	s/GSList of (\w+)s/gslist_\1/g;

	s/char \*[^,]*/string/g;
	s/ulong \*[^,]*/ulongptr/g;
	s/int \*[^,]*/intptr/g;
	s/int [^,]*/int/g;

	# core
        s/CHATNET_REC[^,]*/iobject/g;
        s/SERVER_REC[^,]*/iobject/g;
        s/RECONNECT_REC[^,]*/iobject/g;
	s/CHANNEL_REC[^,]*/iobject/g;
	s/QUERY_REC[^,]*/iobject/g;
	s/COMMAND_REC[^,]*/Irssi::Command/g;
	s/NICK_REC[^,]*/iobject/g;
	s/LOG_REC[^,]*/Irssi::Log/g;
	s/RAWLOG_REC[^,]*/Irssi::Rawlog/g;
	s/IGNORE_REC[^,]*/Irssi::Ignore/g;
	s/MODULE_REC[^,]*/Irssi::Module/g;

	# irc
	s/BAN_REC[^,]*/Irssi::Irc::Ban/g;
	s/NETSPLIT_REC[^,]*/Irssi::Irc::Netsplit/g;
	s/NETSPLIT_SERVER_REC[^,]*/Irssi::Irc::Netsplitserver/g;

	# irc modules
	s/DCC_REC[^,]*/siobject/g;
	s/AUTOIGNORE_REC[^,]*/Irssi::Irc::Autoignore/g;
	s/NOTIFYLIST_REC[^,]*/Irssi::Irc::Notifylist/g;

	# fe-common
	s/THEME_REC[^,]*/Irssi::UI::Theme/g;
	s/KEYINFO_REC[^,]*/Irssi::UI::Keyinfo/g;
	s/PROCESS_REC[^,]*/Irssi::UI::Process/g;
	s/TEXT_DEST_REC[^,]*/Irssi::UI::TextDest/g;
	s/WINDOW_REC[^,]*/Irssi::UI::Window/g;
	s/WI_ITEM_REC[^,]*/iobject/g;

	# perl
	s/PERL_SCRIPT_REC[^,]*/Irssi::Script/g;

	s/([\w\*:]+)(,|$)/"\1"\2/g;
	print "    { \"$signal\", { $_, NULL } },\n";
}

print "\n    { NULL }\n};\n";
