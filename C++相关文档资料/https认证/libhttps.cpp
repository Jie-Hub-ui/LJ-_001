/********************************
https证书信息获取实现
libcurl官网：https://curl.se/libcurl/c/
openSSL：
		https://blog.csdn.net/yyfzy/article/details/46798965?utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromMachineLearnPai2%7Edefault-1.control&dist_request_id=1329187.9212.16178498980521517&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromMachineLearnPai2%7Edefault-1.control
需要安装libcurl和openssl库
*********************************/



#include <openssl/err.h>
#include <openssl/ssl.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>  
#include <string> 
#include <iostream>
#include <abprec.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/asn1.h>
#include <aesencrypt.hpp>

using namespace std;


#define AES_KEY _T("984F90287AB7D93E9726372DD37980CB")


void hex_encode(unsigned char* readbuf, void *writebuf, size_t len)
{
    for(size_t i=0; i < len; i++) {
        char *l = (char*) (2*i + ((intptr_t) writebuf));
        sprintf(l, "%02x", readbuf[i]);
    }
}

int main(int argc, char *argv[])
{
    CURL *curl;
    CURLcode res;
 
    // curl初始化
    curl_global_init(CURL_GLOBAL_DEFAULT);
 
    curl = curl_easy_init();
    if ( curl ) {
        // 设置 curl 访问的 url
        curl_easy_setopt(curl, CURLOPT_URL, "https://10.4.33.212:443/ddd");
        // 设置是否验证对方证书，1 验证， 0 不验证
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        // 设置是否验证服务器证书是否适用于已知的服务器， 0 不验证， 2 表明该服务器是需要连接的服务器，否则连接失败
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        // 设置提取SSL证书信息，0 不提取， 1 提取证书信息
        curl_easy_setopt(curl, CURLOPT_CERTINFO, 1L);
        // 阻塞的方式执行整个请求
        res = curl_easy_perform(curl);

        if (!res) {
            struct curl_certinfo *ci;
            // 从 curl 句柄中提取信息
            // CURLINFO_CERTINFO 表示提取证书链信息，得到一个有关证书链信息的结构体
            res = curl_easy_getinfo(curl, CURLINFO_CERTINFO, &ci);
 
            if (!res) {
                // ci->num_of_certs 表示获取证书链个数(证书个数)
                printf("\n%d certs!\n", ci->num_of_certs);
                for(int i = 0; i < ci->num_of_certs; i++) {
                    // 证书信息结构体，证书信息中每一个字段信息用结构体表示(结构体链表)
                    struct curl_slist *slist;
                    // 循环获取证书信息中每一个字段信息
                    for(slist = ci->certinfo[i]; slist; slist = slist->next) {
                        string certItem = slist->data;
                        size_t pos = certItem.find(":");
                        cout << slist->data << endl;
                        // 获取Cert字段证书信息码(采用SSL解析证书)
                        if(pos != string::npos && certItem.substr(0, pos) == "Cert"){
                            string c = certItem.substr(pos + 1);
                            cout << "c = " << c << endl;
                            char tmpCert[4096] = {0};
                            strncpy(tmpCert, c.c_str(), c.length());
                            OpenSSL_add_all_algorithms();
                            BIO* bio = NULL;
                            X509* x_cert = NULL;

                            bio = BIO_new(BIO_s_mem());
                            int bioRes = BIO_write(bio, tmpCert, strlen(tmpCert));
                            x_cert = PEM_read_bio_X509(bio, &x_cert, 0, NULL);

                            // 证书版本
                            long version = X509_get_version(x_cert);
                      
                            // 证书使用者
                            char commonName[256] = "";
                            X509_NAME* subjectName = X509_get_subject_name(x_cert);
                            int cnRes = X509_NAME_get_text_by_NID(subjectName, NID_commonName, commonName, sizeof(commonName));
                      
                            // 证书颁发者
                            X509_NAME *issuer = X509_get_issuer_name(x_cert);
                            char * issuerEx = X509_NAME_oneline(issuer, NULL, 0);

                            // 证书起始时间和证书结束时间
                            ASN1_TIME* effeciveTime = X509_get_notBefore(x_cert);
                            ASN1_TIME* expiredTime = X509_get_notAfter(x_cert);
                            char effeciveTimeStr[128], expiredTimeStr[128];
                            int rc = 0;
                            BIO *b = BIO_new(BIO_s_mem());
                            rc = ASN1_TIME_print(b, effeciveTime);
                            rc = BIO_gets(b, effeciveTimeStr, 128);

                            int expireTimeRes = X509_cmp_current_time(expiredTime);
                            rc = ASN1_TIME_print(b, effeciveTime);
                            rc = BIO_gets(b, expiredTimeStr, 128);
                            BIO_free(b);

                            // 证书序列号
                            char *serial = NULL;
                            ASN1_INTEGER *asn1_i = X509_get_serialNumber(x_cert);
                            BIGNUM  *num = ASN1_INTEGER_to_BN(asn1_i, NULL);
                            serial = BN_bn2hex(num);
                            BN_free(num);

                            // 创建证书指纹
                            char fingerprint[128];
                            const EVP_MD* digest = EVP_sha1();
                            unsigned sha1Len = 0;
                            int sha1Res = X509_digest(x_cert, digest, (unsigned char*)fingerprint, &sha1Len);
                            char fingerprintBuf[2*128+1];
                            hex_encode((unsigned char*)fingerprint, fingerprintBuf, 128);
                            string encodeFingerPrint = AES_encode_str(fingerprint, (uint8_t*)AES_KEY);    // ab加密算法
                      
                            // 公钥算法
                            EVP_PKEY *pk = NULL;
                            pk = X509_get_pubkey(x_cert);
                      

                            cout << "版本：" << version << endl;
                            cout << "使用者：" <<  commonName << endl;
                            cout << "颁发者：" << issuerEx << endl;
                            cout << "起始时间：" << effeciveTimeStr << endl;
                            cout << "结束时间：" << expiredTimeStr << endl;
                            cout << "序列号：" << serial << endl;
                            cout << "证书指纹：" << encodeFingerPrint << endl;
                            cout << "证书算法：" << pk->type << endl;
                            OPENSSL_free(issuer);
                            OPENSSL_free(subjectName);
                            BIO_free(bio);
                        }
                    }
                }
            }
        }
    }

    curl_global_cleanup();
 
    return 0;
}


/*********************执行输出**********************
{
    "error": null,
    "status": "success",
    "responseData": {
        "id": "cd6fa8258b9c11eb8b8500505682239c",
        "p_group_id": "82e99e15780411ebb15900505682239c",
        "name": "ddd",
        "type": 6,
        "ip": "fe80::d831:cd37:a82d:b6b8%2512",
        "port": 0,
        "version": "Microsoft Windows Server 2019",
        "auth_user": "jyj",
        "deleted": 0,
        "customer": "",
        "create_time": 1616479114273,
        "update_time": 1616479114273,
        "entry_point": "",
        "create_user": "jyj"
    }
}
1 certs!
Subject:C = CN, ST = Shanghai, L = Shanghai, O = Eden, OU = eden, CN = 127.0.0.1, emailAddress = jiang.yijun@aishu.cn
Issuer:C = CN, ST = Shanghai, L = Shanghai, O = aishu.cn, OU = aishu.cn, emailAddress = jiang.yijun@aishu.cn
Version:2
Serial Number:1000
Signature Algorithm:sha256WithRSAEncryption
Public Key Algorithm:rsaEncryption
X509v3 Basic Constraints:CA:FALSE
Netscape Cert Type:SSL Server
Netscape Comment:OpenSSL Generated Server Certificate
X509v3 Subject Key Identifier:F3:C5:D0:B9:02:59:D5:DC:3C:68:0B:6F:14:16:20:BD:32:62:4D:2F
X509v3 Authority Key Identifier:keyid:9E:FB:50:7E:17:16:EA:CD:76:B3:7E:FD:F5:AA:FA:4F:CE:92:3C:EA
DirName:/C=CN/ST=Shanghai/L=Shanghai/O=aishu.cn/OU=aishu.cn/emailAddress=jiang.yijun@aishu.cn
serial:B9:50:6D:EB:40:B9:51:16

X509v3 Key Usage:Digital Signature, Key Encipherment
X509v3 Extended Key Usage:TLS Web Server Authentication
X509v3 Subject Alternative Name:IP Address:127.0.0.1, IP Address:192.168.1.1, DNS:eden.io, DNS:ioptimi.eden.io
Start date:Mar 23 09:50:08 2021 GMT
Expire date:Apr  2 09:50:08 2022 GMT
RSA Public Key:2048
rsa(n):B2B4195485F0093E9CDE298AFEF692E87749C5D7250CB99969536F9BCC5BF07BC14A104043C9EFDC6E5E3A10AFCE07F4698E270EE888F86B323F544C948188848D1AEBB9A306B418ADC46AE38FEF3CA001FE8622C9548166BEF49D02289D462042A03674C2AE5D8011DF24C35827F1B971D11F96D1053AA47BE16D9AA69FFD08678A4C498E9DA2E3C0C81A4EB5C042E7D7E29DFF943CC2266A97F2BC087C93FBBB2914517C9D741F4C24B4F99374FF06A8F9E4535D28BC0F580152D2B72835E9D50ADF49B0EA1E47D1AE194C6DF9BC2D9755B78641B4C140D9BE28A4D7E1E3C33A66E56638B083E93ADCFD6F609C22398F6422DDE8817ECE5E89FFBD3B3CFD55
rsa(e):10001
Signature:12:61:8b:20:9f:14:d0:ba:e9:27:d7:49:82:30:2b:f4:60:34:7b:5c:4c:e9:4c:d3:87:18:58:4c:d7:7f:40:84:73:65:6f:f2:d0:08:b2:80:2a:4f:49:12:98:da:d7:e3:bd:54:f9:bd:64:b0:9d:4e:6b:e7:a3:00:46:2a:fb:a8:cd:69:b4:8b:4e:89:22:a7:3d:37:c6:29:d3:93:92:25:d4:8b:96:5c:5a:69:11:5e:fb:d9:dc:77:27:82:8b:18:af:b4:cc:75:0a:88:59:2f:0b:9a:b0:27:98:b0:bb:8a:69:35:53:05:b3:c0:96:48:42:a0:91:19:5b:cb:56:e1:b5:64:d4:86:a6:20:38:7d:5c:ad:49:b7:67:27:d0:d6:c4:c2:c3:75:13:04:9f:62:5a:ea:ae:e1:f7:0d:97:98:03:9e:d3:55:b2:cf:44:53:b7:75:4f:62:f1:05:09:b8:0c:c3:ae:92:cf:8c:06:d1:66:d7:a3:09:6d:99:94:e7:7b:80:be:08:10:be:b4:75:fc:df:4c:20:3f:e7:98:cb:01:50:9d:ce:85:61:bc:76:6a:f4:db:59:11:e9:4c:2c:11:8b:fd:61:39:b5:33:d7:96:d3:41:9b:09:de:53:19:83:64:ec:96:ab:9f:ed:f6:10:8e:d2:a9:30:f9:74:6f:59:ac:4d:26:00:43:bb:d5:ba:3e:2d:a7:fe:ff:38:bd:8f:3e:3c:d6:39:3d:1b:cb:a8:32:eb:e4:27:97:de:e7:5d:c2:c5:bf:ce:76:b2:08:a8:24:e4:26:af:78:6a:84:a8:7d:1b:ed:70:a0:63:69:c9:4c:94:05:de:99:6a:3d:30:a8:0c:85:aa:29:ac:82:54:f2:94:55:a2:3f:d5:af:ee:76:dd:31:21:1b:a6:24:31:68:f9:32:ac:76:3e:5f:11:2a:71:83:f2:89:37:b9:8d:eb:60:8d:24:07:07:ff:41:9a:d6:27:8e:09:41:4b:a5:38:31:f6:f4:1b:6a:2a:08:79:b2:18:dc:1f:e7:69:35:0f:e7:da:55:29:85:95:d6:8f:cd:5a:c4:27:76:a6:27:cc:07:b8:c5:22:00:5e:84:10:5e:de:9b:bf:0b:a5:d0:28:a9:3f:ae:37:b9:77:f9:d8:95:87:82:4b:23:0e:c0:07:f3:5d:b2:fd:cb:ba:61:47:bd:d1:14:9f:f1:d8:67:a8:8f:02:c8:2f:2b:7c:a9:f7:ff:0c:f6:96:b1:79:4e:89:81:35:bd:3b:75:97:e8:49:35:94:38:7c:09:6b:2c:28:76:d6:d6:21:9a:9f:c0:d7:07:64:c6:c4:9a:cf:90:b7:6c:6e:79:5b:40:b4:
Cert:-----BEGIN CERTIFICATE-----
MIIGCDCCA/CgAwIBAgICEAAwDQYJKoZIhvcNAQELBQAwfjELMAkGA1UEBhMCQ04x
ETAPBgNVBAgMCFNoYW5naGFpMREwDwYDVQQHDAhTaGFuZ2hhaTERMA8GA1UECgwI
YWlzaHUuY24xETAPBgNVBAsMCGFpc2h1LmNuMSMwIQYJKoZIhvcNAQkBFhRqaWFu
Zy55aWp1bkBhaXNodS5jbjAeFw0yMTAzMjMwOTUwMDhaFw0yMjA0MDIwOTUwMDha
MIGKMQswCQYDVQQGEwJDTjERMA8GA1UECAwIU2hhbmdoYWkxETAPBgNVBAcMCFNo
YW5naGFpMQ0wCwYDVQQKDARFZGVuMQ0wCwYDVQQLDARlZGVuMRIwEAYDVQQDDAkx
MjcuMC4wLjExIzAhBgkqhkiG9w0BCQEWFGppYW5nLnlpanVuQGFpc2h1LmNuMIIB
IjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAsrQZVIXwCT6c3imK/vaS6HdJ
xdclDLmZaVNvm8xb8HvBShBAQ8nv3G5eOhCvzgf0aY4nDuiI+GsyP1RMlIGIhI0a
67mjBrQYrcRq44/vPKAB/oYiyVSBZr70nQIonUYgQqA2dMKuXYAR3yTDWCfxuXHR
H5bRBTqke+Ftmqaf/QhnikxJjp2i48DIGk61wELn1+Kd/5Q8wiZql/K8CHyT+7sp
FFF8nXQfTCS0+ZN0/wao+eRTXSi8D1gBUtK3KDXp1QrfSbDqHkfRrhlMbfm8LZdV
t4ZBtMFA2b4opNfh48M6ZuVmOLCD6Trc/W9gnCI5j2Qi3eiBfs5eif+9Ozz9VQID
AQABo4IBgTCCAX0wCQYDVR0TBAIwADARBglghkgBhvhCAQEEBAMCBkAwMwYJYIZI
AYb4QgENBCYWJE9wZW5TU0wgR2VuZXJhdGVkIFNlcnZlciBDZXJ0aWZpY2F0ZTAd
BgNVHQ4EFgQU88XQuQJZ1dw8aAtvFBYgvTJiTS8wgbIGA1UdIwSBqjCBp4AUnvtQ
fhcW6s12s3799ar6T86SPOqhgYOkgYAwfjELMAkGA1UEBhMCQ04xETAPBgNVBAgM
CFNoYW5naGFpMREwDwYDVQQHDAhTaGFuZ2hhaTERMA8GA1UECgwIYWlzaHUuY24x
ETAPBgNVBAsMCGFpc2h1LmNuMSMwIQYJKoZIhvcNAQkBFhRqaWFuZy55aWp1bkBh
aXNodS5jboIJALlQbetAuVEWMA4GA1UdDwEB/wQEAwIFoDATBgNVHSUEDDAKBggr
BgEFBQcDATAvBgNVHREEKDAmhwR/AAABhwTAqAEBggdlZGVuLmlvgg9pb3B0aW1p
LmVkZW4uaW8wDQYJKoZIhvcNAQELBQADggIBABJhiyCfFNC66SfXSYIwK/RgNHtc
TOlM04cYWEzXf0CEc2Vv8tAIsoAqT0kSmNrX471U+b1ksJ1Oa+ejAEYq+6jNabSL
Tokipz03xinTk5Il1IuWXFppEV772dx3J4KLGK+0zHUKiFkvC5qwJ5iwu4ppNVMF
s8CWSEKgkRlby1bhtWTUhqYgOH1crUm3ZyfQ1sTCw3UTBJ9iWuqu4fcNl5gDntNV
ss9EU7d1T2LxBQm4DMOuks+MBtFm16MJbZmU53uAvggQvrR1/N9MID/nmMsBUJ3O
hWG8dmr021kR6UwsEYv9YTm1M9eW00GbCd5TGYNk7Jarn+32EI7SqTD5dG9ZrE0m
AEO71bo+Laf+/zi9jz481jk9G8uoMuvkJ5fe513Cxb/OdrIIqCTkJq94aoSofRvt
cKBjaclMlAXemWo9MKgMhaoprIJU8pRVoj/Vr+523TEhG6YkMWj5Mqx2Pl8RKnGD
8ok3uY3rYI0kBwf/QZrWJ44JQUulODH29BtqKgh5shjcH+dpNQ/n2lUphZXWj81a
xCd2pifMB7jFIgBehBBe3pu/C6XQKKk/rje5d/nYlYeCSyMOwAfzXbL9y7phR73R
FJ/x2GeojwLILyt8qff/DPaWsXlOiYE1vTt1l+hJNZQ4fAlrLCh21tYhmp/A1wdk
xsSaz5C3bG55W0C0
-----END CERTIFICATE-----

c = -----BEGIN CERTIFICATE-----
MIIGCDCCA/CgAwIBAgICEAAwDQYJKoZIhvcNAQELBQAwfjELMAkGA1UEBhMCQ04x
ETAPBgNVBAgMCFNoYW5naGFpMREwDwYDVQQHDAhTaGFuZ2hhaTERMA8GA1UECgwI
YWlzaHUuY24xETAPBgNVBAsMCGFpc2h1LmNuMSMwIQYJKoZIhvcNAQkBFhRqaWFu
Zy55aWp1bkBhaXNodS5jbjAeFw0yMTAzMjMwOTUwMDhaFw0yMjA0MDIwOTUwMDha
MIGKMQswCQYDVQQGEwJDTjERMA8GA1UECAwIU2hhbmdoYWkxETAPBgNVBAcMCFNo
YW5naGFpMQ0wCwYDVQQKDARFZGVuMQ0wCwYDVQQLDARlZGVuMRIwEAYDVQQDDAkx
MjcuMC4wLjExIzAhBgkqhkiG9w0BCQEWFGppYW5nLnlpanVuQGFpc2h1LmNuMIIB
IjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAsrQZVIXwCT6c3imK/vaS6HdJ
xdclDLmZaVNvm8xb8HvBShBAQ8nv3G5eOhCvzgf0aY4nDuiI+GsyP1RMlIGIhI0a
67mjBrQYrcRq44/vPKAB/oYiyVSBZr70nQIonUYgQqA2dMKuXYAR3yTDWCfxuXHR
H5bRBTqke+Ftmqaf/QhnikxJjp2i48DIGk61wELn1+Kd/5Q8wiZql/K8CHyT+7sp
FFF8nXQfTCS0+ZN0/wao+eRTXSi8D1gBUtK3KDXp1QrfSbDqHkfRrhlMbfm8LZdV
t4ZBtMFA2b4opNfh48M6ZuVmOLCD6Trc/W9gnCI5j2Qi3eiBfs5eif+9Ozz9VQID
AQABo4IBgTCCAX0wCQYDVR0TBAIwADARBglghkgBhvhCAQEEBAMCBkAwMwYJYIZI
AYb4QgENBCYWJE9wZW5TU0wgR2VuZXJhdGVkIFNlcnZlciBDZXJ0aWZpY2F0ZTAd
BgNVHQ4EFgQU88XQuQJZ1dw8aAtvFBYgvTJiTS8wgbIGA1UdIwSBqjCBp4AUnvtQ
fhcW6s12s3799ar6T86SPOqhgYOkgYAwfjELMAkGA1UEBhMCQ04xETAPBgNVBAgM
CFNoYW5naGFpMREwDwYDVQQHDAhTaGFuZ2hhaTERMA8GA1UECgwIYWlzaHUuY24x
ETAPBgNVBAsMCGFpc2h1LmNuMSMwIQYJKoZIhvcNAQkBFhRqaWFuZy55aWp1bkBh
aXNodS5jboIJALlQbetAuVEWMA4GA1UdDwEB/wQEAwIFoDATBgNVHSUEDDAKBggr
BgEFBQcDATAvBgNVHREEKDAmhwR/AAABhwTAqAEBggdlZGVuLmlvgg9pb3B0aW1p
LmVkZW4uaW8wDQYJKoZIhvcNAQELBQADggIBABJhiyCfFNC66SfXSYIwK/RgNHtc
TOlM04cYWEzXf0CEc2Vv8tAIsoAqT0kSmNrX471U+b1ksJ1Oa+ejAEYq+6jNabSL
Tokipz03xinTk5Il1IuWXFppEV772dx3J4KLGK+0zHUKiFkvC5qwJ5iwu4ppNVMF
s8CWSEKgkRlby1bhtWTUhqYgOH1crUm3ZyfQ1sTCw3UTBJ9iWuqu4fcNl5gDntNV
ss9EU7d1T2LxBQm4DMOuks+MBtFm16MJbZmU53uAvggQvrR1/N9MID/nmMsBUJ3O
hWG8dmr021kR6UwsEYv9YTm1M9eW00GbCd5TGYNk7Jarn+32EI7SqTD5dG9ZrE0m
AEO71bo+Laf+/zi9jz481jk9G8uoMuvkJ5fe513Cxb/OdrIIqCTkJq94aoSofRvt
cKBjaclMlAXemWo9MKgMhaoprIJU8pRVoj/Vr+523TEhG6YkMWj5Mqx2Pl8RKnGD
8ok3uY3rYI0kBwf/QZrWJ44JQUulODH29BtqKgh5shjcH+dpNQ/n2lUphZXWj81a
xCd2pifMB7jFIgBehBBe3pu/C6XQKKk/rje5d/nYlYeCSyMOwAfzXbL9y7phR73R
FJ/x2GeojwLILyt8qff/DPaWsXlOiYE1vTt1l+hJNZQ4fAlrLCh21tYhmp/A1wdk
xsSaz5C3bG55W0C0
-----END CERTIFICATE-----

版本：2
使用者：127.0.0.1
颁发者：/C=CN/ST=Shanghai/L=Shanghai/O=aishu.cn/OU=aishu.cn/emailAddress=jiang.yijun@aishu.cn
起始时间：Mar 23 09:50:08 2021 GMT
结束时间：Mar 23 09:50:08 2021 GMT
序列号：1000
证书指纹：YXNlX2l2X2hlYWQ6X19fX3YLWEhtOv9mBhWKHOHrnbQAAAAV987Pa7Ic6QrtFpPpdz22L77NP6zBqWRoF6cWtRKrxkQ=
证书算法：6


***************************************************/
