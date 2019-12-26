## Prepare

```shell
ln -svf $PWD/include/pkcs11 /usr/include/
ln -svf $PWD/library/libmkpkcs11.so /lib/x86_64-linux-gnu/
```

## Make

`gcc -Wall -g main.c wosign.c -lcrypto -lmkpkcs11 -o sign_test`
