# TDX Quote 生成示例代码

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

本文主要覆盖了RATS架构中的 认证者（Attester）创建证据（Quote）的流程。

## 先决条件

### 支持的操作系统
* Alibaba Cloud Linux 3 64位
* Anolis 8.6 64位

### 开发环境设置
> 相关依赖仅需要在开发环境中安装，不需要在生产环境中安装。

* yum install -y libtdx-attest-devel gcc make

### 生产环境设置
> 生产环境中仅需要以下依赖。

* yum install -y libtdx-attest

## 构建

```bash
make
```

## 运行

```bash
# 随机生成report_data并签发quote
./main 

# 指定的report_data并签发quote
./main -d <report_data_in_hex>
# 例如:
# ./main -d 1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef
# ./main -d $(cat data.dat | xxd -p)
```

其中：
* `<report_data_in_hex>`是一个16进制字符串，长度为64字节。 
* 您可以在`report_data_in_hex`中指定自定义数据，该数据将被包含在生成的quote中。
* `report_data_in_hex` 因其长度有限，在实践中通常是一个哈希值。例如是一个公钥的哈希，并可以用于后续的密钥协商流程。

