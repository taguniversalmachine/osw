Summary: An extensible graphical signal processing environment.
Name: osw
Version: 1.2.2
Release: 1
Copyright: OSW Public License
Source: osw-1.2.2.tar.bz2
Group: Applications/Multimedia
BuildRoot: /var/tmp/%{name}-buildroot

%description
Open Sound World, or OSW, is a scalable, extensible programming environment that allows musicians, sound designers and researchers to process sound in response to expressive real-time control. OSW combines a familiar visual patching paradigm with solid programming-language features such as a strong type system and hierarchical name spaces. OSW also includes an intuitive model for specifying new components using a graphical interface and high-level C++ expressions, making it easy to develop and share new music and signal-processing algoritms.

%prep
%setup

%build
cd src/linux
./configure --prefix=$RPM_BUILD_ROOT/usr --enable-alsa FINAL_DIR=/usr
make 

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/lib

cd src/linux
make install
make externals
make rpmexecs

%clean
rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig

%files
/usr/lib/osw
/usr/lib/libosw.so
/usr/bin/oswbin
/usr/bin/osw
/usr/bin/externalizer
