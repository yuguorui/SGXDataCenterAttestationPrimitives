Name:           tdx-quote-verification-sample
Version:        @version@
Release:        1%{?dist}
Summary:        Sample code for TDX Quote Verification

License:        BSD License
URL:            https://www.alibabacloud.com/help/zh/ecs/user-guide/build-a-tdx-confidential-computing-environment
Source0:        %{name}-%{version}.tar.gz
Requires:       make gcc gcc-c++ openssl-devel jq tee-appraisal-tool libsgx-dcap-quote-verify-devel libsgx-dcap-default-qpl-devel
BuildArch:      x86_64

%description
This package contains sample code for TDX Quote Verification.

%prep
%setup -q

%build

%install
mkdir -p %{buildroot}/opt/alibaba/tdx-quote-verification-sample
install -m 644 relying_party.cpp verifier.cpp Makefile README.md README.zh.md LICENSE %{buildroot}/opt/alibaba/tdx-quote-verification-sample
cp -a external/ %{buildroot}/opt/alibaba/tdx-quote-verification-sample

mkdir -p %{buildroot}/opt/alibaba/tdx-quote-verification-sample/Policies
install -m 644 Policies/tenant_td_policy.json %{buildroot}/opt/alibaba/tdx-quote-verification-sample/Policies

%files
/opt/alibaba/tdx-quote-verification-sample/relying_party.cpp
/opt/alibaba/tdx-quote-verification-sample/verifier.cpp
/opt/alibaba/tdx-quote-verification-sample/Makefile
/opt/alibaba/tdx-quote-verification-sample/README.md
/opt/alibaba/tdx-quote-verification-sample/README.zh.md
/opt/alibaba/tdx-quote-verification-sample/LICENSE
/opt/alibaba/tdx-quote-verification-sample/external
/opt/alibaba/tdx-quote-verification-sample/Policies/tenant_td_policy.json

%changelog
* Sun Mar 17 2024 Guorui.Yu <Guorui.Yu@linux.alibaba.com> - 1.0-1
- Initial package
