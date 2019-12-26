
/*
 *  this header file is only used for MsSince MK-1 key
 */

#ifndef _MK_SM2_H_
#define _MK_SM2_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#define CKK_SM2                        0x00010001
#define CKK_SM4                        0x00010002

#define CKA_SM2_CURVE_NAME             0x00010001
#define CKA_SM2_X                      0x00010002
#define CKA_SM2_Y                      0x00010003
#define CKA_SM2_D                      0x00010004
#define CKA_SM2_BITS_LENGTH            0x00010005

#define CKM_SM2_CRYPT                  0x00010001
#define CKM_SM2_SIGN_VERIFY            0x00010002
#define CKM_SM2_SM3_CRYPT              0x00010003
#define CKM_SM2_SM3_SIGN_VERIFY        0x00010004
#define CKM_SM3                        0x00010005
#define CKM_SM4                        0x00010006
#define CKM_SM4_ECB                    0x00010007
#define CKM_SM4_CBC                    0x00010008
#define CKM_SM2_KEY_PAIR_GEN           0x00010009
#define CKM_SM4_KEY_GEN                0x0001000A

#define CKA_CONTAINER          0x80000001 //扩展容器属性
#define CKA_CSP_SIGN_EXCHANGE  0x80000002 //扩展签名交换属性

#ifdef __cplusplus
}
#endif

#endif /* _PKCS11_H_ */



