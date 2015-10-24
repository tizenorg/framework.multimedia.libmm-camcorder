Name:       libmm-camcorder
Summary:    Camera and recorder library
Version:    0.9.125
Release:    0
Group:      libdevel
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Requires(post): /usr/bin/vconftool
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(gio-2.0)
BuildRequires:  pkgconfig(capi-system-info)
BuildRequires:  pkgconfig(mm-common)
BuildRequires:  pkgconfig(mm-sound)
BuildRequires:  pkgconfig(libexif)
BuildRequires:  pkgconfig(mmutil-imgp)
BuildRequires:  pkgconfig(mmutil-jpeg)
BuildRequires:  pkgconfig(gstreamer-base-1.0)
BuildRequires:  pkgconfig(gstreamer-video-1.0)
BuildRequires:  pkgconfig(gstreamer-app-1.0)
BuildRequires:  pkgconfig(sndfile)
BuildRequires:  pkgconfig(mm-session)
BuildRequires:  pkgconfig(audio-session-mgr)
BuildRequires:  pkgconfig(libpulse)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  gstreamer1.0-devel
BuildRequires:  pkgconfig(ttrace)
BuildRequires:  pkgconfig(libtbm)
BuildRequires:  pkgconfig(storage)

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
export CFLAGS+=" -DTIZEN_DEBUG_ENABLE"
%endif
export CFLAGS+=" -Wall -Wextra -Wno-array-bounds -Wno-empty-body -Wno-ignored-qualifiers -Wno-unused-parameter -Wshadow -Wwrite-strings -Wswitch-default -Wno-unused-but-set-parameter -Wno-unused-but-set-variable -D_LARGEFILE64_SOURCE"
#export CFLAGS+=" -Werror"
./autogen.sh
%configure \
	--disable-static
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp LICENSE.APLv2 %{buildroot}/usr/share/license/%{name}
%make_install


%post
/sbin/ldconfig

chsmack -a "device::camera" /usr/share/sounds/mm-camcorder/camera_resource
chsmack -a "pulseaudio::record" /usr/share/sounds/mm-camcorder/recorder_resource

%postun -p /sbin/ldconfig

%files
%manifest libmm-camcorder.manifest
%defattr(-,root,root,-)
%{_bindir}/*
%{_libdir}/*.so.*
/usr/share/sounds/mm-camcorder/*
%{_datadir}/license/%{name}

%files devel
%defattr(-,root,root,-)
%{_includedir}/mmf/mm_camcorder.h
%{_libdir}/pkgconfig/mm-camcorder.pc
%{_libdir}/*.so
