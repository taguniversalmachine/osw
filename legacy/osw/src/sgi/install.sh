#!/bin/sh
INSTALL_DIR=`pwd`
BIN_DIR=/usr/local/bin
echo "#!/usr/bin/csh" > osw
echo "setenv OSW_DIR $INSTALL_DIR" >> osw
echo "setenv LD_LIBRARY_PATH .:$INSTALL_DIR:/usr/local/lib:/usr/lib:/lib" >> osw
echo "setenv LD_LIBRARY_PATH .:$INSTALL_DIR:/usr/local/lib:/usr/lib:/lib" >> osw
echo "setenv LD_RUN_PATH .:$INSTALL_DIR:/usr/local/lib:/usr/lib:/lib" >> osw
echo "wish8.3 $INSTALL_DIR/oswinit.tcl" '$1' >> osw
chmod a+x osw
mv osw $BIN_DIR

cp libosw.so /usr/local/lib

echo "#!/usr/bin/csh" > root-osw
echo "setenv OSW_DIR $INSTALL_DIR" >> root-osw
echo "setenv LD_LIBRARY_PATH .:$INSTALL_DIR:/usr/local/lib:/usr/lib:/lib" >> root-osw
echo "setenv LD_LIBRARYN32_PATH .:$INSTALL_DIR:/usr/local/lib:/usr/lib:/lib" >> root-osw
echo "setenv LD_RUN_PATH=.:$INSTALL_DIR:/usr/local/lib:/usr/lib:/lib" >> root-osw
echo "unlimit -h" >> root-osw
echo "unlimit" >> root-osw
echo "limit" >> root-osw
echo "wish8.3 $INSTALL_DIR/oswinit.tcl" '$1' >> root-osw
chmod a+x root-osw
cp root-osw $BIN_DIR	

#echo "OSWSourceDir $OSW_SOURCE_DIR/src" > externalizer/preferences.txt
echo "OSWInstallDir $INSTALL_DIR" >> externalizer/preferences.txt
echo "#!$TCL_DIR/bin/wish8.3" > $BIN_DIR/externalizer
echo "set env(OSW_DIR) $INSTALL_DIR" >> $BIN_DIR/externalizer
echo 'source $env(OSW_DIR)/externalizer/externalizer.tcl' >> $BIN_DIR/externalizer
chmod a+x $BIN_DIR/externalizer
cp oswbin $BIN_DIR
echo "OSW Installed Successfully!"
