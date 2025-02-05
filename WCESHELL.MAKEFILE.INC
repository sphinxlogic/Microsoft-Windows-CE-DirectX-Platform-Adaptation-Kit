
_CPLMAIN_ALLCOMPONENTS=cplmain network comm datetime dialing keyboard password owner power system display backlight colorscheme pointercmn stylus mouse sounds sip remove

#
# Recurse for each combination of (P, L) x (loc, NL)
#
resfiles:
	@set _LOC=
	@set _ASPECT=L
	@set _ASPECTFULL=landscape
	$(MAKE) /NOLOGO resfiles1
	@set _LOC=nl
	@set _ASPECT=L
	@set _ASPECTFULL=landscape
	$(MAKE) /NOLOGO resfiles1
	@set _LOC=
	@set _ASPECT=P
	@set _ASPECTFULL=portrait
	$(MAKE) /NOLOGO resfiles1
	@set _LOC=nl
	@set _ASPECT=P
	@set _ASPECTFULL=portrait
	$(MAKE) /NOLOGO resfiles1
	
resfiles1: $(_CPLMAIN_ALLCOMPONENTS)


#
# Recurse for each component
#
$(_CPLMAIN_ALLCOMPONENTS): 
	@set _COMPNAME=$@
	$(MAKE) /NOLOGO $(_RELEASELIBDIR)\$@$(_ASPECT)$(_LOC).res

$(_RELEASELIBDIR)\$(_COMPNAME)$(_ASPECT)$(_LOC).res: $(_OBJDIR)\$(_COMPNAME)$(_ASPECT)$(_LOC).res
	copy $? $@

$(_OBJDIR)\$(_COMPNAME)$(_ASPECT)$(_LOC).res: $(_OBJDIR)\$(_COMPNAME)$(_ASPECT)$(_LOC).rc
    $(RCOMPILER) $(RCFLAGS) -fo$@ $?

$(_OBJDIR)\$(_COMPNAME)$(_ASPECT)$(_LOC).rc: cplmain$(_LOC).rc cplres.h
	@set CE_MODULES=cplmain
	@set CPLMAIN_COMPONENTS=$(_ASPECTFULL) $(_COMPNAME)
	-@del $@ > nul 2>&1
	cefilter cplmain$(_LOC).rc $@ -N
