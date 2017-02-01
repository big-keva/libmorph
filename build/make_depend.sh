#!/bin/sh

if [ "$mkdeps" = "" ]; then
        echo "This script would be called by the Makefile!"
        exit 1
fi

if [ "$objdir" = "" ]; then
        echo "This script would be caleld by the Makefile!"
        exit 1
fi

echo "\$(OUTPUT): \$(OBJSET)" >> $objdir/.depend

case "$target" in
  "EXE" )
    echo "	@echo Creating executable \$(OUTPUT) ..." >> $objdir/.depend
    echo "	@\$(CC) \$(LFLAGS) -o \$(OUTPUT) \$(OBJSET) \$(LIBSET)" >> $objdir/.depend ;;
  "SO" )
    echo "	@echo Creating shared library \$(OUTPUT) ..." >> $objdir/.depend
    echo "	@\$(CC) \$(LFLAGS) -o \$(OUTPUT) \$(OBJSET) \$(LIBSET))" >> $objdir/.depend ;;
  "LIB" )
    echo "	@echo Creating library \$(OUTPUT) ..." >> $objdir/.depend
    echo "	@ar -rc \$(OUTPUT) \$(OBJSET)" >> $objdir/.depend
    ;;
esac

for src
do
        echo -n $objdir/ >> $objdir/.depend
        if ! $mkdeps $src >> $objdir/.depend
        then
          rm -f $objdir/.depend
          exit 1
        fi
        echo "	@echo Compiling \$< ..."	  >> $objdir/.depend
        echo "	@\$(CC) \$(CFLAGS) -o \$@ -c \$<" >> $objdir/.depend
done


