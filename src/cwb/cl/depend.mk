globals.o: globals.c globals.h cl.h
macros.o: macros.c globals.h cl.h macros.h
list.o: list.c globals.h cl.h macros.h list.h
lexhash.o: lexhash.c globals.h cl.h macros.h lexhash.h
ngram-hash.o: ngram-hash.c globals.h cl.h macros.h lexhash.h ngram-hash.h
bitfields.o: bitfields.c globals.h cl.h macros.h bitfields.h
storage.o: storage.c globals.h cl.h endian2.h macros.h storage.h
fileutils.o: fileutils.c /usr/include/glib-2.0/glib.h \
 /usr/include/glib-2.0/glib/galloca.h /usr/include/glib-2.0/glib/gtypes.h \
 /usr/lib64/glib-2.0/include/glibconfig.h \
 /usr/include/glib-2.0/glib/gmacros.h \
 /usr/include/glib-2.0/glib/gversionmacros.h \
 /usr/include/glib-2.0/glib/garray.h \
 /usr/include/glib-2.0/glib/gasyncqueue.h \
 /usr/include/glib-2.0/glib/gthread.h \
 /usr/include/glib-2.0/glib/gatomic.h /usr/include/glib-2.0/glib/gerror.h \
 /usr/include/glib-2.0/glib/gquark.h /usr/include/glib-2.0/glib/gutils.h \
 /usr/include/glib-2.0/glib/gbacktrace.h \
 /usr/include/glib-2.0/glib/gbase64.h \
 /usr/include/glib-2.0/glib/gbitlock.h \
 /usr/include/glib-2.0/glib/gbookmarkfile.h \
 /usr/include/glib-2.0/glib/gbytes.h \
 /usr/include/glib-2.0/glib/gcharset.h \
 /usr/include/glib-2.0/glib/gchecksum.h \
 /usr/include/glib-2.0/glib/gconvert.h \
 /usr/include/glib-2.0/glib/gdataset.h /usr/include/glib-2.0/glib/gdate.h \
 /usr/include/glib-2.0/glib/gdatetime.h \
 /usr/include/glib-2.0/glib/gtimezone.h /usr/include/glib-2.0/glib/gdir.h \
 /usr/include/glib-2.0/glib/genviron.h \
 /usr/include/glib-2.0/glib/gfileutils.h \
 /usr/include/glib-2.0/glib/ggettext.h /usr/include/glib-2.0/glib/ghash.h \
 /usr/include/glib-2.0/glib/glist.h /usr/include/glib-2.0/glib/gmem.h \
 /usr/include/glib-2.0/glib/gnode.h /usr/include/glib-2.0/glib/ghmac.h \
 /usr/include/glib-2.0/glib/gchecksum.h \
 /usr/include/glib-2.0/glib/ghook.h \
 /usr/include/glib-2.0/glib/ghostutils.h \
 /usr/include/glib-2.0/glib/giochannel.h \
 /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gpoll.h \
 /usr/include/glib-2.0/glib/gslist.h /usr/include/glib-2.0/glib/gstring.h \
 /usr/include/glib-2.0/glib/gunicode.h \
 /usr/include/glib-2.0/glib/gkeyfile.h \
 /usr/include/glib-2.0/glib/gmappedfile.h \
 /usr/include/glib-2.0/glib/gmarkup.h \
 /usr/include/glib-2.0/glib/gmessages.h \
 /usr/include/glib-2.0/glib/gvariant.h \
 /usr/include/glib-2.0/glib/gvarianttype.h \
 /usr/include/glib-2.0/glib/goption.h \
 /usr/include/glib-2.0/glib/gpattern.h \
 /usr/include/glib-2.0/glib/gprimes.h /usr/include/glib-2.0/glib/gqsort.h \
 /usr/include/glib-2.0/glib/gqueue.h /usr/include/glib-2.0/glib/grand.h \
 /usr/include/glib-2.0/glib/grcbox.h \
 /usr/include/glib-2.0/glib/grefcount.h \
 /usr/include/glib-2.0/glib/grefstring.h \
 /usr/include/glib-2.0/glib/gmem.h /usr/include/glib-2.0/glib/gmacros.h \
 /usr/include/glib-2.0/glib/gregex.h \
 /usr/include/glib-2.0/glib/gscanner.h \
 /usr/include/glib-2.0/glib/gsequence.h \
 /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gslice.h \
 /usr/include/glib-2.0/glib/gspawn.h \
 /usr/include/glib-2.0/glib/gstrfuncs.h \
 /usr/include/glib-2.0/glib/gstringchunk.h \
 /usr/include/glib-2.0/glib/gtestutils.h \
 /usr/include/glib-2.0/glib/gthreadpool.h \
 /usr/include/glib-2.0/glib/gtimer.h \
 /usr/include/glib-2.0/glib/gtrashstack.h \
 /usr/include/glib-2.0/glib/gtree.h \
 /usr/include/glib-2.0/glib/gurifuncs.h \
 /usr/include/glib-2.0/glib/guuid.h /usr/include/glib-2.0/glib/gversion.h \
 /usr/include/glib-2.0/glib/deprecated/gallocator.h \
 /usr/include/glib-2.0/glib/deprecated/gcache.h \
 /usr/include/glib-2.0/glib/deprecated/gcompletion.h \
 /usr/include/glib-2.0/glib/deprecated/gmain.h \
 /usr/include/glib-2.0/glib/deprecated/grel.h \
 /usr/include/glib-2.0/glib/deprecated/gthread.h \
 /usr/include/glib-2.0/glib/glib-autocleanups.h globals.h cl.h \
 fileutils.h
special-chars.o: special-chars.c /usr/include/glib-2.0/glib.h \
 /usr/include/glib-2.0/glib/galloca.h /usr/include/glib-2.0/glib/gtypes.h \
 /usr/lib64/glib-2.0/include/glibconfig.h \
 /usr/include/glib-2.0/glib/gmacros.h \
 /usr/include/glib-2.0/glib/gversionmacros.h \
 /usr/include/glib-2.0/glib/garray.h \
 /usr/include/glib-2.0/glib/gasyncqueue.h \
 /usr/include/glib-2.0/glib/gthread.h \
 /usr/include/glib-2.0/glib/gatomic.h /usr/include/glib-2.0/glib/gerror.h \
 /usr/include/glib-2.0/glib/gquark.h /usr/include/glib-2.0/glib/gutils.h \
 /usr/include/glib-2.0/glib/gbacktrace.h \
 /usr/include/glib-2.0/glib/gbase64.h \
 /usr/include/glib-2.0/glib/gbitlock.h \
 /usr/include/glib-2.0/glib/gbookmarkfile.h \
 /usr/include/glib-2.0/glib/gbytes.h \
 /usr/include/glib-2.0/glib/gcharset.h \
 /usr/include/glib-2.0/glib/gchecksum.h \
 /usr/include/glib-2.0/glib/gconvert.h \
 /usr/include/glib-2.0/glib/gdataset.h /usr/include/glib-2.0/glib/gdate.h \
 /usr/include/glib-2.0/glib/gdatetime.h \
 /usr/include/glib-2.0/glib/gtimezone.h /usr/include/glib-2.0/glib/gdir.h \
 /usr/include/glib-2.0/glib/genviron.h \
 /usr/include/glib-2.0/glib/gfileutils.h \
 /usr/include/glib-2.0/glib/ggettext.h /usr/include/glib-2.0/glib/ghash.h \
 /usr/include/glib-2.0/glib/glist.h /usr/include/glib-2.0/glib/gmem.h \
 /usr/include/glib-2.0/glib/gnode.h /usr/include/glib-2.0/glib/ghmac.h \
 /usr/include/glib-2.0/glib/gchecksum.h \
 /usr/include/glib-2.0/glib/ghook.h \
 /usr/include/glib-2.0/glib/ghostutils.h \
 /usr/include/glib-2.0/glib/giochannel.h \
 /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gpoll.h \
 /usr/include/glib-2.0/glib/gslist.h /usr/include/glib-2.0/glib/gstring.h \
 /usr/include/glib-2.0/glib/gunicode.h \
 /usr/include/glib-2.0/glib/gkeyfile.h \
 /usr/include/glib-2.0/glib/gmappedfile.h \
 /usr/include/glib-2.0/glib/gmarkup.h \
 /usr/include/glib-2.0/glib/gmessages.h \
 /usr/include/glib-2.0/glib/gvariant.h \
 /usr/include/glib-2.0/glib/gvarianttype.h \
 /usr/include/glib-2.0/glib/goption.h \
 /usr/include/glib-2.0/glib/gpattern.h \
 /usr/include/glib-2.0/glib/gprimes.h /usr/include/glib-2.0/glib/gqsort.h \
 /usr/include/glib-2.0/glib/gqueue.h /usr/include/glib-2.0/glib/grand.h \
 /usr/include/glib-2.0/glib/grcbox.h \
 /usr/include/glib-2.0/glib/grefcount.h \
 /usr/include/glib-2.0/glib/grefstring.h \
 /usr/include/glib-2.0/glib/gmem.h /usr/include/glib-2.0/glib/gmacros.h \
 /usr/include/glib-2.0/glib/gregex.h \
 /usr/include/glib-2.0/glib/gscanner.h \
 /usr/include/glib-2.0/glib/gsequence.h \
 /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gslice.h \
 /usr/include/glib-2.0/glib/gspawn.h \
 /usr/include/glib-2.0/glib/gstrfuncs.h \
 /usr/include/glib-2.0/glib/gstringchunk.h \
 /usr/include/glib-2.0/glib/gtestutils.h \
 /usr/include/glib-2.0/glib/gthreadpool.h \
 /usr/include/glib-2.0/glib/gtimer.h \
 /usr/include/glib-2.0/glib/gtrashstack.h \
 /usr/include/glib-2.0/glib/gtree.h \
 /usr/include/glib-2.0/glib/gurifuncs.h \
 /usr/include/glib-2.0/glib/guuid.h /usr/include/glib-2.0/glib/gversion.h \
 /usr/include/glib-2.0/glib/deprecated/gallocator.h \
 /usr/include/glib-2.0/glib/deprecated/gcache.h \
 /usr/include/glib-2.0/glib/deprecated/gcompletion.h \
 /usr/include/glib-2.0/glib/deprecated/gmain.h \
 /usr/include/glib-2.0/glib/deprecated/grel.h \
 /usr/include/glib-2.0/glib/deprecated/gthread.h \
 /usr/include/glib-2.0/glib/glib-autocleanups.h globals.h cl.h \
 special-chars.h
regopt.o: regopt.c /usr/include/glib-2.0/glib.h \
 /usr/include/glib-2.0/glib/galloca.h /usr/include/glib-2.0/glib/gtypes.h \
 /usr/lib64/glib-2.0/include/glibconfig.h \
 /usr/include/glib-2.0/glib/gmacros.h \
 /usr/include/glib-2.0/glib/gversionmacros.h \
 /usr/include/glib-2.0/glib/garray.h \
 /usr/include/glib-2.0/glib/gasyncqueue.h \
 /usr/include/glib-2.0/glib/gthread.h \
 /usr/include/glib-2.0/glib/gatomic.h /usr/include/glib-2.0/glib/gerror.h \
 /usr/include/glib-2.0/glib/gquark.h /usr/include/glib-2.0/glib/gutils.h \
 /usr/include/glib-2.0/glib/gbacktrace.h \
 /usr/include/glib-2.0/glib/gbase64.h \
 /usr/include/glib-2.0/glib/gbitlock.h \
 /usr/include/glib-2.0/glib/gbookmarkfile.h \
 /usr/include/glib-2.0/glib/gbytes.h \
 /usr/include/glib-2.0/glib/gcharset.h \
 /usr/include/glib-2.0/glib/gchecksum.h \
 /usr/include/glib-2.0/glib/gconvert.h \
 /usr/include/glib-2.0/glib/gdataset.h /usr/include/glib-2.0/glib/gdate.h \
 /usr/include/glib-2.0/glib/gdatetime.h \
 /usr/include/glib-2.0/glib/gtimezone.h /usr/include/glib-2.0/glib/gdir.h \
 /usr/include/glib-2.0/glib/genviron.h \
 /usr/include/glib-2.0/glib/gfileutils.h \
 /usr/include/glib-2.0/glib/ggettext.h /usr/include/glib-2.0/glib/ghash.h \
 /usr/include/glib-2.0/glib/glist.h /usr/include/glib-2.0/glib/gmem.h \
 /usr/include/glib-2.0/glib/gnode.h /usr/include/glib-2.0/glib/ghmac.h \
 /usr/include/glib-2.0/glib/gchecksum.h \
 /usr/include/glib-2.0/glib/ghook.h \
 /usr/include/glib-2.0/glib/ghostutils.h \
 /usr/include/glib-2.0/glib/giochannel.h \
 /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gpoll.h \
 /usr/include/glib-2.0/glib/gslist.h /usr/include/glib-2.0/glib/gstring.h \
 /usr/include/glib-2.0/glib/gunicode.h \
 /usr/include/glib-2.0/glib/gkeyfile.h \
 /usr/include/glib-2.0/glib/gmappedfile.h \
 /usr/include/glib-2.0/glib/gmarkup.h \
 /usr/include/glib-2.0/glib/gmessages.h \
 /usr/include/glib-2.0/glib/gvariant.h \
 /usr/include/glib-2.0/glib/gvarianttype.h \
 /usr/include/glib-2.0/glib/goption.h \
 /usr/include/glib-2.0/glib/gpattern.h \
 /usr/include/glib-2.0/glib/gprimes.h /usr/include/glib-2.0/glib/gqsort.h \
 /usr/include/glib-2.0/glib/gqueue.h /usr/include/glib-2.0/glib/grand.h \
 /usr/include/glib-2.0/glib/grcbox.h \
 /usr/include/glib-2.0/glib/grefcount.h \
 /usr/include/glib-2.0/glib/grefstring.h \
 /usr/include/glib-2.0/glib/gmem.h /usr/include/glib-2.0/glib/gmacros.h \
 /usr/include/glib-2.0/glib/gregex.h \
 /usr/include/glib-2.0/glib/gscanner.h \
 /usr/include/glib-2.0/glib/gsequence.h \
 /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gslice.h \
 /usr/include/glib-2.0/glib/gspawn.h \
 /usr/include/glib-2.0/glib/gstrfuncs.h \
 /usr/include/glib-2.0/glib/gstringchunk.h \
 /usr/include/glib-2.0/glib/gtestutils.h \
 /usr/include/glib-2.0/glib/gthreadpool.h \
 /usr/include/glib-2.0/glib/gtimer.h \
 /usr/include/glib-2.0/glib/gtrashstack.h \
 /usr/include/glib-2.0/glib/gtree.h \
 /usr/include/glib-2.0/glib/gurifuncs.h \
 /usr/include/glib-2.0/glib/guuid.h /usr/include/glib-2.0/glib/gversion.h \
 /usr/include/glib-2.0/glib/deprecated/gallocator.h \
 /usr/include/glib-2.0/glib/deprecated/gcache.h \
 /usr/include/glib-2.0/glib/deprecated/gcompletion.h \
 /usr/include/glib-2.0/glib/deprecated/gmain.h \
 /usr/include/glib-2.0/glib/deprecated/grel.h \
 /usr/include/glib-2.0/glib/deprecated/gthread.h \
 /usr/include/glib-2.0/glib/glib-autocleanups.h globals.h cl.h regopt.h
corpus.o: corpus.c globals.h cl.h macros.h attributes.h storage.h \
 corpus.h registry.tab.h
attributes.o: attributes.c globals.h cl.h endian2.h corpus.h macros.h \
 fileutils.h cdaccess.h makecomps.h attributes.h storage.h list.h
makecomps.o: makecomps.c globals.h cl.h endian2.h macros.h storage.h \
 fileutils.h corpus.h attributes.h cdaccess.h makecomps.h
registry.tab.o: registry.tab.c globals.h cl.h corpus.h macros.h \
 attributes.h storage.h
lex.creg.o: lex.creg.c globals.h cl.h corpus.h registry.tab.h
cdaccess.o: cdaccess.c globals.h cl.h endian2.h macros.h attributes.h \
 storage.h corpus.h special-chars.h bitio.h compression.h regopt.h \
 cdaccess.h
bitio.o: bitio.c globals.h cl.h endian2.h bitio.h
endian.o: endian.c globals.h cl.h endian2.h
compression.o: compression.c globals.h cl.h bitio.h compression.h
binsert.o: binsert.c globals.h cl.h macros.h binsert.h
class-mapping.o: class-mapping.c globals.h cl.h macros.h cdaccess.h \
 class-mapping.h corpus.h attributes.h storage.h
