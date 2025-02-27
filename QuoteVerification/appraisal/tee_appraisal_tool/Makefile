#
# Copyright (C) 2011-2021 Intel Corporation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
#   * Neither the name of Intel Corporation nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#


DCAP_TOPDIR = ../../..
include $(DCAP_TOPDIR)/QuoteGeneration/buildenv.mk

PREBUILD_OPENSSL_PATH  = $(DCAP_TOPDIR)/prebuilt/openssl
FLAGS += -fpie -g -I./ \
		-I$(SGX_SDK)/include \
		-I$(DCAP_TOPDIR)/external/jwt-cpp/include \
		-I../../QVL/Src/ThirdParty/rapidjson/include \
		-I$(DCAP_TOPDIR)/QuoteGeneration/common/inc/internal/ \
		-I$(DCAP_TOPDIR)/QuoteGeneration/common/inc/internal/linux/ \
		-I$(PREBUILD_OPENSSL_PATH)/inc

ifdef DEBUG
FLAGS +=  -DSE_DEBUG_LEVEL=SE_TRACE_DEBUG
else
FLAGS += -DSE_DEBUG_LEVEL=SE_TRACE_ERROR
endif

CFLAGS += $(FLAGS)
CXXFLAGS += $(FLAGS)
Cpp_Files = $(wildcard *.cpp)
C_Obj_Files = se_trace.o

Obj_Files = $(Cpp_Files:.cpp=.o) $(C_Obj_Files)
LDFLAGS += -L$(PREBUILD_OPENSSL_PATH)/lib/linux64 -lcrypto -lpthread

TARGET_NAME = tee_appraisal_tool

.PHONY: all clean rebuild gen_key

all: $(TARGET_NAME) | $(BUILD_DIR)
	$(CP) $^ $|

$(BUILD_DIR):
	@$(MKDIR) $@

$(TARGET_NAME): $(Obj_Files)
	$(CXX) $(CXXFLAGS) -o $@ $(Obj_Files) $(LDFLAGS)

$(C_Obj_Files): %.o:$(DCAP_TOPDIR)/QuoteGeneration/common/src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

gen_key:
	@openssl ecparam -name secp384r1 -genkey -noout -out ec_private.pem
	@echo "The private key file named ec_private.pem is generated successfully for test purposes."


clean:
	$(RM)  $(TARGET_NAME) *.o $(BUILD_DIR)/$(TARGET_NAME)

rebuild:
	$(MAKE) clean
	$(MAKE) all
