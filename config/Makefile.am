sysconf_DATA = nagperfdiagd.conf

EXTRA_DIST = nagperfdiagd.conf.in

CLEANFILES = $(sysconf_DATA)

nagperfdiagd.conf: nagperfdiagd.conf.in
	@RM@ -f $@ $@-temp
	@SED@ \
		-e 's!\@VARRUNDIR\@!$(VARRUNDIR)!g' \
		-e 's!\@VARLOGDIR\@!$(VARLOGDIR)!g' \
		-e 's!\@SYSCONFDIR\@!$(sysconfdir)!g' \
		$< > $@-temp
	@MV@  $@-temp $@
