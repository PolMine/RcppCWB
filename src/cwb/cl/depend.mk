globals.o: globals.c globals.h cl.h
macros.o: macros.c globals.h cl.h macros.h
list.o: list.c globals.h cl.h macros.h list.h
lexhash.o: lexhash.c globals.h cl.h macros.h lexhash.h
ngram-hash.o: ngram-hash.c globals.h cl.h macros.h lexhash.h ngram-hash.h
bitfields.o: bitfields.c globals.h cl.h macros.h bitfields.h
storage.o: storage.c globals.h cl.h endian2.h macros.h storage.h
fileutils.o: fileutils.c \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/galloca.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gtypes.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/lib/x86_64/glib-2.0/include/glibconfig.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gmacros.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gversionmacros.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/garray.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gasyncqueue.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gthread.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gatomic.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gerror.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gquark.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gutils.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gbacktrace.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gbase64.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gbitlock.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gbookmarkfile.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gbytes.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gcharset.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gchecksum.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gconvert.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gdataset.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gdate.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gdatetime.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gtimezone.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gdir.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/genviron.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gfileutils.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/ggettext.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/ghash.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/glist.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gmem.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gnode.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/ghmac.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/ghook.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/ghostutils.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/giochannel.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gmain.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gpoll.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gslist.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gstring.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gunicode.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gkeyfile.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gmappedfile.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gmarkup.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gmessages.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gvariant.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gvarianttype.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/goption.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gpattern.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gprimes.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gqsort.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gqueue.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/grand.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gregex.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gscanner.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gsequence.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gshell.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gslice.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gspawn.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gstrfuncs.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gstringchunk.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gtestutils.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gthreadpool.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gtimer.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gtrashstack.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gtree.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gurifuncs.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/guuid.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gversion.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/deprecated/gallocator.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/deprecated/gcache.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/deprecated/gcompletion.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/deprecated/gmain.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/deprecated/grel.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/deprecated/gthread.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/glib-autocleanups.h \
  globals.h cl.h fileutils.h
special-chars.o: special-chars.c \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/galloca.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gtypes.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/lib/x86_64/glib-2.0/include/glibconfig.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gmacros.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gversionmacros.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/garray.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gasyncqueue.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gthread.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gatomic.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gerror.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gquark.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gutils.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gbacktrace.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gbase64.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gbitlock.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gbookmarkfile.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gbytes.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gcharset.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gchecksum.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gconvert.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gdataset.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gdate.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gdatetime.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gtimezone.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gdir.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/genviron.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gfileutils.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/ggettext.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/ghash.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/glist.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gmem.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gnode.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/ghmac.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/ghook.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/ghostutils.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/giochannel.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gmain.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gpoll.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gslist.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gstring.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gunicode.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gkeyfile.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gmappedfile.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gmarkup.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gmessages.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gvariant.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gvarianttype.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/goption.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gpattern.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gprimes.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gqsort.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gqueue.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/grand.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gregex.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gscanner.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gsequence.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gshell.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gslice.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gspawn.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gstrfuncs.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gstringchunk.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gtestutils.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gthreadpool.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gtimer.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gtrashstack.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gtree.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gurifuncs.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/guuid.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gversion.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/deprecated/gallocator.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/deprecated/gcache.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/deprecated/gcompletion.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/deprecated/gmain.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/deprecated/grel.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/deprecated/gthread.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/glib-autocleanups.h \
  globals.h cl.h special-chars.h
regopt.o: regopt.c \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/galloca.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gtypes.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/lib/x86_64/glib-2.0/include/glibconfig.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gmacros.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gversionmacros.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/garray.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gasyncqueue.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gthread.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gatomic.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gerror.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gquark.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gutils.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gbacktrace.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gbase64.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gbitlock.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gbookmarkfile.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gbytes.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gcharset.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gchecksum.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gconvert.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gdataset.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gdate.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gdatetime.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gtimezone.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gdir.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/genviron.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gfileutils.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/ggettext.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/ghash.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/glist.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gmem.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gnode.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/ghmac.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/ghook.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/ghostutils.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/giochannel.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gmain.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gpoll.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gslist.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gstring.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gunicode.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gkeyfile.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gmappedfile.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gmarkup.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gmessages.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gvariant.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gvarianttype.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/goption.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gpattern.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gprimes.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gqsort.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gqueue.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/grand.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gregex.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gscanner.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gsequence.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gshell.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gslice.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gspawn.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gstrfuncs.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gstringchunk.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gtestutils.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gthreadpool.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gtimer.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gtrashstack.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gtree.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gurifuncs.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/guuid.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/gversion.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/deprecated/gallocator.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/deprecated/gcache.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/deprecated/gcompletion.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/deprecated/gmain.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/deprecated/grel.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/deprecated/gthread.h \
  /Users/andreasblaette/Lab/github/RcppCWB/macos/libglib-master/include/glib-2.0/glib/glib-autocleanups.h \
  globals.h cl.h regopt.h /usr/local/Cellar/pcre/8.44/include/pcre.h
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
  /usr/local/Cellar/pcre/8.44/include/pcre.h cdaccess.h
bitio.o: bitio.c globals.h cl.h endian2.h bitio.h
endian.o: endian.c globals.h cl.h endian2.h
compression.o: compression.c globals.h cl.h bitio.h compression.h
binsert.o: binsert.c globals.h cl.h macros.h binsert.h
class-mapping.o: class-mapping.c globals.h cl.h macros.h cdaccess.h \
  class-mapping.h corpus.h attributes.h storage.h
