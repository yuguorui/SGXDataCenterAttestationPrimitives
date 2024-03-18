APPRAISAL_TOOL := tee_appraisal_tool
EC_PRIVATE_KEY := ec_priv.pem

Verifier_Cpp_Files := verifier.cpp
Verifier_Cpp_Objects := $(Verifier_Cpp_Files:.cpp=.o)

RelyingParty_Cpp_Files := relying_party.cpp
RelyingParty_Cpp_Objects := $(RelyingParty_Cpp_Files:.cpp=.o)

App_Include_Paths := -Iinc -I./external/jwt-cpp/include

App_C_Flags := -std=c++11 -fPIC -Wno-attributes $(App_Include_Paths)

ifeq ($(DEBUG), 1)
        App_C_Flags += -DDEBUG -UNDEBUG -UEDEBUG -O0 -g3 -ggdb
else
        App_C_Flags += -DNDEBUG -UEDEBUG -UDEBUG -O2
endif

App_Cpp_Flags := $(App_C_Flags)
App_Link_Flags := -lsgx_dcap_quoteverify -ldcap_quoteprov -lssl -lcrypto -lpthread -ldl


Policy_Manifest_Files := $(wildcard Policies/*.json)
Policy_Tokens := $(sort $(Policy_Manifest_Files:.json=.jwt))

.PHONY: all target run
all: target

target: verifier relying_party $(Policy_Tokens)

run: all
	@$(CURDIR)/$(App_Name)
	@echo "RUN  =>  $(App_Name) [OK]"

external/jwt-cpp/include/jwt-cpp/jwt.h:
	test -e $@ || git submodule update --init --recursive

%.o: %.cpp external/jwt-cpp/include/jwt-cpp/jwt.h
	@$(CXX) $(App_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

verifier: $(Verifier_Cpp_Objects)
	@$(CXX) $^ -o $@ $(App_Link_Flags)
	@echo "LINK =>  $@"

relying_party: $(RelyingParty_Cpp_Objects)
	@$(CXX) $^ -o $@ $(App_Link_Flags)
	@echo "LINK =>  $@"

%.jwt: %.json
ifeq ($(wildcard $(EC_PRIVATE_KEY)),)
	openssl ecparam -name secp384r1 -genkey -out $(EC_PRIVATE_KEY)
endif
	$(APPRAISAL_TOOL) sign_policy -in $< -key $(EC_PRIVATE_KEY) -out $@ 

rpm:
	./dist/build.sh

.PHONY: rpm clean
clean:
	rm -f verifier relying_party $(Verifier_Cpp_Objects) $(RelyingParty_Cpp_Objects) $(Policy_Tokens) ./dist/*.rpm
