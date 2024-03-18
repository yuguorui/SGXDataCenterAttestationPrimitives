
# TDX Quote 验证示例代码

## 概述
该文档将“远程认证（Remote Attestation）”一词定义如下：
一个实体 "认证者(Attester)" 向另一个远程实体 "依赖方(Relying Party)" 提供有关其身份和状态的证据，然后 依赖方(Relying Party) 评估 认证者(Attester) 的可信度，以用于依赖方自己的目的。关于远程证明流程和架构的更多详细信息，您可以参考[RFC 9334 - Remote ATtestation procedureS (RATS) Architecture](https://datatracker.ietf.org/doc/rfc9334/)。

具体在 TDX ECS 环境中，远程证明可以用于用于验证“平台”的可信度和在该“平台中运行的代码”的完整性和机密性。

在阿里云环境下，
* 平台指的是阿里云虚拟化软件栈；
* 平台中运行的代码指的是：运行在TDX ECS中的操作系统（如Alibaba Cloud Linux）和应用（如nginx、java等）

### 架构图
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

1. 认证者（Attester）创建证据（Evidence，在TDX ECS环境中即TDX Quote），该证据被传达给验证者（Verifier）。

2. 验证者（Verifier）使用证据（Evidence）以及来自背书者（Endorsers）的背书（Endorsements），通过证据评估策略（Evidence Appraisal Policy）来评估认证者（Attester）的可信度，并生成用于依赖方（Relying Parties）的认证结果（Attestation Results）。
  a. 其中背书者（Endorser）在本例中是 TDX 平台的厂商，即Intel。

3. 依赖方（Relying Party）通过应用其自己的评估策略（Appraisal Policy）来使用认证结果（Attestation Results），以作出特定于应用的决策，如授权决策。

本文主要覆盖了RATS架构中的 验证者（Verifier）和依赖方（Relying Party）相关逻辑。

## 先决条件

### 支持的操作系统
* Alibaba Cloud Linux 3 64位
* Anolis 8.6 64位

### 开发环境依赖
* yum install -y make gcc gcc-c++ openssl-devel git jq
* yum install -y tee-appraisal-tool libsgx-dcap-quote-verify-devel libsgx-dcap-default-qpl-devel
* 配置`/etc/sgx_default_qcnl.conf`文件中的`PCCS_URL`
  * 例如，通过以下命令将PCCS指向阿里云北京地域的DCAP服务：
  * `sed -i.$(date "+%m%d%y") 's|PCCS_URL=.*|PCCS_URL=https://sgx-dcap-server.cn-beijing.aliyuncs.com/sgx/certification/v4/|' /etc/sgx_default_qcnl.conf`

### 运行时环境依赖
* yum install -y libsgx-dcap-quote-verify libsgx-dcap-default-qpl openssl jq
* 配置`/etc/sgx_default_qcnl.conf`文件中的`PCCS_URL`
  * 例如，通过以下命令将PCCS指向阿里云北京地域的DCAP服务：
  * `sed -i.$(date "+%m%d%y") 's|PCCS_URL=.*|PCCS_URL=https://sgx-dcap-server.cn-beijing.aliyuncs.com/sgx/certification/v4/|' /etc/sgx_default_qcnl.conf`

## 构建

### 配置评估策略（Appraisal Policy）并签名
> 注意，此环境建议在已知的安全环境下进行，而非在生产环节中按需生成。

您可以使用json的形式描述您所需要的安全评估策略。
* 例如您可以配置以下安全策略以验证您的TDX ECS是否运行在加密且不可调试的状态，即安全的受保护模式。
* 您还可以在策略中配置额外参数以实现操作系统和应用的完整性校验。关于该评估策略（Appraisal Policy）的详细说明，请参考[Intel DCAP Appraisal Engine Developer Guide](https://download.01.org/intel-sgx/latest/dcap-latest/linux/docs/Intel_DCAP_Appraisal_Engine_Developer_Guide_for_Linux.pdf)
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

通过编写您的自定义策略或使用默认的策略（位于`Policies/tenant_td_policy.json`），您可以生成用于后续校验的策略令牌（Policy Token），该策略令牌可以被传输至任何需要验证远程证明报告的环境中以进行后续的校验流程。
```bash
make Policies/tenant_td_policy.jwt
```

## 校验远程证明报告（Quote）
```bash
# 基于`Policies/tenant_td_policy.json`中的策略，验证远程证明生成的quote
# 并将对应的验证结果、JSON Web Token (JWT)的形式输出到标准输出
./verify -quote <path_to_quote> 

# RelyingParty 可以进一步验证JWT签名的有效性
# 不指定quote的位置时，将默认使用../TDXQuoteGenerationSample/quote.dat作为默认值
./verifier |./relying_party -v |grep "json payload" |awk -F 'payload:' '{print $2}'|jq 
```

典型输出如下（部分字段被省略）：
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

为简单起见，您可以检查`appraisal_result`中的`overall_appraisal_result`字段以确认认证者是否满足您预先设置的评估策略（Appraisal Policy）。
```bash
./verifier |./relying_party -v |grep "json payload" |awk -F 'payload:' '{print $2}'|jq '.[0].result.overall_appraisal_result'
```

此外，在生成quote时所指定的`report_data`也会体现在远程证明的验证结果中，您可以使用该值进行密钥交换等业务逻辑的实现。
```bash
./verifier |./relying_party -v -a|grep "json payload" |awk -F 'payload:' '{print $2}'|jq '.. | .tdx_reportdata? | select(. != null)'
```
