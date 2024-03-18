Name:           tdx-quote-generation-sample
Version:        @version@
Release:        1%{?dist}
Summary:        Sample code for TDX Quote Generation

License:        BSD License
URL:            https://www.alibabacloud.com/help/zh/ecs/user-guide/build-a-tdx-confidential-computing-environment
Source0:        %{name}-%{version}.tar.gz
Requires:       libtdx-attest-devel gcc make
BuildArch:      x86_64

%description
This package contains sample code for TDX Quote Generation.

%prep
%setup -q

%build

%install
mkdir -p %{buildroot}/opt/alibaba/tdx-quote-generation-sample
install -m 644 main.c %{buildroot}/opt/alibaba/tdx-quote-generation-sample
install -m 644 Makefile %{buildroot}/opt/alibaba/tdx-quote-generation-sample
install -m 644 README.md %{buildroot}/opt/alibaba/tdx-quote-generation-sample
install -m 644 README.zh.md %{buildroot}/opt/alibaba/tdx-quote-generation-sample
install -m 644 LICENSE %{buildroot}/opt/alibaba/tdx-quote-generation-sample

%files
/opt/alibaba/tdx-quote-generation-sample/main.c
/opt/alibaba/tdx-quote-generation-sample/Makefile
/opt/alibaba/tdx-quote-generation-sample/README.md
/opt/alibaba/tdx-quote-generation-sample/README.zh.md
/opt/alibaba/tdx-quote-generation-sample/LICENSE

%changelog
* Sun Mar 17 2024 Guorui.Yu <Guorui.Yu@linux.alibaba.com> - 1.0-1
- Initial package
