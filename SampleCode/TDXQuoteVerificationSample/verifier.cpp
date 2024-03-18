#include <stdio.h>
#include <vector>
#include <string>
#include <assert.h>
#include <fstream>
#include <iterator>
// #include "sgx_ql_quote.h"
#include "sgx_dcap_quoteverify.h"
#include "sgx_dcap_qal.h"
#include "jwt-cpp/jwt.h"

#define DEFAULT_QUOTE   "../TDXQuoteGenerationSample/quote.dat"

using namespace std;

string tdx_policy = "Policies/tenant_td_policy.jwt";
string &g_tenant_policy = tdx_policy;

#define TDX_QUOTE_TYPE 0x81

int check_quote_type(uint8_t *quote)
{
    uint32_t *p_type = (uint32_t *) (quote + sizeof(uint16_t) * 2);

    if (*p_type != TDX_QUOTE_TYPE) {
        fprintf(stderr, "Error: Unsupported quote type.\n");
        //quote type is not supported
        return -1;
    }
    return 0;
}

#define PATHSIZE 0x418U

int ecdsa_quote_verification(vector<uint8_t> quote)
{
    quote3_error_t dcap_ret = SGX_QL_ERROR_UNEXPECTED;

    unsigned int jwt_token_size = 0;
    uint8_t *jwt_token = NULL;

    dcap_ret = tee_verify_quote_qvt(
                 quote.data(), (uint32_t)quote.size(),
                 NULL,
                 NULL,
                 NULL,
                 &jwt_token_size,
                 &jwt_token);

    if(dcap_ret !=  SGX_QL_SUCCESS){
        fprintf(stderr, "Error: tee_verify_quote_qvt failed: 0x%04x\n", dcap_ret);
        return -1;
    }

    // appraisal
    uint8_t *appraisal_result = NULL;
    uint32_t appraisal_result_buf_size = 0;

    ifstream file(g_tenant_policy, ios::binary);
    vector<uint8_t> tenant_policy((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    tenant_policy.push_back('\0');

    time_t current_time = time(NULL);
    uint8_t *p_qaps[] = {&tenant_policy[0]};

    dcap_ret = tee_appraise_verification_token((const uint8_t *)jwt_token, (uint8_t **)p_qaps, sizeof(p_qaps)/sizeof(p_qaps[0]),
                                               current_time, NULL, &appraisal_result_buf_size, &appraisal_result);
    if (dcap_ret != SGX_QL_SUCCESS) {
        fprintf(stderr, "\tError: tee_appraise_verification_token failed: 0x%04x\n", dcap_ret);
        return -1;
    }

    fprintf(stdout, "%s", appraisal_result);

    // free the quote verification result token
    if(jwt_token) {
        tee_free_verify_quote_qvt(jwt_token, &jwt_token_size);
    }

    return 0;
}

void usage()
{
    fprintf(stderr, "\nUsage:\n");
    fprintf(stderr, "\tPlease specify quote path, e.g. \"./app -quote <path/to/quote>\"\n");
    fprintf(stderr, "\tDefault quote path is %s when no command line args\n\n", DEFAULT_QUOTE);
}


// Application entry
int main(int argc, char *argv[])
{

    char quote_path[PATHSIZE] = { '\0' };

    //Just for sample use, better to change solid command line args solution in production env
    if (argc != 1 && argc != 3) {
        usage();
        return 0;
    }

    if (argv[1] && argv[2]) {
        if (!strcmp(argv[1], "-quote")) {
            strncpy(quote_path, argv[2], PATHSIZE - 1);
        }
    }

    if (*quote_path == '\0') {
        strncpy(quote_path, DEFAULT_QUOTE, PATHSIZE - 1);
    }

    //read quote from file
    //
    ifstream file(quote_path, ios::binary);
    vector<uint8_t> quote((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    if (quote.empty()) {
        usage();
        return -1;
    }

    // check quote type and specify the policy files
    if (0 != check_quote_type(quote.data())) {
        usage();
        return -1;
    }

    // Unrusted quote verification
    ecdsa_quote_verification(quote);

    return 0;
}
