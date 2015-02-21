# axia coroutine library
cross platform coroutine library
copy from lwan project(https://github.com/lpereira/lwan),  but it is supposed to be cross platform (include windows).

## build
```
git clone https://github.com/thejinchao/axcoro.git axcoro
mkdir _build
cd _build
cmake -G "Unix Makefiles" ../axcoro/
make
./axcoro-test
```

##sample
```C
#include <stdio.h>
#include "ax-coro.h"

static int 
function_foo(coro_t* coro) {
	void* data = coro_get_data(coro);
	for (int i = 0; i < 10; i++) {
		printf("[%d]function_foo, arg=%s\n", i, (const char*)data);
		coro_yield(coro, 0);
	}
	return 1;
}

int 
main(int argc, char* argv[]) {
  coro_switcher_t* switcher = coro_switcher_new();
  coro_t *coro_foo = coro_new(switcher, function_foo, "foo");
  
  do {
    if(coro_resume(coro_foo)) break;
  } while (1);
  
  coro_free(coro_foo);
  coro_switcher_free(switcher);
  return 0;
}
```
