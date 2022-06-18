#!/bin/sh

#edit these lines to reflect your preferences

BIN_DIR=/usr/local/bin
LIB_DIR=/usr/local/lib
WISH=wish8.4

#do not edit below this line!

INSTALL_DIR=$LIB_DIR/osw

cp -r osw $LIB_DIR

echo "#!/bin/sh" > $BIN_DIR/osw
echo "OSW_DIR=$INSTALL_DIR" >> $BIN_DIR/osw
echo 'export OSW_DIR' >> $BIN_DIR/osw
echo "LD_LIBRARY_PATH=$LIB_DIR" >> $BIN_DIR/osw
echo 'export LD_LIBRARY_PATH' >> $BIN_DIR/osw
echo "$WISH $INSTALL_DIR/oswinit.tcl" '$1' >> $BIN_DIR/osw
chmod a+x $BIN_DIR/osw

echo "OSWSourceDir $INSTALL_DIR/src" > $INSTALL_DIR/externalizer/preferences.txt
echo "OSWInstallDir $INSTALL_DIR" >> $INSTALL_DIR/externalizer/preferences.txt
echo "#!"`which $WISH` > $BIN_DIR/externalizer
echo "set env(OSW_DIR) $INSTALL_DIR" >> $BIN_DIR/externalizer
echo 'source $env(OSW_DIR)/externalizer/externalizer.tcl' >> $BIN_DIR/externalizer
chmod a+x $BIN_DIR/externalizer
cp oswbin $BIN_DIR
cp libosw.dylib $LIB_DIR

echo "OSW Installed Successfully!"
