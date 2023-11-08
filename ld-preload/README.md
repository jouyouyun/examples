Test LD Proload

# Example
## g_free
- Compile:

``` shell
gcc -Wall -g hook-g_free.c -o hook-g_free.so -fPIC -shared -ldl `pkg-config --libs --cflags glib-2.0`
```

## test
- Compile:

``` shell
gcc -Wall -g test.c -o test `pkg-config --libs --cflags glib-2.0`
```

- Run

``` shell
LD_PRELOAD="./hook-g_free.so" ./test
```

## sudo
- Run

``` shell
LD_PRELOAD="./hook-g_free.so" sudo
```

# Links
- [A Simple LD_PRELOAD Tutorial, Part Two](https://catonmat.net/simple-ld-preload-tutorial-part-two)
- [Playing with LD_PRELOAD](https://axcheron.github.io/playing-with-ld_preload/)
