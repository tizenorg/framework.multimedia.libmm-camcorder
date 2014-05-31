Name:       libmm-camcorder
Summary:    Camera and recorder library
Version:    0.7.18
Release:    0
Group:      libs
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Requires(post): /usr/bin/vconftool
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
%if "%{_repository}" == "wearable"
BuildRequires:  pkgconfig(capi-system-info)
BuildRequires:  pkgconfig(gstreamer-app-0.10)
%endif
BuildRequires:  pkgconfig(mm-common)
BuildRequires:  pkgconfig(mm-sound)
BuildRequires:  pkgconfig(avsystem)
BuildRequires:  pkgconfig(libexif)
BuildRequires:  pkgconfig(mmutil-imgp)
BuildRequires:  pkgconfig(mm-log)
BuildRequires:  pkgconfig(gstreamer-plugins-base-0.10)
BuildRequires:  pkgconfig(mm-ta)
BuildRequires:  pkgconfig(sndfile)
BuildRequires:  pkgconfig(mm-session)
BuildRequires:  pkgconfig(audio-session-mgr)
BuildRequires:  pkgconfig(camsrcjpegenc)
%if "%{_repository}" == "mobile"
BuildRequires:  pkgconfig(libpulse)
%endif
BuildRequires:  pkgconfig(vconf)
BuildRequires:  gst-plugins-base-devel

%description
Camera and recorder library.


%package devel
Summary:    Camera and recorder development library
Group:      libdevel
Version:    %{version}
Requires:   %{name} = %{version}-%{release}

%description devel 
Camera and recorder development library.


%prep
%setup -q 


%build
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS+=" -DGST_EXT_TIME_ANALYSIS -DTIZEN_DEBUG_ENABLE"
%else
export CFLAGS+=" -DGST_EXT_TIME_ANALYSIS"
%endif
%if 0%{?tizen_profile_wearable}
cd ./wearable
export CFLAGS+=" -DUSE_ASM_SUBEVENT"
%else
cd ./mobile
%endif
./autogen.sh
%configure --disable-static
make %{?jobs:-j%jobs}

%install
%if 0%{?tizen_profile_wearable}
cd ./wearable
%else
cd ./mobile
%endif
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp LICENSE.APLv2 %{buildroot}/usr/share/license/%{name}
%make_install


%post
/sbin/ldconfig

%if 0%{?tizen_profile_wearable}
vconftool set -t int memory/camera/state 0 -i -u 5000 -s system::vconf_multimedia
vconftool set -t int file/camera/shutter_sound_policy 1 -u 5000 -s system::vconf_inhouse
%else
vconftool set -t int memory/camera/state 0 -i -u 5000
vconftool set -t int file/camera/shutter_sound_policy 0 -u 5000
%endif

%postun -p /sbin/ldconfig

%files
%if 0%{?tizen_profile_mobile}
%manifest ./mobile/libmm-camcorder.manifest
%endif
%defattr(-,root,root,-)
%{_bindir}/*
%{_libdir}/*.so.*
/usr/share/sounds/mm-camcorder/*
%{_datadir}/license/%{name}

%files devel
%if 0%{?tizen_profile_mobile}
%manifest ./mobile/libmm-camcorder.manifest
%endif
%defattr(-,root,root,-)
%{_includedir}/mmf/mm_camcorder.h
%{_libdir}/pkgconfig/mm-camcorder.pc
%{_libdir}/*.so
