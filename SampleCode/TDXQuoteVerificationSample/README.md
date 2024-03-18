# TDX Quote Verification Example Code

## Overview
This document defines "Remote Attestation" as follows:
An entity "Attester" provides evidence of its identity and state to another remote entity "Relying Party", who then evaluates the trustworthiness of the Attester for the Relying Party's own purposes. For more detailed information about the remote attestation process and architecture, you can refer to [RFC 9334 - Remote ATtestation procedureS (RATS) Architecture](https://datatracker.ietf.org/doc/rfc9334/).

Specifically, in the TDX ECS environment, remote attestation can be used to verify the trustworthiness of the "platform" and the integrity and confidentiality of the code running on that "platform".

In the Alibaba Cloud environment,
* The platform refers to the Alibaba Cloud virtualization software stack;
* The code running on the platform refers to the operating system (such as Alibaba Cloud Linux) and applications (such as nginx, java, etc.) running in TDX ECS.

### Architecture Diagram
```
           ************   ************    *****************
           * Endorser *   * Verifier *    * Relying Party *
           ************   *  Owner   *    *  Owner        *
                 |        ************    *****************
                 |              |                 |
     Endorsements|              |                 |
                 |              |Appraisal        |
                 |              |Policy for       |
                 |              |Evidence         | Appraisal  
                 |              |                 | Policy for 
                 |              |                 | Attestation
                 |              |                 |  Result    
                 v              v                 |
               .-----------------.                |
        .----->|     Verifier    |------.         |
        |      '-----------------'      |         |
        |                               |         |
        |                    Attestation|         |
        |                    Results    |         |
        | Evidence(TDX Quote)           |         |
        |                               |         |
        |                               v         v
  .----------.                      .-----------------.
  | Attester |                      | Relying Party   |
  '----------'                      '-----------------'
```

1. The Attester creates evidence (Evidence, in the TDX ECS environment, this is the TDX Quote) that is conveyed to the Verifier.

2. The Verifier uses the evidence (Evidence) along with endorsements from the Endorsers, through an evidence appraisal policy, to evaluate the trustworthiness of the Attester and generates attestation results for the Relying Parties.
   a. In this case, the Endorser is the manufacturer of the TDX platform, i.e., Intel.

3. The Relying Party uses the attestation results (Attestation Results) by applying its own appraisal policy to make application-specific decisions, such as authorization decisions.

This document primarily covers the logic related to the Verifier and Relying Party in the RATS architecture.

## Prerequisites

### Supported Operating Systems
* Alibaba Cloud Linux 3 64-bit
* Anolis 8.6 64-bit

### Development Environment Dependencies
* yum install -y make gcc gcc-c++ openssl-devel git jq
* yum install -y tee-appraisal-tool libsgx-dcap-quote-verify-devel libsgx-dcap-default-qpl-devel
* Configure the `PCCS_URL` in the `/etc/sgx_default_qcnl.conf` file
  * For example, to point PCCS to the Alibaba Cloud Beijing region's DCAP service:
  * `sed -i.$(date "+%m%d%y") 's|PCCS_URL=.*|PCCS_URL=https://sgx-dcap-server.cn-beijing.aliyuncs.com/sgx/certification/v4/|' /etc/sgx_default_qcnl.conf`

### Runtime Environment Dependencies
* yum install -y libsgx-dcap-quote-verify libsgx-dcap-default-qpl openssl jq
* Configure the `PCCS_URL` in the `/etc/sgx_default_qcnl.conf` file
  * For example, to point PCCS to the Alibaba Cloud Beijing region's DCAP service:
  * `sed -i.$(date "+%m%d%y") 's|PCCS_URL=.*|PCCS_URL=https://sgx-dcap-server.cn-beijing.aliyuncs.com/sgx/certification/v4/|' /etc/sgx_default_qcnl.conf`

## Build

### Configure Appraisal Policy and Sign
> Note, it's recommended to conduct this in a known secure environment, not on-demand in production.

You can describe your required security appraisal policy in json format.
* For example, you can configure the following security policy to verify if your TDX ECS is running in an encrypted and non-debuggable state, i.e.,

 secure protected mode.
* You can also configure additional parameters in the policy for integrity checks of the operating system and applications. For detailed explanations of the Appraisal Policy, refer to [Intel DCAP Appraisal Engine Developer Guide](https://download.01.org/intel-sgx/latest/dcap-latest/linux/docs/Intel_DCAP_Appraisal_Engine_Developer_Guide_for_Linux.pdf)
```json
{
    "policy_array":[
        {
            "environment":{
                "class_id":"45b734fc-aa4e-4c3d-ad28-e43d08880e68",
                "description":"Application TD TCB 1.5"
            },
            "reference":{
                "tdx_attributes":"0000000010000000",
                "#NOTE": "0000000010000000 means for NO_DEBUG and SPTE_VE_DISABLE"
            }
        }
    ]
}
```

By writing your custom policy or using the default one (located at `Policies/tenant_td_policy.json`), you can generate a Policy Token for subsequent validation, which can be transmitted to any environment requiring remote attestation report verification for further validation processes.
```bash
make Policies/tenant_td_policy.jwt
```

### Build other components
```bash
make all
```

## Verify Remote Attestation Report (Quote)
```bash
# Based on the policy in `Policies/tenant_td_policy.json`, verify the remotely attested quote
# and output the corresponding verification result in the form of JSON Web Token (JWT) to stdout
./verify -quote <path_to_quote> 

# Relying Party can further verify the validity of the JWT signature
# When the quote location is not specified, it defaults to ../TDXQuoteGenerationSample/quote.dat
./verifier |./relying_party -v |grep "json payload" |awk -F 'payload:' '{print $2}'|jq 
```

Typical output (with some fields omitted):
```
[
  {
    "result": {
      "appraisal_check_date": 1710400829000000000,
      "nonce": 502551065253582,
      "certification_data": [
        {
          "certification_data": {
            "qe_identity_issuer_chain": "LS0t...",
            "root_ca_crl": "MzA4...",
            "pck_crl": "LS0t...",
            "pck_crl_issuer_chain": "LS0t...",
            "tcb_info": "eyJ0...",
            "qe_identity": "eyJl...",
            "tcb_info_issuer_chain": "LS0t..."
          }
        }
      ],
      "overall_appraisal_result": 1,
      "appraised_reports": [
        {
          "appraisal_result": 1,
          "detailed_result": [
            {
              "td_mrownerconfig_check": true,
              "td_xfam_check": true,
              "td_mrservicetd_check": true,
              "td_attributes_check": true,
              "td_rtmr3_check": true,
              "td_mrtd_check": true,
              "td_mrowner_check": true,
              "td_rtmr0_check": true,
              "td_mrconfigid_check": true,
              "td_rtmr1_check": true,
              "td_rtmr2_check": true
            }
          ],
          "policy": {
            "environment": {
              "description": "Application TD TCB 1.5",
              "class_id": "45b734fc-aa4e-4c3d-ad28-e43d08880e68"
            },
            "signature": "-6C0-...",
            "reference": {
              ...
            },
            "signing_key": {
              "kty": "EC",
              "crv": "P-384",
              "alg": "ES384",
              "y": "CeW8...",
              "x": "NmSa..."
            }
          },
          "report": {
            "environment": {
              "Description": "Application TD TCB",
              "class_id": "45b734fc-aa4e-4c3d-ad28-e43d08880e68"
            },
            "measurement": {
              "tdx_mrownerconfig": "0000...",
              "tdx_mrservicetd": "3D03...",
              "tdx_xfam": "00000000000642E7",
              "tdx_mrtd": "0A40...",
              "tdx_mrowner": "0000...",
              "tdx_attributes": "0000000010000000",
              "tdx_mrconfigid": "0000...",
              "tdx_reportdata": "D98B...",
              "tdx_rtmr3": "0000...",
              "tdx_rtmr2": "0000...",
              "tdx_rtmr1": "6368...",
              "tdx_rtmr0": "D0FD..."
            }
          }
        },
        {
          "appraisal_result": 1,
          "detailed_result": [
            {
              "platform_provider_id_check": true,
              "sgx_types_check": true,
              "cached_keys_check": true,
              "smt_enabled_check": true,
              "accepted_tcb_level_date_tag_check": true,
              "advisory_ids_check": true,
              "expiration_date_check": true,
              "tcb_eval_num_check": true,
              "earliest_accepted_tcb_level_date_tag_check": true,
              "tcb_status_check": true,
              "dynamic_platform_check": true
            }
          ],
          "policy": {
            "environment": {
              "description": "Alibaba Cloud Evaluation Num Policy for TDX Platform",
              "class_id": "f708b97f-0fb2-4e6b-8b03-8a5bcd1221d3"
            },
            "signature": "l00p...",
            "reference": {
              "accepted_tcb_status": [
                "UpToDate"
              ],
              "allow_dynamic_plaform": true,
              "min_eval_num": 16
            },
            "signing_key": {
              "kty": "EC",
              "crv": "P-384",
              "alg": "ES384",
              "y": "7hlr...",
              "x": "OSbD..."
            }
          },
          "report": {
            "environment": {
              "description": "TDX Platform TCB",
              "class_id": "f708b97f-0fb2-4e6b-8b03-8a5bcd1221d3"
            },
            "measurement": {
              "earliest_issue_date": "2018-05-21T10:45:10Z",
              "is_cached_keys_policy": true,
              "fmspc": "90C06F000000",
              "is_smt_enabled": true,
              "earliest_expiration_date": "2024-04-02T10:22:51Z",
              "root_ca_crl_num": 1,
              "root_key_id": "9309...",
              "pck_crl_num": 1,
              "tcb_eval_num": 16,
              "latest_issue_date": "2024-03-13T15:45:02Z",
              "tcb_status": [
                "UpToDate"
              ],
              "tcb_level_date_tag": "2023-08-09T00:00:00Z",
              "is_dynamic_platform": true,
              "sgx_types": 1
            }
          }
        },
        {
          "appraisal_result": 1,
          "detailed_result": [
            {
              "td_qe_expiration_date_check": true,
              "td_qe_tcb_eval_num_check": true,
              "td_qe_earliest_accepted_tcb_level_date_tag_check": true,
              "td_qe_tcb_status_check": true,
              "td_qe_accepted_tcb_level_date_tag_check": true
            }
          ],
          "policy": {
            "environment": {
              "description": "Alibaba Cloud Num Policy for Verified TDQE",
              "class_id": "3769258c-75e6-4bc7-8d72-d2b0e224cad2"
            },
            "signature": "l00p...",
            "reference": {
              "accepted_tcb_status": [
                "UpToDate"
              ],
              "min_eval_num": 16
            },
            "signing_key": {
              "kty": "EC",
              "crv": "P-384",
              "alg": "ES384",
              "y": "7hlr...",
              "x": "OSbD..."
            }
          },
          "report": {
            "environment": {
              "Description": "RAW TDX QE Report",
              "class_id": "3769258c-75e6-4bc7-8d72-d2b0e224cad2"
            },
            "measurement": {
              "earliest_expiration_date": "2025-05-21T10:50:10Z",
              "earliest_issue_date": "2018-05-21T10:45:10Z",
              "root_key_id": "9309...",
              "tcb_eval_num": 16,
              "latest_issue_date": "2018-05-21T10:50:10Z",
              "tcb_status": [
                "UpToDate"
              ],
              "tcb_level_date_tag": "2023-08-09T00:00:00Z"
            }
          }
        }
      ]
    }
  }
]
```

For simplicity, you may check the `overall_appraisal_result` field within `appraisal_result` to confirm if the attester meets your pre-set Appraisal Policy.
```bash
./verifier |./relying_party -v |grep "json payload" |awk -F 'payload:' '{print $2}'|jq '.[0].result.overall_appraisal_result'
```

Furthermore, the `report_data` specified during quote generation will also be reflected in the remote attestation verification result, which you can use for implementing business logics such as key exchange.
```bash
./verifier |./relying_party -v -a|grep "json payload" |awk -F 'payload:' '{print $2}'|jq '.. | .tdx_reportdata? | select(. != null)'
```
