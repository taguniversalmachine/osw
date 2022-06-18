#!/bin/sh

#edit these lines to reflect your preferences

BIN_DIR=/usr/local/bin
LIB_DIR=/usr/local/lib
WISH=wish8.3

#do not edit below this line!

INSTALL_DIR=`pwd`

echo "#!/bin/sh" > osw
echo "OSW_DIR=$INSTALL_DIR" >> osw
echo 'export OSW_DIR' >> osw
echo "LD_LIBRARY_PATH=$LIB_DIR" >> osw
echo 'export LD_LIBRARY_PATH' >> osw
echo "$WISH $INSTALL_DIR/oswinit.tcl" '$1' >> osw
chmod a+x osw
mv osw $BIN_DIR
echo "OSWSourceDir $INSTALL_DIR/src" > externalizer/preferences.txt
echo "OSWInstallDir $INSTALL_DIR" >> externalizer/preferences.txt
echo "#!"`which $WISH` > $BIN_DIR/externalizer
echo "set env(OSW_DIR) $INSTALL_DIR" >> $BIN_DIR/externalizer
echo 'source $env(OSW_DIR)/externalizer/externalizer.tcl' >> $BIN_DIR/externalizer
chmod a+x $BIN_DIR/externalizer
cp oswbin $BIN_DIR
cp libosw.so $LIB_DIR
echo "OSW Installed Successfully!"
