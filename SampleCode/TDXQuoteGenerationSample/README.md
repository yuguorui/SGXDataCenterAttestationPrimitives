# TDX Quote Generation Example Code

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

This document primarily covers the process of the Attester creating evidence (Quote) in the RATS architecture.

## Prerequisites

### Supported Operating Systems
* Alibaba Cloud Linux 3 64-bit
* Anolis 8.6 64-bit

### Development Environment Setup
> The following dependencies only need to be installed in the development environment, not in the production environment.

* yum install -y libtdx-attest-devel gcc make

### Production Environment Setup
> Only the following dependencies are required in the production environment.

* yum install -y libtdx-attest

## Build

```bash
make
```

## Run

```bash
# Generate report_data randomly and issue a quote
./main 

# Issue a quote with specified report_data
./main -d <report_data_in_hex>
# For example:
# ./main -d 1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef
# ./main -d $(cat data.dat | xxd -p)
```

Where:
* `<report_data_in_hex>` is a 64-byte long hexadecimal string.
* You can specify custom data in `report_data_in_hex`, which will be included in the generated quote.
* Due to its length, `report_data_in_hex` is usually a hash value in practice, such as a public key hash, and can be used for subsequent key negotiation processes.
